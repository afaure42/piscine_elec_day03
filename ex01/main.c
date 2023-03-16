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

	//enable transmitter
	UCSR0B = 1 << TXEN0;

	//set frame format (8data, no parity, 1 stop)
	UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);

}

void uart_tx(char c)
{
	//writing data to send register
	while(((UCSR0A >> UDRE0) & 1) == 0);
	UDR0 = c;
}

void uart_printstr(const char *str)
{
	while(*str)
	{
		uart_tx(*str);
		str++;
	}
}

int main()
{
	uart_init();


	//setting timer1 as CTC mode with 0.5 hertz frequency ( 2s )
	//and an interrupt to trigger the message transmission
	TCCR1A = 0;
	TCCR1B = 0;

	//base clock with a 1024 prescaler
	TCCR1B |= 1 << CS10 | 1 << CS12;

	//CTC mode 4 with OCR1A as TOP
	TCCR1B |= 1 << WGM12;

	OCR1A = F_CPU / 1024 * 2; // 0.5hz or 2 sec freq

	//enabling interrupt on COMPA
	TIMSK1 = 1 << OCIE1A;

	sei(); //setting global interrupt enable

	while (1)
	{
	}
}

ISR(TIMER1_COMPA_vect)
{
	uart_printstr("Hello World!\n\r");
}
