cmake_minimum_required(VERSION 3.13)
include(pico_sdk_import.cmake)
project(Breadboard_Sensor_Test VERSION 1.0.0)

add_executable(Breadboard_Sensor_Test
               breadboard_sensor_test.c
               pico-bme280.c
               BME280_driver/bme280.c)

pico_sdk_init()

pico_enable_stdio_usb(Breadboard_Sensor_Test 1)
pico_enable_stdio_uart(Breadboard_Sensor_Test 1)
pico_add_extra_outputs(Breadboard_Sensor_Test)

include_directories(BME280_driver)

target_link_libraries(Breadboard_Sensor_Test
                      pico_stdlib
                      pico_malloc
                      pico_mem_ops
                      hardware_gpio
                      hardware_i2c
                      hardware_adc)
