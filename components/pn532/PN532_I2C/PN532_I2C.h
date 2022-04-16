/**
 * @modified picospuch
 */

#ifndef __PN532_I2C_H__
#define __PN532_I2C_H__

#define PN532_I2C_ADDRESS (0x48 >> 1)

#include <driver/gpio.h>
#include <driver/i2c.h>
#include "PN532Interface.h"

class PN532_I2C : public PN532Interface
{
public:
    PN532_I2C(i2c_port_t i2c_num, gpio_num_t scl, gpio_num_t sda);

    void begin();
    void wakeup();
    virtual int8_t writeCommand(const uint8_t *header, uint8_t hlen, const uint8_t *body = 0, uint8_t blen = 0);
    int16_t readResponse(uint8_t buf[], uint8_t len, uint16_t timeout);

private:
    i2c_port_t i2c_num;
    gpio_num_t scl;
    gpio_num_t sda;

    uint8_t address;

    uint8_t rx_buffer[256];
    uint8_t rx_ptr;

    uint8_t tx_buffer[256];
    uint8_t tx_ptr;

    bool inited;

    uint8_t command;

    int8_t readAckFrame();
    int16_t getResponseLength(uint8_t buf[], uint8_t len, uint16_t timeout);

    inline void beginTransmission(uint8_t deviceAddress) {
        this->address = deviceAddress;
        tx_ptr = 0;
    }

    inline esp_err_t write(uint8_t data) {
        if(tx_ptr == 255) {
            return false;
        }

        tx_buffer[tx_ptr ++] = data;
        return true;
    }

    inline esp_err_t endTransmission() {
        auto error = i2c_master_write_to_device(i2c_num, address, tx_buffer, tx_ptr, 50 / portTICK_RATE_MS);
        tx_ptr = 0;

        return error;
    }

    inline bool requestFrom(uint8_t deviceAddress, size_t size) {
        rx_ptr = 0;
        return i2c_master_read_from_device(i2c_num, deviceAddress, rx_buffer, size, 50 / portTICK_RATE_MS) == ESP_OK;
    }

    inline uint8_t read()
    {
        return rx_buffer[rx_ptr++];
    }
};

#endif
