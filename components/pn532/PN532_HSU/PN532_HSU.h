
#ifndef __PN532_HSU_H__
#define __PN532_HSU_H__

#include "PN532Interface.h"

#define PN532_HSU_DEBUG

#define PN532_HSU_READ_TIMEOUT (1000)

class PN532_HSU : public PN532Interface
{
public:
    PN532_HSU(uart_port_t port, gpio_num_t tx, gpio_num_t rx);

    void begin();
    void wakeup();
    virtual int8_t writeCommand(const uint8_t *header, uint8_t hlen, const uint8_t *body = 0, uint8_t blen = 0);
    int16_t readResponse(uint8_t buf[], uint8_t len, uint16_t timeout);

private:
    uart_port_t port;
    gpio_num_t tx;
    gpio_num_t rx;
    uint8_t command;

    int8_t readAckFrame();
    int8_t receive(uint8_t *buf, int len, uint16_t timeout = PN532_HSU_READ_TIMEOUT);

    inline int write(uint8_t data) {
        return uart_write_bytes(port, &data, 1);
    }

    inline int write(const uint8_t *buffer, size_t len) {
        if(buffer && len)
            return uart_write_bytes(port, buffer, len);

        return 0;
    }

    inline uint8_t read() {
        uint8_t c = 0;
        int len = uart_read_bytes(port, &c, 1, 20 / portTICK_RATE_MS);
        if (len == 0) {
            c  = 0;
        }

        return c;
    }

    inline size_t available() {
        size_t result;
        uart_get_buffered_data_len(port, &result);

        return result;
    }
};

#endif
