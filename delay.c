#include <avr/io.h>

void delay_1ms(){
    TCNT0 = 6;  
    TIFR0 = 1;
    while ((TIFR0 & (1 << 0) )==0);
}