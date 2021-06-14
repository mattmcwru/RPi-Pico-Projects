#ifndef _PICO_BME280_H_
#define _PICO_BME280_H_

#include "bme280.h"

struct bme280_dev *pico_bme280_init();
int8_t stream_sensor_data_forced_mode(struct bme280_dev *dev);

#endif // _PICO_BME280_H_
