#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#ifndef F_CPU
#define F_CPU 16000000UL
#endif

// F_CPU / 
#define UART_BAUD_SETTING (((F_CPU / 8 / UART_BAUDRATE ) -1 ) / 2)

#define BUFFER_SIZE 256
char g_uart_buffer[BUFFER_SIZE];
uint8_t g_uart_buffer_index = 0;

const char user[] = "afaure";
const char pass[] = "goodpass";

void uart_init()
{
	//set baudrate
	UBRR0H = UART_BAUD_SETTING >> 8;
	UBRR0L = UART_BAUD_SETTING;

	//enable transmitter and receiver
	UCSR0B = 1 << TXEN0 | 1 << RXEN0;

	//enable RX complete interrupt
	// UCSR0B |= 1 << RXCIE0;

	//set frame format (8data, no parity, 1 stop)
	UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);

	// sei();

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

char uart_rx(void)
{
	while (((UCSR0A >> RXC0) & 1) == 0);

	char ret = UDR0;
	return (ret);
}

void receive_word(char * buffer, uint8_t * index, uint8_t no_echo)
{
	uint8_t finished = 0;

	*index = 0;
	while (finished == 0) //read until we receive \n adn stop writing on screen when buffer is full
	{
		buffer[*index] = uart_rx();

		switch(buffer[*index])
		{
		case 127:
		{
			if (*index != 0)
			{
				uart_printstr("\b \b"); //erase last printed char
				(*index)--; //go back one index
			}
			break;
		}
		case '\r':
		{
			buffer[*index] = '\0';
			uart_printstr("\r\n");
			finished = 1;
			break;
		}
		default:
		{
			if (*index < BUFFER_SIZE - 2)
			{
				if (no_echo)
					uart_tx('*');
				else
					uart_tx(buffer[*index]);
				(*index)++;
			}
			break;
		}
		}
	}
}

int8_t ft_strcmp(const char * str1, const char * str2)
{
	while (*str1 && *str2)
	{
		if (*str1 != *str2)
			return (*str1 - *str2);
		str1++;
		str2++;
	}
	return *str1 - *str2;
}

void blink_leds()
{
	DDRB |= 1 << PB0 | 1 << PB1 | 1 << PB2 | 1 << PB4;

	PORTB = 0;

	while (1)
	{
		PORTB = 1 << PB0 | 1 << PB2;
		_delay_ms(100);
		PORTB = 1 << PB1 | 1 << PB4;
		_delay_ms(100);
	}
}

int main()
{
	uart_init();

	char user_buffer[BUFFER_SIZE];
	uint8_t user_buffer_index = BUFFER_SIZE - 1;
	while (user_buffer_index != 0) {
		user_buffer[user_buffer_index--] = '\0';
	}

	char pass_buffer[BUFFER_SIZE];
	uint8_t pass_buffer_index = BUFFER_SIZE - 1;
	while (pass_buffer_index != 0) {
		pass_buffer[pass_buffer_index--] = '\0';
	}

	while (1)
	{
		//first step, prompt
		uart_printstr("Enter your login:\r\n");
		uart_printstr("\tusername: ");
		//second step, username
		receive_word(user_buffer, &user_buffer_index, 0);

		uart_printstr("\tpassword: ");
		//third step, password
		receive_word(pass_buffer, &pass_buffer_index, 1);

		//fourth step error or login
		if (ft_strcmp(user_buffer, user) == 0
			&& ft_strcmp(pass_buffer, pass) == 0)
		{
			uart_printstr("Welcome aboard Captain !\r\n");
			blink_leds();
			while(1);
		}
		else
			uart_printstr("Bad user/password\r\n");
	}
}

ISR(USART_UDRE_vect)
{
	if (g_uart_buffer[g_uart_buffer_index] != '\0')
	{
		//sending a new char in UART port
		//when it is finished it will trigger a new interrupt
		UDR0 = g_uart_buffer[g_uart_buffer_index];
		g_uart_buffer_index++;
	}
	else //if sending finished, disable interrupt
		UCSR0B &= ~(1 << UDRIE0);
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
