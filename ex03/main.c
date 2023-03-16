#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>


// F_CPU / 
#define UART_BAUD_SETTING (((F_CPU / 8 / UART_BAUDRATE ) -1 ) / 2)

void uart_init()
{
	//set baudrate
	UBRR0H = UART_BAUD_SETTING >> 8;
	UBRR0L = UART_BAUD_SETTING;

	//enable transmitter and receiver
	UCSR0B = 1 << TXEN0 | 1 << RXEN0;

	//enable RX complete interrupt
	UCSR0B |= 1 << RXCIE0;

	//set frame format (8data, no parity, 1 stop)
	UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);

	sei();

}

void uart_tx(char c)
{
	//writing data to send register
	while(((UCSR0A >> UDRE0) & 1) == 0);
	UDR0 = c;
}

char uart_rx(void)
{
	while (((UCSR0A >> RXC0) & 1) == 0);

	char ret = UDR0;
	return (ret);
}

ISR(USART_RX_vect)
{
	char reception = UDR0;

	if (reception >= 'a' && reception <= 'z')
		reception -= 32;
	
	else if (reception >= 'A' && reception <= 'Z')
		reception += 32;
	uart_tx(reception);
}

int main()
{
	uart_init();

	while (1)
	{
		//nothing
	}
}
