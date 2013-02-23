#include <inttypes.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include <stdlib.h>
#include "usart.h"


#include "main.h"

#include "avr_mcu_section.h"
AVR_MCU(F_CPU, "atmega88pa");
AVR_MCU_SIMAVR_COMMAND(&GPIOR0);

const struct avr_mmcu_vcd_trace_t _mytrace[]  _MMCU_ = {
	{ AVR_MCU_VCD_SYMBOL("UDR0"), .what = (void*)&UDR0, },
	{ AVR_MCU_VCD_SYMBOL("UDRE0"), .mask = (1 << UDRE0), .what = (void*)&UCSR0A, },
	{ AVR_MCU_VCD_SYMBOL("GPIOR1"), .what = (void*)&GPIOR1, },
	{ AVR_MCU_VCD_SYMBOL("Parity"), .mask = (1 << TXB80), .what = (void*)&UCSR0B, },
	{ AVR_MCU_VCD_SYMBOL("SEND_ENABLE"), .mask = (1 << PORTD5), .what = (void*)&PORTD, },
};




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
        uint16_t time;
	while (count--) {
		led_w = count; //DEBUG
		time = count*count;
		while (time--) _delay_ms(10); //make sleeping longer each time
		led_b = 10;
		//send packet for address request
		uint8_t buffer[] = {id};
		USART_send_package(0,sizeof buffer,buffer);
		led_g = 10; //DEBUG
				
		//receive data packet from server with address response
		if ( ! USART_receive_package(0,buffer)) continue;
		led_w = 40; //DEBUG
		if (buffer[0] == id) {
			led_w = 0; //DEBUG
			address = (buffer[1]<<8|buffer[2]);
			//send ack
			USART_send_package(0,sizeof buffer,buffer);
			return 1;
		}
	}
	return 0;
}
int main(void)
{
	// this tell simavr to put the UART in loopback mode
	GPIOR0 = SIMAVR_CMD_UART_LOOPBACK;

	TCCR0B |= (1<<CS00);
	TIMSK0 |= (1<<TOIE0);

	PORTC &= ~((1<<PORTC2)|(1<<PORTC3)|(1<<PORTC4)|(1<<PORTC5));
	DDRC |= (1<<PORTC2)|(1<<PORTC3)|(1<<PORTC4)|(1<<PORTC5);


	USART_Init();
	

	// this tells simavr to start the trace
	GPIOR0 = SIMAVR_CMD_VCD_START_TRACE;


	sei();
	

	led_r = 0;
	led_g = 0;
	led_b = 0;
	led_w = 0;
	
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

	USART_send_package(0,sizeof buffer,buffer);

	uint16_t size = USART_receive_package(0,buffer);
	USART_send_package(0,size,buffer);
}




