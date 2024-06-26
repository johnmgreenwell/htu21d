/*!
 * @file HTU21D.cpp
 * @mainpage HTU21D Sensor Library
 *
 * This is an Arduino library for the HTU21D temperature and humidity sensor.
 * 
 * For details on the sensor please refer to the datasheet.
 * It can be found on the manufactures page:
 * https://www.te.com/deu-de/product-CAT-HSC0004.html
 *
 * This library was written by Daniel Wiese (DevXplained).
 * It has been slightly modified by John Greenwell for a custom HAL framework.
 * 
 * Redistribution is possible under the terms of the MIT license.
 */

#include "htu21d.h"

#define constrain(amt,low,high) ((amt)<(low)?(low):((amt)>(high)?(high):(amt)))

namespace PeripheralIO
{

static const uint8_t HTU21D_DELAY_T[] = {50, 13, 25, 7};
static const uint8_t HTU21D_DELAY_H[] = {16, 3, 5, 8};
static const float HTU21D_TCoeff = -0.15;

/**
 * Constructor
 * Initializes a new sensor instance
 * 
 * @param addr Sensor Address (default 0x40)
 * @param i2c Reference to instance of HAL I2C
 */
HTU21D::HTU21D(HAL::I2C& i2c_bus) : _i2c(i2c_bus), _addr(HTU21D_ADDR), _resolution(RESOLUTION_RH12_T14)
{ }

bool HTU21D::checkCRC8(uint8_t data[]) {
  uint8_t crc = 0;
  for(int i = 0; i < 2; i++) {
    crc ^= data[i];
    for(int b = 7; b >= 0; b--) {
      if(crc & 0x80) crc = (crc << 1) ^ 0x31;
      else crc = (crc << 1);
    }
  }
  
  return crc == data[2];
}

bool HTU21D::measureTemperature() {
  uint8_t data[3];

  /* Measure temperature */
  _i2c.write(_addr, static_cast<uint8_t>(TRIGGER_TEMP_MEAS_NH));
  
  HAL::delay_ms(HTU21D_DELAY_T[_resolution]);
  
  _i2c.read(_addr, data, static_cast<uint8_t>(3));
  
  if(!checkCRC8(data)) return false;
  
  uint16_t St = (data[0] << 8) | (data[1] & 0xFC);
  temperature = -46.85 + 175.72 * St / 65536.0;
  
  return true;
}

bool HTU21D::measureHumidity() {
  uint8_t data[3];

  /* Measure humidity */
  _i2c.write(_addr, static_cast<uint8_t>(TRIGGER_HUM_MEAS_NH));
  
  delay(HTU21D_DELAY_H[_resolution]);

  _i2c.read(_addr, data, static_cast<uint8_t>(3));

  if(!checkCRC8(data)) return false;
  
  uint16_t Srh = (data[0] << 8) | (data[1] & 0xFC);
  humidity = -6.0 + 125.0 * Srh / 65536.0;
  humidity += (25.0 - temperature) * HTU21D_TCoeff;
  humidity = constrain(humidity, 0.0, 100.0);
  
  return true;
}

/**
 * Starts a temperature and a humidity measurement and reads the result.
 * @return true if the result was read correctly, otherwise false
 */
bool HTU21D::measure() {
  /* Reset values */
  temperature = NAN;
  humidity = NAN;
  
  /* NOTE: Order is important as the temperature is needed to correct the humidity reading */
  if(!measureTemperature()) return false;
  if(!measureHumidity()) return false;
  return true;
}

/**
 * Sets the measurement resolution.
 * 
 * @param resolution Resolution 
 * @see HTU21DResolution for possible resolutions
 */
void HTU21D::setResolution(HTU21DResolution resolution) {

  _i2c.write(_addr, WRITE_USER_REG, (resolution & 0x01) | ((resolution & 0x02) << 6) | 0x02);
  
  _resolution = resolution;
}

/**
 * Get the current resolution
 * @return Resolution
 * @see HTU21DResolution for possible resolutions
 */
HTU21DResolution HTU21D::getResolution() {
  return _resolution;
}

/**
 * Initializes the I2C transport (Wire.begin()) and resets the sensor.
 * @return true if the initialization was successful, otherwise false
 */
bool HTU21D::begin() {
  return reset();
}
  
/**
 * Resets the sensor to its default settings and resolution.
 * @return true if the reset was successful, otherwise false
 */
bool HTU21D::reset() {
  uint8_t data[3];

  _i2c.write(_addr, static_cast<uint8_t>(SOFT_RESET));
  
  HAL::delay_ms(15);
  
  _i2c.writeRead(_addr, static_cast<uint8_t>(READ_USER_REG), data, static_cast<uint8_t>(3));

  if(data[0] != 0x02) return false;
  
  _resolution = RESOLUTION_RH12_T14;

  return true;
}

/**
 * Returns the temperature value acquired with the last measurement.
 * To refresh this value call measure().
 * 
 * @returns Temperature in °C or NaN
 */
float HTU21D::getTemperature() const {
  return temperature;
}

/**
 * Returns the humidity value acquired with the last measurement.
 * To refresh this value call measure().
 * 
 * @returns Relative Humidity in percent or NaN
 */
float HTU21D::getHumidity() const {
  return humidity;
}

}

// EOF
