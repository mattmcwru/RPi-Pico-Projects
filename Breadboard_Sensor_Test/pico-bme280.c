/*
 * BME280 Driver Wrapper for Pico
 *
 * @version     1.0.0
 * @author      Matt McConnell
 * @copyright   2021
 * @licence     MIT
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pico/stdlib.h>
#include <hardware/i2c.h>

// The Bosch Sensortec BME280 sensor driver is required: 
//   https://github.com/BoschSensortec/BME280_driver
#include "bme280.h"

// Set the I2C device and GPIO pair connected to the sensor
#define BME280_I2C_DEV     i2c1
#define BME280_I2C_SDA_PIN 14
#define BME280_I2C_SCL_PIN 15


typedef struct {
    /* Variable to hold device address */
    uint8_t dev_addr;

    /* Variable that contains i2c descriptor */
    i2c_inst_t *fd;
} intf_t;

/*!
 * @brief This function provides the delay for required time (Microseconds) as per the input 
 * provided in some of the APIs
 */
void user_delay_us(uint32_t period, void *intf_ptr)
{
    sleep_us(period);
}

/*!
 * @brief This function reading the sensor's registers through I2C bus.
 */
int8_t user_i2c_read(uint8_t reg_addr, uint8_t *data, uint32_t len, void *intf_ptr)
{
    intf_t *id = (intf_t*)intf_ptr;

    i2c_write_blocking(id->fd, id->dev_addr, &reg_addr, 1, true);
    i2c_read_blocking(id->fd, id->dev_addr, data, len, false); 

    return BME280_OK;
}

/*!
 * @brief This function for writing the sensor's registers through I2C bus.
 */
int8_t user_i2c_write(uint8_t reg_addr, const uint8_t *data, uint32_t len, void *intf_ptr)
{
    uint8_t *buf;
    intf_t *id = (intf_t*)intf_ptr;
    int8_t ret = BME280_OK;

    buf = malloc(len + 1);
    if (NULL == buf) {
        return BME280_E_NULL_PTR;
    }

    buf[0] = reg_addr;
    memcpy(buf + 1, data, len);
    
    if (i2c_write_blocking(id->fd, id->dev_addr, buf, len + 1, false) < len) {
        ret = BME280_E_COMM_FAIL;
    }

    free(buf);
    return ret;
}

/*!
 * @brief This function outputs a sensor data debug message
 */
void print_sensor_data(struct bme280_data *comp_data)
{
#ifdef BME280_FLOAT_ENABLE
    printf("%0.2f, %0.2f, %0.2f\r\n", comp_data->temperature, comp_data->pressure, comp_data->humidity);
#else
    printf("%ld, %ld, %ld\r\n", comp_data->temperature, comp_data->pressure, comp_data->humidity);
#endif
}

/*!
 * @brief This function initializes the I2C device and the BME280 driver.
 */
struct bme280_dev *pico_bme280_init()
{
    static struct bme280_dev dev = {0};
    static intf_t id = {0};

#if !defined(i2c_default) || !defined(PICO_DEFAULT_I2C_SDA_PIN) || !defined(PICO_DEFAULT_I2C_SCL_PIN)
    #warning i2c/mpu6050_i2c example requires a board with I2C pins
#endif

    id.dev_addr = BME280_I2C_ADDR_PRIM;
    id.fd = BME280_I2C_DEV;

    i2c_init(id.fd, 400000);
    gpio_set_function(BME280_I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(BME280_I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(BME280_I2C_SDA_PIN);
    gpio_pull_up(BME280_I2C_SCL_PIN);

    dev.intf = BME280_I2C_INTF;
    dev.read = user_i2c_read;
    dev.write = user_i2c_write;
    dev.delay_us = user_delay_us;
    dev.intf_ptr = &id;

    int8_t status = bme280_init(&dev);
    if (status != BME280_OK) {
        printf("Failed to initialize the device (code %+d).\n", status);
    }

    return &dev;
}

/*!
 * @brief This function continuously reads data from the BME280 sensor.
 */
int8_t stream_sensor_data_forced_mode(struct bme280_dev *dev)
{
    int8_t rslt;
    uint8_t settings_sel;
	uint32_t req_delay;
    struct bme280_data comp_data;

    /* Recommended mode of operation: Indoor navigation */
    dev->settings.osr_h = BME280_OVERSAMPLING_1X;
    dev->settings.osr_p = BME280_OVERSAMPLING_16X;
    dev->settings.osr_t = BME280_OVERSAMPLING_2X;
    dev->settings.filter = BME280_FILTER_COEFF_16;

    settings_sel = BME280_OSR_PRESS_SEL | BME280_OSR_TEMP_SEL | BME280_OSR_HUM_SEL | BME280_FILTER_SEL;

    rslt = bme280_set_sensor_settings(settings_sel, dev);
	
	/*Calculate the minimum delay required between consecutive measurement based upon the sensor enabled
     *  and the oversampling configuration. */
    req_delay = bme280_cal_meas_delay(&dev->settings);

    printf("Temperature, Pressure, Humidity\r\n");
    /* Continuously stream sensor data */
    while (1) {
        rslt = bme280_set_sensor_mode(BME280_FORCED_MODE, dev);
        /* Wait for the measurement to complete and print data @25Hz */
        dev->delay_us(req_delay*1000, dev->intf_ptr);
        rslt = bme280_get_sensor_data(BME280_ALL, &comp_data, dev);
        print_sensor_data(&comp_data);
    }
    return rslt;
}

