# HTU21D Sensor Driver
This is a custom driver for the HTU21D temperature and humidity sensor.

It allows to set the measurement resolution and implements the temperature correction formula and the CRC algorithm given in the manufactures datasheet.

For details on the sensor please refer to the datasheet.

This library was written by Daniel Wiese (DevXplained).
It was modified by John Greenwell for compatibility with a custom HAL framework.
Redistribution is possible under the terms of the MIT license.

## Usage

For this modified version, the following hardware abstraction layer (HAL) requirements must be satisfied:

* A header file `hal.h` providing access to HAL classes and methods.
* An `I2C` class within the `HAL` namespace with the following methods:
  - Write one byte to the device: `write(uint8_t device_address, uint8_t data_to_write)`.
  - Write two bytes to the device (a register and data to place there): `write(uint8_t device_address, uint8_t register, uint8_t data_to_write)`.
  - Write one byte to the device, execute a repeated start, then read n bytes from the device: `writeRead(uint8_t device_address, uint8_t register, uint8_t * data_buffer, uint32_t length_in_bytes)`
  - Read n bytes from the device:`read(uint8_t device_address, uint8_t * data_buffer, uint32_t length_in_bytes)`.
* A `delay_ms(uint32_t milliseconds)` function within the `HAL` namespace.

## References
Manufacturer Page: [https://www.te.com/deu-de/product-CAT-HSC0004.html](https://www.te.com/deu-de/product-CAT-HSC0004.html)

## Platform Specifics
For ESP32 boards the i2c communication pattern deviates from the datasheet.
A delay during the i2c communication is not possible on the ESP32. For this reason the data is queried in two operations.

If your target platform has i2c communication problems and requires similar quirks please open an issue.
