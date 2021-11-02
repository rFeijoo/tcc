/**
 * @file objects_def.h
 * @date Agosto 2020
 * @brief Arquivo de definições gerais do firmware.
 */


#ifndef _OBJECTS_DEF_H_
#define _OBJECTS_DEF_H_

#include <stdint.h>
#include "adc.h"
#include "tim.h"
#include "meas.h"
#include "events.h"

void objects_def_init(void);

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc);

void HAL_ADC_LevelOutOfWindowCallback(ADC_HandleTypeDef *hadc);

void objects_def_exti_gpio(uint16_t GPIO_Pin);

void objects_def_loop(void);

#endif /*_OBJECTS_DEF_H_*/
