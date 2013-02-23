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
uint16_t inbuf[BUFSIZE_IN];
fifo_t infifo;

#define BUFSIZE_OUT 0x40 //64
uint16_t outbuf[BUFSIZE_OUT];
fifo_t outfifo;


ISR (USART_UDRE_vect)
{

    // Warten, bis UDR bereit ist für einen neuen Wert
    if (outfifo.count > 0) {
	PORTD |= (1<<PORTD5); //send mode for MAX
	uint16_t fifo = _inline_fifo_get (&outfifo);
	UCSR0B &= ~(1<<TXB80);
	if ( (8>>fifo) & 0x01 ) //if parity is wanted
		UCSR0B |= (1<<TXB80);
	UDR0 = fifo & 0xFF;
    }
    else {
        UCSR0B &= ~(1 << UDRIE0);
	PORTD &= ~(1<<PORTD5); //back to receive mode for MAX
   }
}


uint16_t rx_address;
uint8_t rx_state; //0 no address, 1 one address byte, 2 full address
ISR (USART_RX_vect)
{

	unsigned char status, resh, resl;
	/* Get status and 9th bit, then data */
	/* from buffer */
	status = UCSR0A;
	resh = UCSR0B;
	resl = UDR0;
	/* If error, return -1 */
	if ( status & ((1<<DOR0)) ) return 0; //TODO ADD MORE ERRORCORRECTION

	resh = (resh >> 1) & 0x01;
	if (resh == 1) { //received an address
		if(rx_state > 2) rx_state=0;
		if (rx_state == 0) { //get first part of address
			rx_address = resl << 8;
			rx_state++;
		}
		else if (rx_state == 1 ) { //get second part of address
			rx_address |= resl;//TODO CHECK FOR OWN ADDRESS
			rx_state++;
		}
	} else if (rx_state == 2) {
		_inline_fifo_put (&infifo,UDR0);
	}
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
	
	//Enable Receiver Interrupts
	UCSR0B |= (1 << RXCIE0);

	fifo_init (&infifo,   inbuf, BUFSIZE_IN);
        fifo_init (&outfifo, outbuf, BUFSIZE_OUT);

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

void USART_send_package (uint16_t address, uint16_t data_len, uint8_t data[]) {

  uint16_t crc = checkcrc(data, data_len);
  
  //send address
  fifo_put(&outfifo,1<<8|address>>8);
  fifo_put(&outfifo,1<<8|address&0xFF);
  
  //send data_len
  fifo_put(&outfifo,data_len>>8);
  fifo_put(&outfifo,data_len&0xFF);
  
  //send data
  uint16_t i = data_len;
  while (i--) {
    fifo_put(&outfifo,data[data_len-1-i]);
  }
  
  fifo_put(&outfifo,crc>>8);
  fifo_put(&outfifo,crc&0xFF);

  UCSR0B |= (1 << UDRIE0);
}


/*
RECEIVING OPERATIONS
*/


//return >=1 if succesful (data len), return 0 if not, e.g. crcr fails
uint16_t USART_receive_package(uint16_t address, uint8_t *data)
{
	PORTD &= ~(1<<PORTD5); //receive mode for MAX
	uint8_t byte;
	uint16_t data_len = 0, crc, i;
	byte = fifo_get_wait(&infifo);
	data_len = (byte << 8);
	byte = fifo_get_wait(&infifo);
	data_len |= byte;
	led_r++;
	uint8_t buffer[data_len];
	for (i = 0; i < data_len; i++) {
	led_g=200;
	_delay_ms(100);
	led_g=0;
	_delay_ms(100);
		byte = fifo_get_wait(&infifo);
		buffer[i] = byte;
    }
	led_b++;
    byte = fifo_get_wait(&infifo);
    crc = byte<<8;
    byte = fifo_get_wait(&infifo);
    crc |= byte;
	led_w++;
//    if (crc != checkcrc(buffer, sizeof buffer)) return 0;

    data = buffer;
    return data_len;
}
