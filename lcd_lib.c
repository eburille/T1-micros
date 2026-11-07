#include <avr/io.h>
#include "lcd_lib.h"
#include "delay.h"

//Definicoes pinos lcd
#define LCD_comando_porta PORTC
#define RS PC0
#define E PC1
#define LCD_dado_porta PORTA

/// @brief Envia um comando para o LCD
/// @param cmd 
void lcd_cmd (unsigned char cmd){
	LCD_comando_porta &= ~(1<<RS);
	
	LCD_dado_porta = (LCD_dado_porta & 0x0F) | (cmd & 0xF0);
	LCD_comando_porta |= (1<<E);
	delay_1ms();
	LCD_comando_porta &= ~(1<<E);

	delay_1ms();

	LCD_dado_porta = (LCD_dado_porta & 0x0F) | ((cmd<<4) & 0xF0);

	LCD_comando_porta |= (1<<E);
	delay_1ms();
	LCD_comando_porta &= ~(1<<E);
	delay_1ms();
}

/// @brief Coloca um caracter no display e incrementa o cursor
/// @param disp_data 
void lcd_data(unsigned char disp_data){
	LCD_comando_porta |= (1<<RS);
	delay_1ms();

	LCD_dado_porta = (LCD_dado_porta & 0x0F) | (disp_data & 0xF0);
	LCD_comando_porta |= (1<<E);
	delay_1ms();
	LCD_comando_porta &= ~(1<<E);
	
	delay_1ms();

	LCD_dado_porta = (LCD_dado_porta & 0x0F) | ((disp_data<<4) & 0xF0);
	
	LCD_comando_porta |= (1<<E);
	delay_1ms();
	LCD_comando_porta &= ~(1<<E);
	delay_1ms();
}

/// @brief Inicializa LCD
void lcd_init (){
    DDRC |= (1 << RS) | (1 << E);
    DDRA |= 0xF0;

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

/// @brief Escreve uma string no LCD
/// @param string 
void envia_string(char string[16]){
	int i;
	for (i = 0; i < 16; i++){
		if (string[i] == '\0'){
			return;
		}
		
		lcd_data(string[i]);
	}
}

/// @brief Limpa a tela do display
void limpa_lcd(){
	lcd_cmd(0x01); // limpa o display
	delay_1ms();
}