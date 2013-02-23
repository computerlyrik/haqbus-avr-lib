#ifndef _USART_H
#define _USART_H

void USART_Init (void);

void USART_putc (char c);
void USART_send_byte (unsigned char data, unsigned char parity);
void USART_send (uint16_t *s, unsigned n);
void USART_send_package (uint16_t address, uint16_t data_len, uint8_t* data);

uint8_t USART_receive_byte(uint8_t *byte, uint8_t wanted_parity);
uint16_t USART_receive_address(void);
uint16_t USART_receive_package(uint16_t address, uint8_t* data);
#endif

