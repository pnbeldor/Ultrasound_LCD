#ifndef INC_LCD_I2C_H_
#define INC_LCD_I2C_H_

#include "stm32f4xx_hal.h"
#include <stdint.h>

/* Change to 0x3F << 1 if your backpack jumpers differ */
#define LCD_ADDR   (0x27 << 1)
#define LCD_COLS   16
#define LCD_ROWS   2

void LCD_Init(I2C_HandleTypeDef *hi2c);
void LCD_SetCursor(I2C_HandleTypeDef *hi2c, uint8_t col, uint8_t row);
void LCD_WriteChar(I2C_HandleTypeDef *hi2c, char c);
void LCD_WriteString(I2C_HandleTypeDef *hi2c, const char *str);
void LCD_Clear(I2C_HandleTypeDef *hi2c);

#endif
