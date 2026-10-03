#ifndef LCD_LIB__H
#define LCD_LIB__H

void lcd_init();
void envia_string(char string[16]);
void limpa_lcd();
void lcd_cmd (unsigned char cmd);
void lcd_data(unsigned char disp_data);

#endif