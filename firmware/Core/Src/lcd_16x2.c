#include "lcd_16x2.h"

static I2C_HandleTypeDef* lcd16x2_i2cHandle;
static uint8_t LCD_I2C_SLAVE_ADDRESS = 0;

static void lcd16x2_i2c_sendCommand(uint8_t command)
{
  const uint8_t command_0_3 = (0xF0 & (command << 4));
  const uint8_t command_4_7 = (0xF0 & command);

  uint8_t i2cData[4] =
  {
      command_4_7 | LCD_EN | LCD_BK_LIGHT,
      command_4_7 | LCD_BK_LIGHT,
      command_0_3 | LCD_EN | LCD_BK_LIGHT,
      command_0_3 | LCD_BK_LIGHT,
  };

  HAL_I2C_Master_Transmit(lcd16x2_i2cHandle, LCD_I2C_SLAVE_ADDRESS, i2cData, 4, 200);
}

static void lcd16x2_i2c_sendData(uint8_t data)
{
  const uint8_t data_0_3 = (0xF0 & (data << 4));
  const uint8_t data_4_7 = (0xF0 & data);

  uint8_t i2cData[4] =
  {
      data_4_7 | LCD_EN | LCD_BK_LIGHT | LCD_RS,
      data_4_7 | LCD_BK_LIGHT | LCD_RS,
      data_0_3 | LCD_EN | LCD_BK_LIGHT | LCD_RS,
      data_0_3 | LCD_BK_LIGHT | LCD_RS,
  };

  HAL_I2C_Master_Transmit(lcd16x2_i2cHandle, LCD_I2C_SLAVE_ADDRESS, i2cData, 4, 200);
}

int lcd16x2_i2c_init(I2C_HandleTypeDef *pI2cHandle)
{
  HAL_Delay(50);

  lcd16x2_i2cHandle = pI2cHandle;

  if (HAL_I2C_IsDeviceReady(lcd16x2_i2cHandle, LCD_I2C_SLAVE_ADDRESS_0, 5, 500) != HAL_OK)
  {
    if (HAL_I2C_IsDeviceReady(lcd16x2_i2cHandle, LCD_I2C_SLAVE_ADDRESS_1, 5, 500) != HAL_OK)
    {
      return (0);
    }
    else
    {
      LCD_I2C_SLAVE_ADDRESS = LCD_I2C_SLAVE_ADDRESS_1;
    }
  }
  else
  {
    LCD_I2C_SLAVE_ADDRESS = LCD_I2C_SLAVE_ADDRESS_0;
  }

  //1. Wait at least 15ms
  HAL_Delay(45);

  //2. Attentions sequence
  lcd16x2_i2c_sendCommand(0x30);
  HAL_Delay(5);
  lcd16x2_i2c_sendCommand(0x30);
  HAL_Delay(1);
  lcd16x2_i2c_sendCommand(0x30);
  HAL_Delay(8);
  lcd16x2_i2c_sendCommand(0x20);
  HAL_Delay(8);

  lcd16x2_i2c_sendCommand(LCD_FUNCTIONSET | LCD_FUNCTION_N);
  HAL_Delay(1);
  lcd16x2_i2c_sendCommand(LCD_DISPLAYCONTROL);
  HAL_Delay(1);
  lcd16x2_i2c_sendCommand(LCD_CLEARDISPLAY);
  HAL_Delay(3);
  lcd16x2_i2c_sendCommand(0x04 | LCD_ENTRY_ID);
  HAL_Delay(1);
  lcd16x2_i2c_sendCommand(LCD_DISPLAYCONTROL | LCD_DISPLAY_D);
  HAL_Delay(3);

  return (1);
}

void lcd16x2_i2c_setCursor(uint8_t row, uint8_t col)
{
  uint8_t maskData;

  maskData = (col) & 0x0F;

  if (row==0)
  {
    maskData |= (0x80);
    lcd16x2_i2c_sendCommand(maskData);
  }
  else
  {
    maskData |= (0xc0);
    lcd16x2_i2c_sendCommand(maskData);
  }
}

void lcd16x2_i2c_1stLine(void)
{
  lcd16x2_i2c_setCursor(0, 0);
}

void lcd16x2_i2c_2ndLine(void)
{
  lcd16x2_i2c_setCursor(1, 0);
}

void lcd16x2_i2c_TwoLines(void)
{
  lcd16x2_i2c_sendCommand(LCD_FUNCTIONSET | LCD_FUNCTION_N);
}

void lcd16x2_i2c_OneLine(void)
{
  lcd16x2_i2c_sendCommand(LCD_FUNCTIONSET);
}

void lcd16x2_i2c_cursorShow(int state)
{
  if (state)
  {
    lcd16x2_i2c_sendCommand(LCD_DISPLAYCONTROL | LCD_DISPLAY_B | LCD_DISPLAY_C | LCD_DISPLAY_D);
  }
  else
  {
    lcd16x2_i2c_sendCommand(LCD_DISPLAYCONTROL | LCD_DISPLAY_D);
  }
}

void lcd16x2_i2c_clear(void)
{
	lcd16x2_i2c_sendCommand(LCD_CLEARDISPLAY);

	HAL_Delay(3);
}

void lcd16x2_i2c_display(int state)
{
  if (state)
  {
    lcd16x2_i2c_sendCommand(LCD_DISPLAYCONTROL | LCD_DISPLAY_B | LCD_DISPLAY_C | LCD_DISPLAY_D);
  }
  else
  {
    lcd16x2_i2c_sendCommand(LCD_DISPLAYCONTROL | LCD_DISPLAY_B | LCD_DISPLAY_C);
  }
}

void lcd16x2_i2c_shiftRight(uint8_t offset)
{
  for (uint8_t i=0; i<offset;i++)
  {
    lcd16x2_i2c_sendCommand(0x1c);
  }
}

void lcd16x2_i2c_shiftLeft(uint8_t offset)
{
  for (uint8_t i=0; i<offset;i++)
  {
    lcd16x2_i2c_sendCommand(0x18);
  }
}

void lcd16x2_i2c_printf(const char* str, ...)
{
  char stringArray[20];
  va_list args;

  va_start(args, str);

  vsprintf(stringArray, str, args);

  va_end(args);

  for (uint8_t i=0;  i<strlen(stringArray) && i<16; i++)
  {
    lcd16x2_i2c_sendData((uint8_t)stringArray[i]);
  }
}

void lcd16x2_i2c_update(photovoltaic *ptr)
{
	// Tensão
	lcd16x2_i2c_setCursor(0,0);
	lcd16x2_i2c_printf("V:%.2f [V]                ", ptr->voltage->scnd_level_value);

	// Corrente
	lcd16x2_i2c_setCursor(1,0);
	lcd16x2_i2c_printf("I:%.2f [A]                ", ptr->current->scnd_level_value);
}
