#define F_CPU 8000000UL
#include <avr/io.h>
#include<string.h>
#include<stdlib.h>
#include <util/delay.h> //delay header
#define LCD_DPRT PORTD //LCD DATA PORT
#define LCD_DDDR DDRD //LCD DATA DDR
#define LCD_CPRT PORTC //LCD COMMANDS PORT
#define LCD_CDDR DDRC //LCD COMMANDS DDR

#define CU_DDDR DDRB
#define CU_PIN PINB
#define CU_TOTAL 0
#define CU_RESULT 1
#define CU_CLEAR 2
#define CU_READY 3


#define LCD_RS 5
#define LCD_RW 6
#define LCD_EN 7

#define KEY_PRT PORTA //keyboard PORT
#define KEY_DDR DDRA //keyboard DDR
#define KEY_PIN PINA
unsigned char keypad[4][4] ={ {'7','8','9','/'},{'4','5','6','*'},{'1','2','3','-'},{' ','0','=','+'}};
//unsigned char keypad[4][4] ={ {'7','8','9','/'},{'1','4','7','*'},{'2','5','8','0'},{' ','6','9','#'}};

void lcdCommand( unsigned char cmnd )
{
	LCD_DPRT = cmnd;
	LCD_CPRT &= ~ ((1<<LCD_RS)|(1<<LCD_RW)); //RS = 0 RW = 0
	LCD_CPRT |= (1<<LCD_EN); //EN = 1
	_delay_us(100);
	LCD_CPRT &= ~ (1<<LCD_EN); //EN = 0 for H-to-L pulse
	_delay_us(100);
}
void lcdData( unsigned char data )
{
	LCD_DPRT = data; //send data
	LCD_CPRT &= ~ (1<<LCD_RW); //RW = 0 for write
	LCD_CPRT |= ((1<<LCD_RS)|(1<<LCD_EN)); //RS = 1 for data //EN = 1
	_delay_us(100);
	LCD_CPRT &= ~ (1<<LCD_EN); //EN = 0 for H-to-L pulse
	_delay_us(100);
}

void lcd_init()
{
	LCD_DDDR = 0xFF;
	LCD_CDDR = 0xFF;
	LCD_CPRT &=~(1<<LCD_EN); //LCD_EN = 0
	_delay_us(2000);
	lcdCommand(0x38); //init. LCD 2 line, 5 × 7 matrix
	lcdCommand(0x0E); //display on, cursor on
	lcdCommand(0x01); //clear LCD
	_delay_us(2000);
	lcdCommand(0x06); //shift cursor right
}
void lcd_print(char *str)
{
	
	unsigned int i=0;
	while(str[i]!='\0')
	{
		lcdData(str[i++]);
	}
}
void keyboard_init()
{
	DDRD = 0xFF;
	KEY_DDR = 0xF0;
	KEY_PRT = 0xFF;
}
char keyfind()
{
	unsigned char rowloc, colloc;
	
	while(1)
	{
		while(1)
		{
			keyboard_init();
			KEY_PRT = 0xEF; //ground row 0
			colloc = (KEY_PIN & 0x0F); //read the columns
			if(colloc != 0x0F) //column detected
			{
				rowloc = 0;
				break;
			}
			
			
			KEY_PRT = 0xDF; //ground row 1
			colloc = (KEY_PIN & 0x0F); //read the columns
			if(colloc != 0x0F) //column detected
			{
				rowloc = 1;
				break;
			}
			
			
			KEY_PRT = 0xBF; //ground row 2
			colloc = (KEY_PIN & 0x0F); //read the columns
			if(colloc != 0x0F)
			{
				rowloc = 2;
				break;
			}
			
			
			KEY_PRT = 0x7F; //ground row 3
			colloc = (KEY_PIN & 0x0F);
			if(colloc != 0x0F) //column detected
			{
				rowloc = 3;
				break;
			}
		}
		if(colloc == 0x0E)
		{
			return(keypad[rowloc][0]);
		}
		else if(colloc == 0x0D)
		{
			return (keypad[rowloc][1]);
		}
		else if(colloc == 0x0B)
		{
			return (keypad[rowloc][2]);
		}
		else
		{
			return (keypad[rowloc][3]);
		}
	}
}
int c1,c2,c3;
void accept_votes()
{
	lcd_print("Ready");
	unsigned char ch=keyfind();
	if(ch=='1')
	{
		++c1;
		lcdCommand(0x01);
		return;
	}
	else if(ch=='2')
	{
		++c2;
		lcdCommand(0x01);
		return;
	}
	else if(ch=='3')
	{
		++c3;
		lcdCommand(0x01);
		return;
	}
	else
	{
		lcdCommand(0x01);
		return;
	}
	
}
int main(void)
{
	c1=0;c2=0;c3=0;
	lcd_init();
	CU_DDDR=0x00;
	lcdCommand(0X80);
	CU_PIN=0xff;
	KEY_PIN=0xff;
	while (1)
	{
		if((PINB&(1<<3))==0)
		{
			accept_votes();
			_delay_us(1000000);
		}
		else if((PINB&(1<<CU_RESULT))==0)
		{
			char str[20];
			itoa(c1,str,10);
			lcd_print("Candidate 1: ");
			lcd_print(str);
			_delay_us(4000000);
			lcdCommand(0x01);
		}
		else if((PINB&(1<<CU_TOTAL))==0)
		{
			int total=c1+c2+c3;
			char str[20];
			itoa(total,str,10);
			lcd_print("Total ");
			lcd_print(str);
			_delay_us(4000000);
			lcdCommand(0x01);
			
		}
		else
		{
			
			continue;
		}
		
	
	}
}

