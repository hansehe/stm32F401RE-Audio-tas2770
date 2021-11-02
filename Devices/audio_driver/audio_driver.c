/*
 * audio_driver.c
 *
 *  Created on: 14. okt. 2021
 *      Author: Hans Erik Heggem
 */

#include <stdint.h>
#include "main.h"
#include "stm32f4xx_hal.h"
#include "audio_driver.h"
#include "tas2770.h"
#include "../i2c_port/i2c_port.h"

#define I2S_TX_TIMEOUT_MS 1000
#define AUDIO_I2C_TIMEOUT_MS 10000U

extern I2S_HandleTypeDef hi2s2;

HAL_StatusTypeDef audio_init()
{
	uint8_t txData[2];

	HAL_GPIO_WritePin(GPIO_TAS2770_SDZ_GPIO_Port, GPIO_TAS2770_SDZ_Pin, GPIO_PIN_RESET);
	HAL_Delay(100);
	HAL_GPIO_WritePin(GPIO_TAS2770_SDZ_GPIO_Port, GPIO_TAS2770_SDZ_Pin, GPIO_PIN_SET);
	HAL_Delay(100);;

	// Verify revision id and pg id
	txData[0] = 0x7d;
	if(i2c_port_write_read(TAS2770_I2C_SLAVE_ADDRESS, txData, 1, txData, 1, AUDIO_I2C_TIMEOUT_MS) != I2C_PORT_OK)
	{
		return HAL_ERROR;
	}
	uint8_t revision_id = (txData[0] & 0xf0) >> 4;
	uint8_t pg_id = txData[0] & 0x0f;

	if (revision_id != 0x02 || pg_id != 0x00)
	{
		return HAL_ERROR;
	}

	// Page-0
	txData[0] = 0x00;
	txData[1] = 0x00;
	if(i2c_port_write(TAS2770_I2C_SLAVE_ADDRESS, txData, 2, AUDIO_I2C_TIMEOUT_MS) != I2C_PORT_OK)
	{
		return HAL_ERROR;
	}

	// Book-0
	txData[0] = 0x7f;
	txData[1] = 0x00;
	if(i2c_port_write(TAS2770_I2C_SLAVE_ADDRESS, txData, 2, AUDIO_I2C_TIMEOUT_MS) != I2C_PORT_OK)
	{
		return HAL_ERROR;
	}

	// Software Reset
	txData[0] = 0x01;
	txData[1] = 0x01;
	if(i2c_port_write(TAS2770_I2C_SLAVE_ADDRESS, txData, 2, AUDIO_I2C_TIMEOUT_MS) != I2C_PORT_OK)
	{
		return HAL_ERROR;
	}

	HAL_Delay(100);

	// sbclk to fs ratio = 64
	txData[0] = 0x3c;
	txData[1] = 0x11;
	if(i2c_port_write(TAS2770_I2C_SLAVE_ADDRESS, txData, 2, AUDIO_I2C_TIMEOUT_MS) != I2C_PORT_OK)
	{
		return HAL_ERROR;
	}

	// TX bus keeper, Hi-Z, offset 1, TX on Falling edge
	txData[0] = 0x0e;
	txData[1] = 0x33;
	if(i2c_port_write(TAS2770_I2C_SLAVE_ADDRESS, txData, 2, AUDIO_I2C_TIMEOUT_MS) != I2C_PORT_OK)
	{
		return HAL_ERROR;
	}

	// TDM stereo, Word = 16 bit, Frame = 16 bit
	txData[0] = 0x0c;
	txData[1] = 0x30;
	if(i2c_port_write(TAS2770_I2C_SLAVE_ADDRESS, txData, 2, AUDIO_I2C_TIMEOUT_MS) != I2C_PORT_OK)
	{
		return HAL_ERROR;
	}

	// TDM TX voltage sense transmit enable with slot 2,
	txData[0] = 0x0f;
	txData[1] = 0x42;
	if(i2c_port_write(TAS2770_I2C_SLAVE_ADDRESS, txData, 2, AUDIO_I2C_TIMEOUT_MS) != I2C_PORT_OK)
	{
		return HAL_ERROR;
	}

	// TDM TX current sense transmit enable with slot 0
	txData[0] = 0x10;
	txData[1] = 0x40;
	if(i2c_port_write(TAS2770_I2C_SLAVE_ADDRESS, txData, 2, AUDIO_I2C_TIMEOUT_MS) != I2C_PORT_OK)
	{
		return HAL_ERROR;
	}

	// 21 dB gain
	txData[0] = 0x03;
	txData[1] = 0x14;
	if(i2c_port_write(TAS2770_I2C_SLAVE_ADDRESS, txData, 2, AUDIO_I2C_TIMEOUT_MS) != I2C_PORT_OK)
	{
		return HAL_ERROR;
	}

	// power up audio playback with I,V enabled
	txData[0] = 0x02;
	txData[1] = 0x00;
	if(i2c_port_write(TAS2770_I2C_SLAVE_ADDRESS, txData, 2, AUDIO_I2C_TIMEOUT_MS) != I2C_PORT_OK)
	{
		return HAL_ERROR;
	}

	HAL_Delay(100);

	txData[0] = 0x77;
	if(i2c_port_write_read(TAS2770_I2C_SLAVE_ADDRESS, txData, 1, txData, 1, AUDIO_I2C_TIMEOUT_MS) != I2C_PORT_OK)
	{
		return HAL_ERROR;
	}
	uint8_t fs_ratio = (txData[0] & 0b00011100) >> 2; // 05h = 96
	uint8_t fs_rate_v = txData[0] & 0b00000011; // 011b = 44.1/48 KHz

	txData[0] = 0x24;
	if(i2c_port_write_read(TAS2770_I2C_SLAVE_ADDRESS, txData, 1, txData, 1, AUDIO_I2C_TIMEOUT_MS) != I2C_PORT_OK)
	{
		return HAL_ERROR;
	}

	if (txData[0] != 0x00)
	{
		return HAL_ERROR;
	}

	txData[0] = 0x25;
	if(i2c_port_write_read(TAS2770_I2C_SLAVE_ADDRESS, txData, 1, txData, 1, AUDIO_I2C_TIMEOUT_MS) != I2C_PORT_OK)
	{
		return HAL_ERROR;
	}

	if (txData[0] != 0x00)
	{
		return HAL_ERROR;
	}

	return HAL_OK;
}

HAL_StatusTypeDef audio_mute()
{
	uint8_t txData[2];

	// Set DVC Ramp Rate to 0.5 dB / 8 samples
	txData[0] = 0x07;
	txData[1] = 0x80;
	if(i2c_port_write(TAS2770_I2C_SLAVE_ADDRESS, txData, 2, AUDIO_I2C_TIMEOUT_MS) != I2C_PORT_OK)
	{
		return HAL_ERROR;
	}

	// Mute
	txData[0] = 0x02;
	txData[1] = 0x01;
	if(i2c_port_write(TAS2770_I2C_SLAVE_ADDRESS, txData, 2, AUDIO_I2C_TIMEOUT_MS) != I2C_PORT_OK)
	{
		return HAL_ERROR;
	}

	osDelay(100);
	return HAL_OK;
}

HAL_StatusTypeDef audio_unmute()
{
	uint8_t txData[2];

	// Set DVC Ramp Rate to 0.5 dB / 8 samples
	txData[0] = 0x07;
	txData[1] = 0x80;
	if(i2c_port_write(TAS2770_I2C_SLAVE_ADDRESS, txData, 2, AUDIO_I2C_TIMEOUT_MS) != I2C_PORT_OK)
	{
		return HAL_ERROR;
	}

	// Un-mute
	txData[0] = 0x02;
	txData[1] = 0x00;
	if(i2c_port_write(TAS2770_I2C_SLAVE_ADDRESS, txData, 2, AUDIO_I2C_TIMEOUT_MS) != I2C_PORT_OK)
	{
		return HAL_ERROR;
	}

	osDelay(100);
	return HAL_OK;
}

HAL_StatusTypeDef audio_sleep()
{
	audio_mute();
	uint8_t txData[2];

	// Software shutdown
	txData[0] = 0x02;
	txData[1] = 0x02;
	if(i2c_port_write(TAS2770_I2C_SLAVE_ADDRESS, txData, 2, AUDIO_I2C_TIMEOUT_MS) != I2C_PORT_OK)
	{
		return HAL_ERROR;
	}

	return HAL_OK;
}

HAL_StatusTypeDef audio_wakeup()
{
	if (audio_mute() != HAL_OK) {
		return HAL_ERROR;
	}

	uint8_t txData[2];

	// Set DVC Ramp Rate to 0.5 dB / 8 samples
	txData[0] = 0x07;
	txData[1] = 0x80;
	if(i2c_port_write(TAS2770_I2C_SLAVE_ADDRESS, txData, 2, AUDIO_I2C_TIMEOUT_MS) != I2C_PORT_OK)
	{
		return HAL_ERROR;
	}

	// Take device out of low-power shutdown
	txData[0] = 0x02;
	txData[1] = 0x01;
	if(i2c_port_write(TAS2770_I2C_SLAVE_ADDRESS, txData, 2, AUDIO_I2C_TIMEOUT_MS) != I2C_PORT_OK)
	{
		return HAL_ERROR;
	}

	osDelay(100);
	return audio_unmute();
}

HAL_StatusTypeDef audio_play(uint16_t *buffer, uint16_t buflen)
{
    return HAL_I2S_Transmit(&hi2s2, buffer, buflen, I2S_TX_TIMEOUT_MS);
}
