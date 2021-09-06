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

void objects_def_init(void);

void objects_def_adc_conversion_complete(ADC_HandleTypeDef *hadc, float v, float i);

#endif /*_OBJECTS_DEF_H_*/
