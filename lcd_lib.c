#include <avr/io.h>
#include "lcd_lib.h"
#include "delay.h"

//Definicoes pinos lcd
#define LCD_comando_porta PORTC
#define RS PC0
#define E PC1
#define LCD_dado_porta PORTD


void lcd_cmd (unsigned char cmd){
	LCD_comando_porta &= ~(1<<RS);
	
	LCD_dado_porta = (LCD_dado_porta & 0x0F) | (cmd & 0xF0);
	LCD_comando_porta |= (1<<E);
	delay_1ms();
	LCD_comando_porta &= ~(1<<E);
	
	LCD_dado_porta = (LCD_dado_porta & 0x0F) | ((cmd<<4) & 0xF0);
	LCD_comando_porta |= (1<<E);
	delay_1ms();
	LCD_comando_porta &= ~(1<<E);
	
}


void lcd_data(unsigned char disp_data){
	LCD_comando_porta |= (1<<RS);
	
	LCD_dado_porta = (LCD_dado_porta & 0x0F) | (disp_data & 0xF0);
	LCD_comando_porta |= (1<<E);
	delay_1ms();
	LCD_comando_porta &= ~(1<<E);
	
	LCD_dado_porta = (LCD_dado_porta & 0x0F) | ((disp_data<<4) & 0xF0);
	LCD_comando_porta |= (1<<E);
	delay_1ms();
	LCD_comando_porta &= ~(1<<E);
	
}

void lcd_init (){
    DDRC |= (1 << RS) | (1 << E);
    DDRD |= 0xF0;

	LCD_comando_porta &= ~(1<<RS);
	
	LCD_dado_porta &= 0x0F;
	LCD_dado_porta |= 0X20;
	LCD_comando_porta |= (1<<E);
	LCD_comando_porta &= ~(1<<E);
	delay_1ms();
	
	 lcd_cmd(0x28);
	 lcd_cmd(0x0C);
	 lcd_cmd(0x06);
}

void envia_string(char string[16]){
	int i;
	for (i = 0; i < 16; i++){
		if (string[i] == '\0'){
			return;
		}
		
		lcd_data(string[i]);
	}
}


void limpa_lcd(){
	lcd_cmd(0x01); // limpa o display
	delay_1ms();
}