#include <cstring>
#include "stdint.h"

#include "driver/gpio.h"
#include "driver/uart.h"

#include "PN532_HSU.h"
#include "PN532_debug.h"


PN532_HSU::PN532_HSU(uart_port_t port, gpio_num_t tx, gpio_num_t rx)
{
    this->port = port;
    this->tx = tx;
    this->rx = rx;

    command = 0;
}

void PN532_HSU::begin()
{
    if(uart_is_driver_installed(port)) {
        return;
    }

    uart_config_t uart_config;
    uart_config.baud_rate = 115200;
    uart_config.data_bits = UART_DATA_8_BITS;
    uart_config.parity = UART_PARITY_DISABLE;
    uart_config.stop_bits = UART_STOP_BITS_1;
    uart_config.flow_ctrl = UART_HW_FLOWCTRL_DISABLE;
    uart_config.rx_flow_ctrl_thresh = 112;
    uart_config.source_clk = UART_SCLK_APB;

    uart_driver_install(port, 256, 0, 20, nullptr, 0);
    uart_param_config(port, &uart_config);
    uart_set_pin(port, tx, rx, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
}

void PN532_HSU::wakeup()
{
    write(0x55);
    write(0x55);
    write(uint8_t(0x00));
    write(uint8_t(0x00));
    write(uint8_t(0x00));

    /** dump serial buffer */
    if (available())
    {
        DMSG("Dump serial buffer: ");
    }
    while (available())
    {
        DMSG_HEX(read());
    }
}

int8_t PN532_HSU::writeCommand(const uint8_t *header, uint8_t hlen, const uint8_t *body, uint8_t blen)
{

    /** dump serial buffer */
    if (available())
    {
        DMSG("Dump serial buffer: ");
    }
    while (available())
    {
        uint8_t ret = read();
        DMSG_HEX(ret);
    }

    command = header[0];

    write(uint8_t(PN532_PREAMBLE));
    write(uint8_t(PN532_STARTCODE1));
    write(uint8_t(PN532_STARTCODE2));

    uint8_t length = hlen + blen + 1; // length of data field: TFI + DATA
    write(length);
    write(~length + 1); // checksum of length

    write(PN532_HOSTTOPN532);
    uint8_t sum = PN532_HOSTTOPN532; // sum of TFI + DATA

    DMSG("\nWrite: ");

    write(header, hlen);
    for (uint8_t i = 0; i < hlen; i++)
    {
        sum += header[i];

        DMSG_HEX(header[i]);
    }

    write(body, blen);

    for (uint8_t i = 0; i < blen; i++) {
        sum += body[i];

        DMSG_HEX(body[i]);
    }

    uint8_t checksum = ~sum + 1; // checksum of TFI + DATA
    write(checksum);
    write(uint8_t(PN532_POSTAMBLE));

    return readAckFrame();
}

int16_t PN532_HSU::readResponse(uint8_t buf[], uint8_t len, uint16_t timeout)
{
    uint8_t tmp[3];

    DMSG("\nRead:  ");

    /** Frame Preamble and Start Code */
    if (receive(tmp, 3, timeout) <= 0)
    {
        return PN532_TIMEOUT;
    }
    if (0 != tmp[0] || 0 != tmp[1] || (0xFF != tmp[2] && 0x0 != tmp[2]))
    {
        DMSG("Preamble error");
        return PN532_INVALID_FRAME;
    }

    /** receive length and check */
    uint8_t length[2];
    if (receive(length, 2, timeout) <= 0)
    {
        return PN532_TIMEOUT;
    }
    if (0 != (uint8_t)(length[0] + length[1]))
    {
        DMSG("Length error");
        return PN532_INVALID_FRAME;
    }
    length[0] -= 2;
    if (length[0] > len)
    {
        return PN532_NO_SPACE;
    }

    /** receive command byte */
    uint8_t cmd = command + 1; // response command
    if (receive(tmp, 2, timeout) <= 0)
    {
        return PN532_TIMEOUT;
    }
    if (PN532_PN532TOHOST != tmp[0] || cmd != tmp[1])
    {
        DMSG("Command error");
        return PN532_INVALID_FRAME;
    }

    if (receive(buf, length[0], timeout) != length[0])
    {
        return PN532_TIMEOUT;
    }
    uint8_t sum = PN532_PN532TOHOST + cmd;
    for (uint8_t i = 0; i < length[0]; i++)
    {
        sum += buf[i];
    }

    /** checksum and postamble */
    if (receive(tmp, 2, timeout) <= 0)
    {
        return PN532_TIMEOUT;
    }
    if (0 != (uint8_t)(sum + tmp[0]) || 0 != tmp[1])
    {
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

    if (receive(ackBuf, sizeof(PN532_ACK), PN532_ACK_WAIT_TIME) <= 0)
    {
        DMSG("Timeout\n");
        return PN532_TIMEOUT;
    }

    if (memcmp(ackBuf, PN532_ACK, sizeof(PN532_ACK)))
    {
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
    auto read = uart_read_bytes(port, buf, len,timeout);

    for(auto i = 0; i < read; i++) {
        DMSG_HEX(buf[i])
    }

    return read;
}
