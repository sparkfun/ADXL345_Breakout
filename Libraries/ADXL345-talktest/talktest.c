/*
   Accelerometer, ADXL345, test code
	
	8/20/2009
	Viliam Klein 
	
	This code sets up the power and mode registers and then prints out the data registers forever.
	UART is set to 9600 8N1
	
	using ATMega328 on Arduino Pro Board 3.3V, 8MHz board
	
*/

#include <avr/io.h>
#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#define FOSC 8000000
#define BAUD 9600
#define MYUBRR 103

#define sbi(var, mask)   ((var) |= (uint8_t)(1 << mask))
#define cbi(var, mask)   ((var) &= (uint8_t)~(1 << mask))

//ADXL345
#define adxl 2 //PORTB




//Define functions
//======================
void ioinit(uint8_t baud);      //Initializes IO
void delay_ms(uint16_t x); //General purpose delay

static int uart_putchar(char c, FILE *stream);
uint8_t uart_getchar(void);
static FILE mystdout = FDEV_SETUP_STREAM(uart_putchar, NULL, _FDEV_SETUP_WRITE);


void iniSPI(void);
void send_byte(uint8_t byte);
void send_dummy (void);
void resetCS(uint16_t delay);


//======================

int main (void)
{
    uint8_t recv1=0;
	uint8_t recv2=0;
	uint8_t byte;
	uint8_t data[6];
		
	ioinit(103);
	iniSPI();
	
	printf("\nStart\n");
	
	delay_ms(40000);//long delay
	
	//waits for power register to respond correctly
	while(byte != 0x28)
	{
		cbi(PORTB, adxl);
		send_byte(0x2D);
		send_byte(0x28);
		sbi(PORTB, adxl);
		
		delay_ms(5);
		
		cbi(PORTB, adxl);
		send_byte(0xAD);
		send_byte(0x00);
		byte = SPDR;
		sbi(PORTB, adxl);
		
		printf("Power is: %x\n", byte);
		
		delay_ms(1000);
	}
	
	cbi(PORTB, adxl);
	send_byte(0x31);
	send_byte(0x08);
	sbi(PORTB, adxl);
	
	delay_ms(5);
	
	cbi(PORTB, adxl);
	send_byte(0xB1);
	send_byte(0x00);
	byte = SPDR;
	sbi(PORTB, adxl);
	
	printf("Format is: %x\n", byte);
	
	cbi(PORTB, adxl);
	send_byte(0x38);
	send_byte(0x00);
	sbi(PORTB, adxl);
	
	delay_ms(5);
	
	cbi(PORTB, adxl);
	send_byte(0xB8);
	send_byte(0x00);
	byte = SPDR;
	sbi(PORTB, adxl);
	
	printf("FIFO is: %x\n", byte);
		
	
	printf("data is: ");
	
	while(1)
	{
	printf("\n");
	
		cbi(PORTB, adxl);
		send_byte(0xF2);
		
		for(uint8_t i=0; i<6; i++)
		{
			send_byte(0x00);
			data[i] = SPDR;
			
		}
		
		sbi(PORTB, adxl);
		
		for(int i=0; i<6; i++)
		{
			printf("%x ",data[i]);
		}
		
		delay_ms(4000);
	
	}
	
	return(0);
}

 void send_dummy(void)
{

	SPDR = 0xAA;
	while(!(SPSR & (1<<SPIF)));

}

void send_byte(uint8_t byte)
{

	SPDR = byte;
	while(!(SPSR & (1<<SPIF)));

}

void iniSPI(void)
{
	// Set MOSI, SCK, and SS output, all others input 
	sbi(DDRB, adxl);	//acc1 cs output
	
	
	sbi(PORTB, adxl);
	
	cbi(SPCR, CPHA);
	SPCR = (1<<SPE)|(1<<MSTR)|(1<<SPR1)|(1<<SPR0)|(1<<CPHA)|(1<<CPOL);
	cbi(SPCR, SPR1);
    cbi(SPCR, SPR0);
	//sbi(SPSR, 0);
    delay_ms(1);
}


void ioinit (uint8_t baud)
{
    //1 = output, 0 = input
    DDRB = 0b11101111; //All inputs
    DDRC = 0b11111111; //All outputs
    DDRD = 0b11001110;  //PORTD (RX on PD0)
	//DDRA = 0b11111111;
	//CLKPR = (1 << CLKPCE);
	//CLKPR = (3<<CLKPS);
	
	UBRR0H = MYUBRR >> 8;
    UBRR0L = MYUBRR;
    UCSR0B = (1<<RXEN0)|(1<<TXEN0);
	UCSR0A = (1<<U2X0);		
		
    stdout = &mystdout; //Required for printf init

}

static int uart_putchar(char c, FILE *stream)
{
    if (c == '\n') uart_putchar('\r', stream);
  
    loop_until_bit_is_set(UCSR0A, UDRE0);
    UDR0 = c;
    
    return 0;
}

uint8_t uart_getchar(void)
{
    while( !(UCSR0A & (1<<RXC0)) );
    return(UDR0);
}


//General short delays
void delay_ms(uint16_t x)
{
  uint8_t y, z;
  for ( ; x > 0 ; x--){
    for ( y = 0 ; y < 90 ; y++){
      for ( z = 0 ; z < 6 ; z++){
        asm volatile ("nop");
      }
    }
  }
}
