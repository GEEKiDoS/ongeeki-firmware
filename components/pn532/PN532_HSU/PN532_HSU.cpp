#include <driver/uart.h>
#include <cstring>
#include "PN532_HSU.h"
#include "PN532_debug.h"

void uart_write_byte(uart_port_t uart_num, uint8_t byte) {
    uart_write_bytes(uart_num, &byte, 1);
}

size_t uart_available(uart_port_t uart_num) {
    size_t len = 0;
    uart_get_buffered_data_len(uart_num, (size_t*)&len);
    return len;
}

uint8_t uart_read_byte(uart_port_t uart_num) {
    uint8_t result;
    uart_read_bytes(uart_num, &result, 1, 10 / portTICK_PERIOD_MS);

    return result;
}

PN532_HSU::PN532_HSU(gpio_num_t tx, gpio_num_t rx)
{
    uart_config_t uart_config = {
            .baud_rate = 115200,
            .data_bits = UART_DATA_8_BITS,
            .parity = UART_PARITY_DISABLE,
            .stop_bits = UART_STOP_BITS_1,
            .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
            .source_clk = UART_SCLK_APB,
    };

    uart_param_config(UART_NUM_1, &uart_config);
    uart_set_pin(UART_NUM_1, tx, rx, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    uart_driver_install(UART_NUM_1, 128, 128, 0, NULL, 0);

    command = 0;
}

void PN532_HSU::begin()
{

}

void PN532_HSU::wakeup()
{
    uart_write_byte(UART_NUM_1, 0x55);
    uart_write_byte(UART_NUM_1, 0x55);
    uart_write_byte(UART_NUM_1, 0);
    uart_write_byte(UART_NUM_1, 0);
    uart_write_byte(UART_NUM_1, 0);

    /** dump serial buffer */
    if(uart_available(UART_NUM_1)){
        DMSG("Dump serial buffer: ");
    }
    while(uart_available(UART_NUM_1)){
        uint8_t ret = uart_read_byte(UART_NUM_1);
        DMSG_HEX(ret);
    }

}

int8_t PN532_HSU::writeCommand(const uint8_t *header, uint8_t hlen, const uint8_t *body, uint8_t blen)
{

    /** dump serial buffer */
    if(uart_available(UART_NUM_1)){
        DMSG("Dump serial buffer: ");
    }
    while(uart_available(UART_NUM_1)){
        uint8_t ret = uart_read_byte(UART_NUM_1);
        DMSG_HEX(ret);
    }

    command = header[0];
    
    uart_write_byte(UART_NUM_1, PN532_PREAMBLE);
    uart_write_byte(UART_NUM_1, PN532_STARTCODE1);
    uart_write_byte(UART_NUM_1, PN532_STARTCODE2);
    
    uint8_t length = hlen + blen + 1;   // length of data field: TFI + DATA
    uart_write_byte(UART_NUM_1, length);
    uart_write_byte(UART_NUM_1, ~length + 1);         // checksum of length
    
    uart_write_byte(UART_NUM_1, PN532_HOSTTOPN532);
    uint8_t sum = PN532_HOSTTOPN532;    // sum of TFI + DATA

    DMSG("\nWrite: ");
    
    uart_write_bytes(UART_NUM_1, header, hlen);
    for (uint8_t i = 0; i < hlen; i++) {
        sum += header[i];

        DMSG_HEX(header[i]);
    }

    uart_write_bytes(UART_NUM_1, body, blen);
    for (uint8_t i = 0; i < blen; i++) {
        sum += body[i];

        DMSG_HEX(body[i]);
    }
    
    uint8_t checksum = ~sum + 1;            // checksum of TFI + DATA
    uart_write_byte(UART_NUM_1, checksum);
    uart_write_byte(UART_NUM_1, PN532_POSTAMBLE);

    return readAckFrame();
}

int16_t PN532_HSU::readResponse(uint8_t buf[], uint8_t len, uint16_t timeout)
{
    uint8_t tmp[3];
    
    DMSG("\nRead:  ");
    
    /** Frame Preamble and Start Code */
    if(receive(tmp, 3, timeout)<=0){
        return PN532_TIMEOUT;
    }
    if(0 != tmp[0] || 0!= tmp[1] || 0xFF != tmp[2]){
        DMSG("Preamble error");
        return PN532_INVALID_FRAME;
    }
    
    /** receive length and check */
    uint8_t length[2];
    if(receive(length, 2, timeout) <= 0){
        return PN532_TIMEOUT;
    }
    if( 0 != (uint8_t)(length[0] + length[1]) ){
        DMSG("Length error");
        return PN532_INVALID_FRAME;
    }
    length[0] -= 2;
    if( length[0] > len){
        return PN532_NO_SPACE;
    }
    
    /** receive command byte */
    uint8_t cmd = command + 1;               // response command
    if(receive(tmp, 2, timeout) <= 0){
        return PN532_TIMEOUT;
    }
    if( PN532_PN532TOHOST != tmp[0] || cmd != tmp[1]){
        DMSG("Command error");
        return PN532_INVALID_FRAME;
    }
    
    if(receive(buf, length[0], timeout) != length[0]){
        return PN532_TIMEOUT;
    }
    uint8_t sum = PN532_PN532TOHOST + cmd;
    for(uint8_t i=0; i<length[0]; i++){
        sum += buf[i];
    }
    
    /** checksum and postamble */
    if(receive(tmp, 2, timeout) <= 0){
        return PN532_TIMEOUT;
    }
    if( 0 != (uint8_t)(sum + tmp[0]) || 0 != tmp[1] ){
        DMSG("Checksum error");
        return PN532_INVALID_FRAME;
    }
    
    return length[0];
}

int8_t PN532_HSU::readAckFrame()
{
    const uint8_t PN532_ACK[] = {0, 0, 0xFF, 0, 0xFF, 0};
    uint8_t ackBuf[sizeof(PN532_ACK)];
    
    DMSG("\nAck: ");
    
    if( receive(ackBuf, sizeof(PN532_ACK), PN532_ACK_WAIT_TIME) <= 0 ){
        DMSG("Timeout\n");
        return PN532_TIMEOUT;
    }
    
    if( memcmp(ackBuf, PN532_ACK, sizeof(PN532_ACK)) ){
        DMSG("Invalid\n");
        return PN532_INVALID_ACK;
    }
    return 0;
}

/**
    @brief receive data .
    @param buf --> return value buffer.
           len --> length expect to receive.
           timeout --> time of reveiving
    @retval number of received bytes, 0 means no data received.
*/
int8_t PN532_HSU::receive(uint8_t *buf, int len, uint16_t timeout)
{
    return uart_read_bytes(UART_NUM_1, buf, len, timeout  / portTICK_PERIOD_MS);
}
