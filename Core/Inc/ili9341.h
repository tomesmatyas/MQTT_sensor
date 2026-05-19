#ifndef ILI9341_H
#define ILI9341_H

#include "main.h"


#define ILI9341_BLACK       0x0000
#define ILI9341_WHITE       0xFFFF
#define ILI9341_RED         0xF800
#define ILI9341_GREEN       0x07E0
#define ILI9341_BLUE        0x001F
#define ILI9341_YELLOW      0xFFE0
#define ILI9341_CYAN        0x07FF
#define ILI9341_MAGENTA     0xF81F

void ILI9341_Init(SPI_HandleTypeDef *hspi);
void ILI9341_FillScreen(uint16_t color);
void ILI9341_DrawPixel(uint16_t x, uint16_t y, uint16_t color);
void ILI9341_DrawChar(uint16_t x, uint16_t y, char c, uint16_t color, uint16_t bg, uint8_t size);
void ILI9341_DrawText(uint16_t x, uint16_t y, const char *str, uint16_t color, uint16_t bg, uint8_t size);
void ILI9341_FillRectangle(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);

#endif
