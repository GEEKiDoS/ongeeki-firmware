/**
 * @modified picospuch
 */

#include "PN532_I2C.h"
#include "PN532_debug.h"
#include <driver/i2c.h>
#include <cstring>

PN532_I2C::PN532_I2C(i2c_port_t i2c_num, gpio_num_t scl, gpio_num_t sda)
{
    this->i2c_num = i2c_num;
    this->scl = scl;
    this->sda = sda;

    inited = false;
    command = 0;
}

void PN532_I2C::begin()
{
    if(inited) {
        i2c_driver_delete(i2c_num);
    }

    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = sda,
        .scl_io_num = scl,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master = {
            .clk_speed = 100000,
        },
        .clk_flags = I2C_SCLK_SRC_FLAG_FOR_NOMAL,
    };

    i2c_param_config(i2c_num, &conf);
    i2c_driver_install(i2c_num, conf.mode, 0, 0, 0);
    i2c_set_timeout(i2c_num, 400000);

    inited = true;
}

void PN532_I2C::wakeup()
{
    vTaskDelay(500 / portTICK_RATE_MS); // wait for all ready to manipulate pn532
}

int8_t PN532_I2C::writeCommand(const uint8_t *header, uint8_t hlen, const uint8_t *body, uint8_t blen)
{
    command = header[0];
    this->beginTransmission(PN532_I2C_ADDRESS);

    write(PN532_PREAMBLE);
    write(PN532_STARTCODE1);
    write(PN532_STARTCODE2);

    uint8_t length = hlen + blen + 1; // length of data field: TFI + DATA
    write(length);
    write(~length + 1); // checksum of length

    write(PN532_HOSTTOPN532);
    uint8_t sum = PN532_HOSTTOPN532; // sum of TFI + DATA

    DMSG("write: ");

    for (uint8_t i = 0; i < hlen; i++)
    {
        if (write(header[i]))
        {
            sum += header[i];

            DMSG_HEX(header[i]);
        }
        else
        {
            DMSG("\nToo many data to send, I2C doesn't support such a big packet\n"); // I2C max packet: 32 bytes
            return PN532_INVALID_FRAME;
        }
    }

    for (uint8_t i = 0; i < blen; i++)
    {
        if (write(body[i]))
        {
            sum += body[i];

            DMSG_HEX(body[i]);
        }
        else
        {
            DMSG("\nToo many data to send, I2C doesn't support such a big packet\n"); // I2C max packet: 32 bytes
            return PN532_INVALID_FRAME;
        }
    }

    uint8_t checksum = ~sum + 1; // checksum of TFI + DATA
    write(checksum);
    write(PN532_POSTAMBLE);

    this->endTransmission();

    DMSG("\n");

    return readAckFrame();
}

int16_t PN532_I2C::getResponseLength(uint8_t buf[], uint8_t len, uint16_t timeout)
{
    const uint8_t PN532_NACK[] = {0, 0, 0xFF, 0xFF, 0, 0};
    uint16_t time = 0;

    do
    {
        if (this->requestFrom(PN532_I2C_ADDRESS, 6))
        {
            if (read() & 1)
            {          // check first byte --- status
                break; // PN532 is ready
            }
        }

        vTaskDelay(1 / portTICK_RATE_MS);
        time++;
        if ((0 != timeout) && (time > timeout))
        {
            return -1;
        }
    } while (1);

    if (0x00 != read() || // PREAMBLE
        0x00 != read() || // STARTCODE1
        0xFF != read()    // STARTCODE2
    )
    {

        return PN532_INVALID_FRAME;
    }

    uint8_t length = read();

    // request for last respond msg again
    this->beginTransmission(PN532_I2C_ADDRESS);
    for (uint16_t i = 0; i < sizeof(PN532_NACK); ++i)
    {
        write(PN532_NACK[i]);
    }
    this->endTransmission();

    return length;
}

int16_t PN532_I2C::readResponse(uint8_t buf[], uint8_t len, uint16_t timeout)
{
    uint16_t time = 0;
    uint8_t length;

    length = getResponseLength(buf, len, timeout);

    // [RDY] 00 00 FF LEN LCS (TFI PD0 ... PDn) DCS 00
    do
    {
        if (this->requestFrom(PN532_I2C_ADDRESS, 6 + length + 2))
        {
            if (read() & 1)
            {          // check first byte --- status
                break; // PN532 is ready
            }
        }

        vTaskDelay(1 / portTICK_RATE_MS);
        time++;
        if ((0 != timeout) && (time > timeout))
        {
            return -1;
        }
    } while (1);

    if (0x00 != read() || // PREAMBLE
        0x00 != read() || // STARTCODE1
        0xFF != read()    // STARTCODE2
    )
    {

        return PN532_INVALID_FRAME;
    }

    length = read();

    if (0 != (uint8_t)(length + read()))
    { // checksum of length
        return PN532_INVALID_FRAME;
    }

    uint8_t cmd = command + 1; // response command
    if (PN532_PN532TOHOST != read() || (cmd) != read())
    {
        return PN532_INVALID_FRAME;
    }

    length -= 2;
    if (length > len)
    {
        return PN532_NO_SPACE; // not enough space
    }

    DMSG("read: ");
    DMSG_HEX(cmd);

    uint8_t sum = PN532_PN532TOHOST + cmd;
    for (uint8_t i = 0; i < length; i++)
    {
        buf[i] = read();
        sum += buf[i];

        DMSG_HEX(buf[i]);
    }
    DMSG("\n");

    uint8_t checksum = read();
    if (0 != (uint8_t)(sum + checksum))
    {
        DMSG("checksum is not ok\n");
        return PN532_INVALID_FRAME;
    }
    read(); // POSTAMBLE

    DMSG("i2c read length: %d\n", length);
    return length;
}

int8_t PN532_I2C::readAckFrame()
{
    const uint8_t PN532_ACK[] = {0, 0, 0xFF, 0, 0xFF, 0};
    uint8_t ackBuf[sizeof(PN532_ACK)];

    DMSG("wait for ack at : ");
    DMSG_INT(xTaskGetTickCount() * portTICK_RATE_MS);
    DMSG("\n");

    uint16_t time = 0;
    do
    {
        if (this->requestFrom(PN532_I2C_ADDRESS, sizeof(PN532_ACK) + 1))
        {
            if (read() & 1)
            {          // check first byte --- status
                break; // PN532 is ready
            }
        }

        vTaskDelay(1 / portTICK_RATE_MS);
        time++;
        if (time > PN532_ACK_WAIT_TIME)
        {
            DMSG("Time out when waiting for ACK\n");
            return PN532_TIMEOUT;
        }
    } while (1);

    DMSG("ready at:");
    DMSG_INT(xTaskGetTickCount() * portTICK_RATE_MS);
    DMSG("\n");

    for (uint8_t i = 0; i < sizeof(PN532_ACK); i++)
    {
        ackBuf[i] = read();
    }

    if (memcmp(ackBuf, PN532_ACK, sizeof(PN532_ACK)))
    {
        DMSG("Invalid ACK\n");
        return PN532_INVALID_ACK;
    }

    return 0;
}
