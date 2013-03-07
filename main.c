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

uint8_t request_address(void) {
	uint8_t count = 23;
	uint8_t id = rand() % 255;
        uint16_t time, size;
	while (count--) {
		time = count*count;
		while (time--) _delay_ms(10); //make sleeping longer each time
		//send packet for address request
		USART_send_package(0,1,&id);
	    uint8_t buffer[12];
		//receive data packet from server with address response
		size= USART_receive_package(0,buffer);
		if (buffer[0] == id) {
			address = (buffer[1]<<8|buffer[2]);
			//send ack
			USART_send_package(0,size,buffer);
			return 1;
		}
	}
	return 0;
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
     if (!address) request_address();
            size = USART_receive_package(address,rgbw);
        led_r = rgbw[0];
        led_g = rgbw[1];
        led_b = rgbw[2];
        led_w = rgbw[3];
  }
}




