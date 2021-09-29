/**
 * @file lcd_16x2.h
 * @date Setembro 2020
 * @brief Arquivo de definições da estrutura do módulo LCD 16X2 I2C.
 */

#ifndef _LCD_16X2_H_
#define _LCD_16X2_H_

#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include "gpio.h"
#include "i2c.h"
#include "structs.h"

#define LCD_I2C_SLAVE_ADDRESS_0  0x4E
#define LCD_I2C_SLAVE_ADDRESS_1  0x7E

int lcd16x2_i2c_init(I2C_HandleTypeDef *pI2cHandle);

void lcd16x2_i2c_setCursor(uint8_t row, uint8_t col);

void lcd16x2_i2c_1stLine(void);

void lcd16x2_i2c_2ndLine(void);

void lcd16x2_i2c_TwoLines(void);
void lcd16x2_i2c_OneLine(void);

void lcd16x2_i2c_cursorShow(int state);

void lcd16x2_i2c_clear(void);

void lcd16x2_i2c_display(int state);

void lcd16x2_i2c_shiftRight(uint8_t offset);

void lcd16x2_i2c_shiftLeft(uint8_t offset);

void lcd16x2_i2c_printf(const char* str, ...);

void lcd16x2_i2c_update(photovoltaic *ptr);

/* LCD Commands */
#define LCD_CLEARDISPLAY    0x01
#define LCD_RETURNHOME      0x02
#define LCD_ENTRYMODESET    0x04
#define LCD_DISPLAYCONTROL  0x08
#define LCD_CURSORSHIFT     0x10
#define LCD_FUNCTIONSET     0x20
#define LCD_SETCGRAMADDR    0x40
#define LCD_SETDDRAMADDR    0x80

/* Commands bitfields */
//1) Entry mode Bitfields
#define LCD_ENTRY_SH      0x01
#define LCD_ENTRY_ID      0x02
//2) Display control
#define LCD_DISPLAY_B     0x01
#define LCD_DISPLAY_C     0x02
#define LCD_DISPLAY_D     0x04
//3) Shift control
#define LCD_SHIFT_RL      0x04
#define LCD_SHIFT_SC      0x08
//4) Function set control
#define LCD_FUNCTION_F    0x04
#define LCD_FUNCTION_N    0x08
#define LCD_FUNCTION_DL   0x10

/* I2C Control bits */
#define LCD_RS        (1 << 0)
#define LCD_RW        (1 << 1)
#define LCD_EN        (1 << 2)
#define LCD_BK_LIGHT  (1 << 3)

#endif /*_LCD_16X2_H_*/
