#include <inttypes.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include <stdlib.h>
#include "usart.h"


#include "main.h"


uint8_t cycle = 0;

volatile uint8_t led_r = 0;
volatile uint8_t led_g = 0;
volatile uint8_t led_b = 0;
volatile uint8_t led_w = 0;

volatile uint16_t address = 0;

// interpolate a PWM
ISR (TIMER0_OVF_vect)
{
	cycle++;
	
	//set all leds on if in on cycle
	PORTC |= (((led_w > cycle)<<PORTC2)
					|	((led_b > cycle)<<PORTC3)
					|	((led_g > cycle)<<PORTC4)
					|	((led_r > cycle)<<PORTC5));

	//set all leds off if outside cycle
	PORTC &= ~(((led_w < cycle)<<PORTC2)
					|	((led_b < cycle)<<PORTC3)
					|	((led_g < cycle)<<PORTC4)
					|	((led_r < cycle)<<PORTC5));
}

int main(void)
{
	TCCR0B |= (1<<CS00);
	TIMSK0 |= (1<<TOIE0);

	PORTC &= ~((1<<PORTC2)|(1<<PORTC3)|(1<<PORTC4)|(1<<PORTC5));
	DDRC |= (1<<PORTC2)|(1<<PORTC3)|(1<<PORTC4)|(1<<PORTC5);


	USART_Init();
	

	sei();
	
	//init random generator
	unsigned short seed = 0;
	unsigned short *p = (unsigned short*) (RAMEND+1);
	extern unsigned short __heap_start;
	while (p >= &__heap_start + 1)
		seed ^= * (--p);
	srand(seed);

	char c = 'f';
	uint8_t buffer[] = {'f','o','o'};
	uint8_t counter;

	uint8_t rgbw[255];
	uint16_t size,i;

	while (1) {
		size = USART_receive_package(0,rgbw);
        for (i=0; i<rgbw[0]; i++) {
           led_b = 10;
            _delay_ms(100);
           led_b=0;
            _delay_ms(100);
        }

        led_r = rgbw[0];
        led_g = rgbw[1];
        led_b = rgbw[2];
        led_w = rgbw[3];
	//	USART_send_package(0,size,buffer);
led_w++;
	}
}




