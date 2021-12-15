#include "esp_log.h"
#include "esp_err.h"
#include "driver/i2c.h"
#include "i2c-comp.h"
#include "esp_check.h"

#define TAG ""

i2c::i2c(i2c_port_t _port, i2c_mode_t _mode)
{
    port = _port;
    mode = _mode;
}

i2c::~i2c()
{
    i2c_driver_delete(port);
}

bool i2c::config()
{
    i2c_config_t conf = {};
    conf.mode = mode;
    conf.sda_io_num = (gpio_num_t)sda;
    conf.scl_io_num = (gpio_num_t)scl;
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    
    if (!mode)
    {
        conf.slave.addr_10bit_en = 0;
        conf.slave.slave_addr = address;
    }
    else
    {
        conf.master.clk_speed = 100000;
    }
    esp_err_t err = i2c_param_config((i2c_port_t)port, &conf);
    if (err != ESP_OK)
    {
        return false;
    }
    return true;
}

bool i2c::begin(uint8_t _sda, uint8_t _scl, uint8_t addr)
{
    address = addr;
    sda = _sda;
    scl = _scl;

    if (!config())
        return false;

    return ESP_OK == i2c_driver_install((i2c_port_t)port, mode, I2C_SLAVE_RX_BUF_LEN, I2C_SLAVE_TX_BUF_LEN, 0);
}

int i2c::get(uint8_t *buffer)
{
    int len = 0;
    len = i2c_slave_read_buffer(port, buffer, RW_TEST_LENGTH, 10 / portTICK_RATE_MS);
    return len;
}

int i2c::set(uint8_t *data, size_t len)
{
    if(ESP_OK != i2c_reset_tx_fifo(port)) return ESP_FAIL;
    len = i2c_slave_write_buffer(port, data, len, 10 / portTICK_RATE_MS);
    return len;
}

int i2c::put(uint8_t *data, size_t len)
{
    len = i2c_slave_write_buffer(port, data, len, 10 / portTICK_RATE_MS);
    return len;
}

esp_err_t i2c::write(uint8_t *data, size_t len, int8_t reg)
{
    esp_err_t err = ESP_OK;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    ESP_RETURN_ON_FALSE(cmd != NULL, ESP_ERR_NO_MEM, TAG, "create link");
    ESP_RETURN_ON_FALSE((err = i2c_master_start(cmd)) == ESP_OK, err, TAG, "master start");
    ESP_RETURN_ON_FALSE((err = i2c_master_write_byte(cmd, address << 1 | I2C_MASTER_WRITE, ACK_CHECK_EN)) == ESP_OK, err, TAG, "master start");

    // use register
    if (reg >= 0)
        ESP_RETURN_ON_FALSE((err = i2c_master_write_byte(cmd, reg, ACK_CHECK_EN)) == ESP_OK, err, TAG, "master start");

    ESP_RETURN_ON_FALSE((err = i2c_master_write(cmd, data, len, ACK_CHECK_EN)) == ESP_OK, err, TAG, "master start");

    ESP_RETURN_ON_FALSE((err = i2c_master_stop(cmd)) == ESP_OK, err, TAG, "master stop");

    ESP_RETURN_ON_FALSE((err = i2c_master_cmd_begin(port, cmd, 1000 / portTICK_RATE_MS)) == ESP_OK, err, TAG, "master start");

    i2c_cmd_link_delete(cmd);
    return err;
}

esp_err_t i2c::read(uint8_t *data, size_t len, int8_t reg)
{
    esp_err_t err = ESP_OK;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    ESP_RETURN_ON_FALSE(cmd != NULL, ESP_ERR_NO_MEM, TAG, "create link");
    // use register
    if (reg >= 0)
    {

        ESP_RETURN_ON_FALSE((err = i2c_master_start(cmd)) == ESP_OK, err, TAG, "master start");
        ESP_RETURN_ON_FALSE((err = i2c_master_write_byte(cmd, address << 1 | I2C_MASTER_WRITE, ACK_CHECK_EN)) == ESP_OK, err, TAG, "master start");
        ESP_RETURN_ON_FALSE((err = i2c_master_write_byte(cmd, reg, ACK_CHECK_EN)) == ESP_OK, err, TAG, "master start");
    }

    ESP_RETURN_ON_FALSE((err = i2c_master_start(cmd)) == ESP_OK, err, TAG, "master start");
    ESP_RETURN_ON_FALSE((err = i2c_master_write_byte(cmd, address << 1 | I2C_MASTER_READ, ACK_CHECK_EN)) == ESP_OK, err, TAG, "master start");
    if (len > 1)
    {
        ESP_RETURN_ON_FALSE((err = i2c_master_read(cmd, data, len - 1, (i2c_ack_type_t)ACK_VAL)) == ESP_OK, err, TAG, "master start");
    }
    ESP_RETURN_ON_FALSE((err = i2c_master_read_byte(cmd, data + len - 1, (i2c_ack_type_t)NACK_VAL)) == ESP_OK, err, TAG, "master start");
    ESP_RETURN_ON_FALSE((err = i2c_master_stop(cmd)) == ESP_OK, err, TAG, "master stop");
    ESP_RETURN_ON_FALSE((err = i2c_master_cmd_begin(port, cmd, 100 / portTICK_RATE_MS)) == ESP_OK, err, TAG, "master start");

    i2c_cmd_link_delete(cmd);
    return ESP_OK;
}

int i2c::scan(uint8_t* buf, size_t num)
{
    if(mode != I2C_MODE_MASTER) return 0;

    esp_err_t err = ESP_OK;
    uint8_t device_count = 0;
    for (uint8_t dev_address = 1; dev_address < 127; dev_address++) {
        i2c_cmd_handle_t cmd = i2c_cmd_link_create();
        ESP_RETURN_ON_FALSE(cmd != NULL, -ESP_ERR_NO_MEM, TAG, "create link");
        ESP_RETURN_ON_FALSE((err = i2c_master_start(cmd)) == ESP_OK, -err, TAG, "master start");
        ESP_RETURN_ON_FALSE((err = i2c_master_write_byte(cmd, (dev_address << 1) | I2C_MASTER_WRITE, ACK_CHECK_EN)) == ESP_OK, -err, TAG, "master start");
        ESP_RETURN_ON_FALSE((err = i2c_master_stop(cmd)) == ESP_OK, -err, TAG, "master stop");

        if (i2c_master_cmd_begin(port, cmd, 100 / portTICK_RATE_MS) == ESP_OK) {
            ESP_LOGI("TAG", "found i2c device address = 0x%02x", dev_address);
            if (buf != NULL && device_count < num) {
                *(buf + device_count) = dev_address;
            }
            device_count++;
        }

        i2c_cmd_link_delete(cmd);
    }
    return device_count;
}

    // ESP_RETURN_ON_FALSE(cmd_handle != NULL, ESP_ERR_INVALID_ARG, I2C_TAG, I2C_CMD_LINK_INIT_ERR_STR);
