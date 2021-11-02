/*
 * i2c_port.c
 *
 *  Created on: 10. sep. 2021
 *      Author: Hans Erik Heggem
 */

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "main.h"
#include "i2c_port.h"

extern I2C_HandleTypeDef hi2c1;
I2C_HandleTypeDef *peripheral_i2c_handle = &hi2c1;

i2c_port_status_t i2c_port_write(uint8_t address,
								 uint8_t *data,
                                 uint16_t len,
                                 uint32_t timeout)
{
	HAL_StatusTypeDef ret = HAL_I2C_Master_Transmit(peripheral_i2c_handle, address, data, len, timeout);
	if (ret == HAL_OK) {
		return I2C_PORT_OK;
	}
	HAL_I2C_DeInit(&hi2c1);         //release IO port is GPIO, the reset status flag handle
	HAL_I2C_Init(&hi2c1);           //I2C controller phrase reinitialized
	return i2c_port_write(address, data, len, timeout);
	return I2C_PORT_FAIL;
}

i2c_port_status_t i2c_port_read(uint8_t address,
								uint8_t *rxbuf,
                                uint16_t len,
                                uint32_t timeout)
{
	HAL_StatusTypeDef ret = HAL_I2C_Master_Receive(peripheral_i2c_handle, address, rxbuf, len, timeout);
	if (ret == HAL_OK) {
		return I2C_PORT_OK;
	}
	return I2C_PORT_FAIL;
}

i2c_port_status_t i2c_port_write_read(uint8_t address,
									  uint8_t *txbuf,
                                      uint16_t txlen,
                                      uint8_t *rxbuf,
                                      uint16_t rxlen,
                                      uint32_t timeout)
{
	if (i2c_port_write(address, txbuf, txlen, timeout) != I2C_PORT_OK)
	{
		return I2C_PORT_FAIL;
	}
	if (i2c_port_read(address, rxbuf, rxlen, timeout) != I2C_PORT_OK)
	{
		return I2C_PORT_FAIL;
	}
	return I2C_PORT_OK;
}
