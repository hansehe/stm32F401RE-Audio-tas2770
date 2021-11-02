/*
 * i2c_port.h
 *
 *  Created on: 10. sep. 2021
 *      Author: Hans Erik Heggem
 */

#ifndef I2C_PORT_I2C_PORT_H_
#define I2C_PORT_I2C_PORT_H_

typedef enum
{
    I2C_PORT_OK,
    I2C_PORT_FAIL
} i2c_port_status_t;

i2c_port_status_t i2c_port_write(uint8_t address,
								 uint8_t *data,
                                 uint16_t len,
                                 uint32_t timeout);

i2c_port_status_t i2c_port_read(uint8_t address,
								uint8_t *rxbuf,
                                uint16_t len,
                                uint32_t timeout);

i2c_port_status_t i2c_port_write_read(uint8_t address,
									  uint8_t *txbuf,
                                      uint16_t txlen,
                                      uint8_t *rxbuf,
                                      uint16_t rxlen,
                                      uint32_t timeout);

#endif /* I2C_PORT_I2C_PORT_H_ */
