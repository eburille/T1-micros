#include <avr/io.h>
#include "led.h"

#define LED_PIN              PA3
#define LED_PORT           PORTA
#define LED_CONFIG_PORT     DDRA

void init_led(){
    LED_CONFIG_PORT |= (1 << LED_PIN);
}

void liga_led(){
    LED_PORT |= (1 << LED_PIN);
}

void desl_led(){
    LED_PORT &= ~(1 << LED_PIN);
}

void toggle_led(){
    LED_PORT ^= (1 << LED_PIN);
}
