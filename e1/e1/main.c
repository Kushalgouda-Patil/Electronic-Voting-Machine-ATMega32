#include <avr/io.h>
#include <avr/interrupt.h>
ISR(ADC_vect)
{
	PORTC=ADCL;
	PORTD=ADCH;
	if(ADC>0xB4)
	{
		PORTB|=0XFF;
	}
	else
	{
		PORTB=0X00;
	}
	ADCSRA|=(1<<ADSC);
}
int main(void)
{
	DDRD=0XFF;
	DDRB=0XFF;
	DDRC=0XFF;
	DDRA=0;
	sei();
	ADCSRA=0X8F;
	ADMUX=0XC0;
	ADCSRA|=(1<<ADSC);
	while(1)
	{
		
	}
	return 0;
}