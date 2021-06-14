/*
 * Breadboard_Sensor_Test for Raspberry Pi Pico
 *
 * @version     1.0.0
 * @author      Matt McConnell
 * @copyright   2021
 * @licence     MIT
 *
 */
#include <stdio.h>

#include "breadboard_sensor_test.h"
#include "pico-bme280.h"

#define BME280_I2C_DEV     i2c1
#define BME280_I2C_SDA_PIN 14
#define BME280_I2C_SCL_PIN 15

// Set this to true to run the I2C bus scan if having trouble finding the BME280 sensor.
#define RUN_I2C_BUS_SCAN false

int i2c_bus_scan();

/*
 *  Main Program
 */ 
int main() {
    stdio_init_all();

#if RUN_I2C_BUS_SCAN
    printf("Starting I2C Bus Scan...\n");
    i2c_bus_scan();
#endif

    printf("Starting BME280 Sensor Test...\n");
    struct bme280_dev *dev = pico_bme280_init();
    stream_sensor_data_forced_mode(dev);

    return 0;
}

/*
 *  I2C Bus Scan
 */
bool reserved_addr(uint8_t addr) {
    return (addr & 0x78) == 0 || (addr & 0x78) == 0x78;
}

int i2c_bus_scan() {

    i2c_init(BME280_I2C_DEV, 100 * 1000);
    gpio_set_function(BME280_I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(BME280_I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(BME280_I2C_SDA_PIN);
    gpio_pull_up(BME280_I2C_SCL_PIN);
    //bi_decl(bi_2pins_with_func(BME280_I2C_SDA_PIN, BME280_I2C_SCL_PIN, GPIO_FUNC_I2C));

    printf("\nI2C Bus Scan\n");
    printf("   0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F\n");

    for (int addr = 0; addr < (1 << 7); ++addr) {
        if (addr % 16 == 0) {
            printf("%02x ", addr);
        }

        // Perform a 1-byte dummy read from the probe address. If a slave
        // acknowledges this address, the function returns the number of bytes
        // transferred. If the address byte is ignored, the function returns
        // -1.

        // Skip over any reserved addresses.
        int ret;
        uint8_t rxdata;
        if (reserved_addr(addr))
            ret = PICO_ERROR_GENERIC;
        else
            ret = i2c_read_blocking(BME280_I2C_DEV, addr, &rxdata, 1, false);

        printf(ret < 0 ? "." : "@");
        printf(addr % 16 == 15 ? "\n" : "  ");
    }
    printf("Done.\n");
}