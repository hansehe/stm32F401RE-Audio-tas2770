/*
 * audio_driver.h
 *
 *  Created on: 14. okt. 2021
 *      Author: Hans Erik Heggem
 */

#ifndef AUDIO_DRIVER_H_
#define AUDIO_DRIVER_H_

#include "stm32f4xx_hal.h"

HAL_StatusTypeDef audio_init();
HAL_StatusTypeDef audio_mute();
HAL_StatusTypeDef audio_unmute();
HAL_StatusTypeDef audio_sleep();
HAL_StatusTypeDef audio_wakeup();
HAL_StatusTypeDef audio_play(uint16_t *buffer, uint16_t buflen);

#endif /* AUDIO_DRIVER_H_ */
