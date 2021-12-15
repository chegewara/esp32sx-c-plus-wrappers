#pragma once
#include "esp_err.h"
#include "driver/i2c.h"

#define DATA_LENGTH 512                        /*!< Data buffer length of test buffer */
#define I2C_SLAVE_TX_BUF_LEN (2 * DATA_LENGTH) /*!< I2C slave tx buffer size */
#define I2C_SLAVE_RX_BUF_LEN (2 * DATA_LENGTH) /*!< I2C slave rx buffer size */
#define RW_TEST_LENGTH 128                     /*!< Data length for r/w test, [0,DATA_LENGTH] */
#define ACK_CHECK_EN 0x1                       /*!< I2C master will check ack from slave*/
#define ACK_CHECK_DIS 0x0                      /*!< I2C master will not check ack from slave */
#define ACK_VAL 0x0                            /*!< I2C ack value */
#define NACK_VAL 0x1                           /*!< I2C nack value */

class i2c
{
private:
    // address of this device in slave mode
    uint8_t address = 0;
    // I2C port
    i2c_port_t port = 0;
    // I2C mode (master or slave)
    i2c_mode_t mode;
    uint8_t sda = 0;
    uint8_t scl = 0;
    bool config();
    
public:
    /**
     * Constructor
     */
    i2c(i2c_port_t port = I2C_NUM_0, i2c_mode_t mode = I2C_MODE_MASTER);
    /**
     * Destructor
     */
    ~i2c();

    /**
     * @brief Configure and install I2C driver on port
     * @param [in] sda SDA pin
     * @param [in] scl SCL pin
     * @param [in] addr I2C address on slave device
     * 
     * @return return true if driver installed
     */
    bool begin(uint8_t sda, uint8_t scl, uint8_t addr = 4);
    /**
     * @brief Get data from slave RX FIFO buffer, used only on slave port
     * @param [out] buffer to which data is read
     * @return return number of read bytes
     */
    int get(uint8_t* buffer);
    /**
     * @brief Set data into slave TX FIFO buffer, used only n slave port
     * @param [out] buffer data to write into TX FIFO
     * @return return number of wrote bytes
     */
    int set(uint8_t* data, size_t len);
    int put(uint8_t* data, size_t len);

    /**
     * @brief Read data from slave device, used only on master port
     * @param [in] data buffer to read data to
     * @param [in] number max bytes to read
     * @param [in] register which to read from; if negative then ommit register
     */
    esp_err_t read(uint8_t* data, size_t len, int8_t reg = -1);
    /**
     * @brief Write data to slave device, used only on master port
     * @param [in] data to write
     * @param [in] number bytes to write
     * @param [in] register to which to write; if negative then ommit register
     */
    esp_err_t write(uint8_t* data, size_t len, int8_t reg = -1);

    int scan(uint8_t* buf = NULL, size_t num = 0);
};

