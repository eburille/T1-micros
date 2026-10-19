#ifndef LCD_LIB__H
#define LCD_LIB__H

#define PRIMEIRA_LINHA 0x80
#define SEGUNDA_LINHA  0xC0

void lcd_init();
void envia_string(char string[16]);
void limpa_lcd();
void lcd_cmd (unsigned char cmd);
void lcd_data(unsigned char disp_data);

#endif