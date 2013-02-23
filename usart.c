#include <avr/io.h>
#include <util/delay.h>

#include <avr/interrupt.h>
#include <util/crc16.h>

#include "main.h"
#include "usart.h"

#define UART_RXBUFSIZE 250
#define USE_2X

volatile static uint8_t rxbuf0[UART_RXBUFSIZE];
volatile static uint8_t *volatile rxhead0, *volatile rxtail0;
//volatile uint8_t xon = 0;


ISR (USART_RX_vect)
{
	UCSR0B &= ~(1 << RXCIE0);
	asm volatile("sei");

	int diff;
	uint8_t c;
	c=UDR0;
	diff = rxhead0 - rxtail0;
	if (diff < 0) diff += UART_RXBUFSIZE;
	if (diff < UART_RXBUFSIZE -1)
	{
		*rxhead0 = c;
		++rxhead0;
		if (rxhead0 == (rxbuf0 + UART_RXBUFSIZE)) rxhead0 = rxbuf0;
//		if((diff > 100)&&(xon==0))
//		{
//			xon=1;
//			//set the CTS pin
//		}
	}
	UCSR0B |= (1 << RXCIE0);
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
	
	//Enable Interrupts
	//UCSR0B |= (1 << RXCIE0);


	rxhead0 = rxtail0 = rxbuf0;

}



void USART_putc (char c)
{
	loop_until_bit_is_set(UCSR0A, UDRE0);
	UDR0 = c;
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

void USART_send_byte (unsigned char data, unsigned char parity)
{
    // Warten, bis UDR bereit ist für einen neuen Wert
    while (!(UCSR0A & (1 << UDRE0)))
        ;
    UCSR0B &= ~(1<<TXB80);
    if ( parity )
      UCSR0B |= (1<<TXB80);

    // UDR schreiben startet die Übertragung      
    UDR0 = data;
}

void USART_send (uint16_t *s, unsigned n) {
  //  loop until *s != NULL
  if (n<5) return; //not an package

  //send address
  USART_send_byte(*s,1);
  n--; s++;
  USART_send_byte(*s,1);
  n--; s++;
  
  while (n--) {
    USART_send_byte(*s,0);
    s++;
  }
}

void USART_send_package (uint16_t address, uint16_t data_len, uint8_t data[]) {

  PORTD |= (1<<PORTD5); //send mode for MAX
  uint16_t crc = checkcrc(data, data_len);
  
  //send address
  USART_send_byte(address>>8,1);
  USART_send_byte(address,1);
  
  //send data_len
  USART_send_byte(data_len>>8,0);
  USART_send_byte(data_len,0);
  
  //send data
  uint16_t i = data_len;
  while (i--) {
    USART_send_byte(data[data_len-1-i],0);
  }
  
  USART_send_byte(crc>>8,0);
  USART_send_byte(crc,0);

}


/*
RECEIVING OPERATIONS
*/

//returnes 1 if ok, 0 if error, 
uint8_t USART_receive_byte(uint8_t *byte, uint8_t wanted_parity)
{
  unsigned char status, resh, resl;
  /* Wait for data to be received */
  while ( !(UCSR0A & (1<<RXC0)) )
  ;
  /* Get status and 9th bit, then data */
  /* from buffer */
  status = UCSR0A;
  resh = UCSR0B;
  resl = UDR0;
  
  /* If error, return -1 */
  if ( status & ((1<<DOR0)) ) return 0;
  
  resh = (resh >> 1) & 0x01;
  if (wanted_parity != resh) {
	led_g++;
	return 0;
  }
  
  byte = &resl;
  return 1;
}

//try to get two bytes with parity 1
uint16_t USART_receive_address(void)
{
  uint8_t address1, address2;
  uint8_t byte;
  while(1) {
    
    if (USART_receive_byte(&byte,1)) {
      address1 = byte;
      if (USART_receive_byte(&byte, 1)) {
        address2 = byte;
        return (address1<<8|address2);
      }
    }
  }
  return 0;
}


//return >=1 if succesful (data len), return 0 if not, e.g. crcr fails
uint16_t USART_receive_package(uint16_t address, uint8_t *data)
{
  PORTD &= ~(1<<PORTD5); //receive mode for MAX
  uint8_t byte, i;
  uint16_t data_len = 0, crc;

  while(1) {
    if (USART_receive_address() != address) continue;
    
    led_b++;

    if(! USART_receive_byte(&byte, 0)) return 0;
    data_len = (byte << 8);
    if(! USART_receive_byte(&byte, 0)) return 0;
    data_len |= byte;
    led_w++;
    uint8_t buffer[data_len];
    for (i = 0; i < data_len; i++) {
      if(! USART_receive_byte(&byte, 0)) return 0;
      buffer[i] = byte;
    }
    
    if(! USART_receive_byte(&byte, 0)) return 0;
    crc = byte<<8;
    if(! USART_receive_byte(&byte, 0)) return 0;
    crc |= byte;
    
    if (crc != checkcrc(buffer, sizeof buffer)) return 0;
    
    data = buffer;
    return data_len;
  }
}
