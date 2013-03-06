#include <avr/io.h>
#include <util/delay.h>

#include <avr/interrupt.h>
#include <util/crc16.h>

#include "main.h"
#include "usart.h"
#include "fifo.h"

#define USE_2X


// FIFO-Objekte und Puffer für die Ein- und Ausgabe

#define BUFSIZE_IN  0x40 //64
uint8_t inbuf[BUFSIZE_IN];
fifo_t infifo;

ISR (USART_RX_vect)
{
    
	UCSR0B &= ~(1 << RXCIE0); //disable interrupt
    unsigned char status, resh, resl;
	/* Get status and 9th bit, then data */
	/* from buffer */
	status = UCSR0A;
	resh = UCSR0B;
	resl = UDR0;

	resh = (resh >> 1) & 0x01;
    _inline_fifo_put (&infifo, resh);
    _inline_fifo_put (&infifo, resl);

	UCSR0B |= (1 << RXCIE0); //reenable Interrupt

}


void USART_Init (void)
{
//	uint16_t ubrr = 21

	UBRR0H = 0;
//	UBRR0L = 4; //500000baud
//	UBRR0L = 10;//230400baud
//	UBRR0L = 21;//115200baud
	UBRR0L = 129; // 19200baud

//	#if USE_2X
	UCSR0A |= (1 << U2X0);	// enable double speed operation
//	#else
//	UCSR0A &= ~(1 << U2X0);	// disable double speed operation
//	#endif


	//set MAX into receive or send mode
	DDRD |= (1<<PORTD5);
	PORTD &= ~(1<<PORTD5); //receive - default
	//PORTD |= (1<<PORTD5); //send


	// flush receive buffer
	while ( UCSR0A & (1 << RXC0) ) UDR0;


	// set parity bit usage
	UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
	//UCSR0B &= ~(1 << UCSZ02); //8N1
	UCSR0B |= (1 << UCSZ02); //9N1

	/*
	disabled checking of error vector
	//disable parity bit checking
	UCSR0C &= ~((1<<UPM00) | (1 << UPM01));
	*/

	//Enable Receive and transmit
	UCSR0B |= (1 << RXEN0) | (1 << TXEN0);
	UCSR0B &= ~(1 << TXEN0); //disable transmit
	
	//Enable Receiver Interrupts
	UCSR0B |= (1 << RXCIE0);

	fifo_init (&infifo,   inbuf, BUFSIZE_IN);

}



/*
CRC CHECKER
*/

uint8_t checkcrc(uint8_t data[], uint16_t len)
{
    uint8_t crc = 0, i;

    for (i = 0; i < len; i++)
        crc = _crc16_update(crc, data[i]);

    return crc;
}

/*
SENDING OPERATIONS
*/


/*
RECEIVING OPERATIONS
*/


//return >=1 if succesful (data len), return 0 if not, e.g. crcr fails
uint16_t USART_receive_package(uint16_t address, uint8_t *data)
{
	PORTD &= ~(1<<PORTD5); //receive mode for MAX
	uint8_t byte, parity;
	uint16_t crc, i, myaddress = 0, data_len; 

while(1) {

parity = fifo_get_wait(&infifo);
if (!parity) continue;
byte = fifo_get_wait(&infifo);
address = byte << 8;

parity = fifo_get_wait(&infifo);
if (!parity) continue;
byte = fifo_get_wait(&infifo);
address |= byte;

if (! address == myaddress) continue;
led_r = 10;
//DATA_LEN
parity = fifo_get_wait(&infifo);
if (parity) continue;
byte = fifo_get_wait(&infifo);
data_len = byte << 8;
for (i = 0; i < byte; i++) {
          led_g=20;
               _delay_ms(100);
               led_g=0;
               _delay_ms(100);
       }


parity = fifo_get_wait(&infifo);
if (parity) continue;
byte = fifo_get_wait(&infifo);
data_len |= byte;
for (i = 0; i < byte; i++) {
          led_g=20;
               _delay_ms(100);
               led_g=0;
               _delay_ms(100);
       }


led_g=10;
//DATA
uint8_t buffer[data_len];
for (i = 0; i<data_len; i++) {
 parity = fifo_get_wait(&infifo);
 if (parity) break;
 byte = fifo_get_wait(&infifo);
 buffer[i] = byte;
}
if (i != (data_len-1) ) continue;
 
led_b=10;
//CRC
parity = fifo_get_wait(&infifo);
if (parity) continue;
byte = fifo_get_wait(&infifo);
crc = byte<<8;
parity = fifo_get_wait(&infifo);
if (parity) continue;
byte = fifo_get_wait(&infifo);
crc |= byte;

data = buffer;
return data_len;
 }
}
