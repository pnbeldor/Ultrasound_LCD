#include "lcd_i2c.h"

/* PCF8574 bit mapping
   Bit: 7   6   5   4   3    2   1    0
        D7  D6  D5  D4  BL  EN  R/W  RS  */

#define RS 0x01
#define EN 0x04
#define BL 0x08  /* backlight always on */

static void lcd_send_byte(I2C_HandleTypeDef *hi2c, uint8_t data) {
    HAL_I2C_Master_Transmit(hi2c, LCD_ADDR, &data, 1, 10);
}

static void lcd_pulse_en(I2C_HandleTypeDef *hi2c, uint8_t data) {
    lcd_send_byte(hi2c, data | EN | BL);
    HAL_Delay(1);
    lcd_send_byte(hi2c, (data & ~EN) | BL);
    HAL_Delay(1);
}

static void lcd_write_nibble(I2C_HandleTypeDef *hi2c, uint8_t nibble, uint8_t mode) {
    uint8_t data = (nibble & 0xF0) | mode | BL;
    lcd_pulse_en(hi2c, data);
}

static void lcd_send(I2C_HandleTypeDef *hi2c, uint8_t value, uint8_t mode) {
    lcd_write_nibble(hi2c, value & 0xF0, mode);          /* high nibble */
    lcd_write_nibble(hi2c, (value << 4) & 0xF0, mode);   /* low nibble  */
}

void LCD_Init(I2C_HandleTypeDef *hi2c) {
    HAL_Delay(50);                          /* power-on wait */
    lcd_write_nibble(hi2c, 0x30, 0); HAL_Delay(5);
    lcd_write_nibble(hi2c, 0x30, 0); HAL_Delay(1);
    lcd_write_nibble(hi2c, 0x30, 0); HAL_Delay(1);
    lcd_write_nibble(hi2c, 0x20, 0);            /* switch to 4-bit mode */
    lcd_send(hi2c, 0x28, 0);                  /* 4-bit, 2 lines, 5x8 font */
    lcd_send(hi2c, 0x0C, 0);                  /* display on, cursor off */
    lcd_send(hi2c, 0x06, 0);                  /* entry mode: increment, no shift */
    LCD_Clear(hi2c);
}

void LCD_Clear(I2C_HandleTypeDef *hi2c) {
    lcd_send(hi2c, 0x01, 0);
    HAL_Delay(2);
}

void LCD_SetCursor(I2C_HandleTypeDef *hi2c, uint8_t col, uint8_t row) {
    uint8_t row_offsets[] = { 0x00, 0x40 };
    lcd_send(hi2c, 0x80 | (col + row_offsets[row & 1]), 0);
}

void LCD_WriteChar(I2C_HandleTypeDef *hi2c, char c) {
    lcd_send(hi2c, (uint8_t)c, RS);
}

void LCD_WriteString(I2C_HandleTypeDef *hi2c, const char *str) {
    while (*str) LCD_WriteChar(hi2c, *str++);
}
