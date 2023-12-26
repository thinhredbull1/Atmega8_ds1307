/*
 * main.c
 *
 *
 *  Author: WINDOWS
 */ 

#define RS PORTB0
#define LCD_Port PORTD
#define EN PORTD6
#define RW PORTD7
#define LCD_CLEARDISPLAY 0x01
#define LCD_RETURNHOME 0x02
#define LCD_ENTRYMODESET 0x04
#define LCD_DISPLAYCONTROL 0x08
#define LCD_CURSORSHIFT 0x10
#define LCD_FUNCTIONSET 0x20
#define LCD_SETCGRAMADDR 0x40
#define LCD_SETDDRAMADDR 0x80

// flags for display entry mode
#define LCD_ENTRYRIGHT 0x00
#define LCD_ENTRYLEFT 0x02
#define LCD_ENTRYSHIFTINCREMENT 0x01
#define LCD_ENTRYSHIFTDECREMENT 0x00

// flags for display on/off control
#define LCD_DISPLAYON 0x04
#define LCD_DISPLAYOFF 0x00
#define LCD_CURSORON 0x02
#define LCD_CURSOROFF 0x00
#define LCD_BLINKON 0x01
#define LCD_BLINKOFF 0x00

// flags for display/cursor shift
#define LCD_DISPLAYMOVE 0x08
#define LCD_CURSORMOVE 0x00
#define LCD_MOVERIGHT 0x04
#define LCD_MOVELEFT 0x00

// flags for function set
#define LCD_8BITMODE 0x10
#define LCD_4BITMODE 0x00
#define LCD_2LINE 0x08
#define LCD_1LINE 0x00
#define LCD_5x10DOTS 0x04
#define LCD_5x8DOTS 0x00
#include <avr/io.h>
#include <avr/interrupt.h>
#include "ds1307.h"
#define HOUR 0
#define MINUTE 1
#define SECOND 2
#define YEAR 5
#define MONTH 4
#define DAY 3
#define setup_state 2
#define alarm_st 1
#define normal 0
volatile uint8_t date_now[6];
volatile uint8_t date_alarm[6];
volatile uint8_t state_now=0;
volatile uint8_t change_cur=0;
volatile uint8_t save_press=0;
char space=':';
char hou[3];
char min_[2];
char sec_[2];
char space2='-';
char day_[2];
char mon_[2];
char year_[2];
char data_display[8]="TIME:";
char data_display2[8]="DATE:";
char year_bonus[3]="20";
char setup_display[16]="Setup";
char alarm_display[16]="Alarm";
void ds1307_Set(volatile uint8_t *date)
{
	ds1307_setdate(date[YEAR],date[MONTH],date[DAY],date[HOUR],date[MINUTE],date[SECOND]);
}
void Toggle_Enable() {
	LCD_Port &= ~(1<<EN); 
	_delay_us(1);
	LCD_Port |= (1<<EN);
	_delay_us(1);
	LCD_Port &= ~(1<<EN); 
	_delay_us(100);
}
void LCD_Command( unsigned char cmnd )
{
	uint8_t high_byte=cmnd >>4;
	uint8_t low_byte=cmnd&0x0F;
	uint8_t reversedBits_H = ((high_byte & 0b1000) >> 3) |
	((high_byte & 0b0100) >> 1) |
	((high_byte & 0b0010) << 1) |
	((high_byte & 0b0001) << 3);
	uint8_t reversedBits_L = ((low_byte & 0b1000) >> 3) |
	((low_byte & 0b0100) >> 1) |
	((low_byte & 0b0010) << 1) |
	((low_byte & 0b0001) << 3);
	PORTB &=~(1<<RS);		//rs =0
	PORTD &= 0b11000011; 
	PORTD |= (reversedBits_H<<2) ;
	
	Toggle_Enable();
	PORTD &= 0b11000011; //
	PORTD |= (reversedBits_L<<2); 
	Toggle_Enable();
	_delay_ms(2);
}
void LCD_Init (void)  /* LCD Initialize function */
{
	DDRD=0xFF;
	DDRD&=~(1<<PORTD0);
	DDRB|=(1<<RS);
	PORTB&=~(1<<RS);
	PORTD&=~(1<<RW);
	_delay_ms(50);		/* LCD Power ON delay always >15ms */
LCD_Command(0x03);
_delay_ms(5);
LCD_Command(0x03);
_delay_ms(5);
LCD_Command(0x03);
_delay_ms(1);
LCD_Command(0x02);

uint8_t _displayfunction = LCD_4BITMODE | LCD_1LINE | LCD_5x8DOTS;
_displayfunction |= LCD_2LINE;
LCD_Command(LCD_FUNCTIONSET | _displayfunction);
_delay_ms(5);
uint8_t  _displaycontrol = LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF; 
_displaycontrol|= LCD_DISPLAYON;
  LCD_Command(LCD_DISPLAYCONTROL | _displaycontrol);
   LCD_Command(LCD_CLEARDISPLAY);  // 
   _delay_ms(2);  // 
uint8_t _displaymode = LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT;
LCD_Command(LCD_ENTRYMODESET | _displaymode);
_delay_ms(2);
}
void LCD_Char( unsigned char data )
{
	// high_byte=0b11110000;
	uint8_t high_byte=data >>4;
	uint8_t low_byte=data &0x0F;
	uint8_t reversedBits_H = ((high_byte & 0b1000) >> 3) |
	((high_byte & 0b0100) >> 1) |
	((high_byte & 0b0010) << 1) |
	((high_byte & 0b0001) << 3);
	uint8_t reversedBits_L = ((low_byte & 0b1000) >> 3) |
	((low_byte & 0b0100) >> 1) |
	((low_byte & 0b0010) << 1) |
	((low_byte & 0b0001) << 3);
	PORTB |=(1<<RS);		//rs =1
	PORTD &= 0b11000011; // 
	PORTD |= (reversedBits_H<<2); //
	Toggle_Enable();
	PORTD &= 0b11000011;
	PORTD |= (reversedBits_L <<2);
	Toggle_Enable();
	_delay_ms(2);
}
void LCD_String (char *str)		/* Send string to LCD function */
{
	int i;
	for(i=0;str[i]!=0;i++)		/* Send each char of string till the NULL */
	{
		LCD_Char (str[i]);
	}
}
void set_cursor(uint8_t col, uint8_t row)
{
	uint8_t DRAM_ADDRESS = 0x00;
	uint8_t DRAM_OFFSET[4] = {0x00, 0x40, 0x10, 0x50};
	  if ( row >= 4 ) {
		  row = 3;    // we count rows starting w/0
	  }
	 if ( row >= 2 ) {
		 row = 1;    // we count rows starting w/0
	 }
	DRAM_ADDRESS =col + DRAM_OFFSET[row];
	LCD_Command(LCD_SETDDRAMADDR|DRAM_ADDRESS);
}
void LCD_Clear()
{
	LCD_Command (0x01);		/* Clear display */
	_delay_ms(2);
	LCD_Command (0x80);		/* Cursor at home position */
}

void timer1_init() {
	TCCR1B |= (1 << WGM12); 
	TCCR1B |= (1 << CS12) | (1 << CS10); // /1024
	 
	 OCR1A = 3124; // 0.2s
	 TIMSK |= (1 << OCIE1A); // 
	 sei(); //
}
ISR(TIMER1_COMPA_vect) {
	static uint8_t alarm=0;
	static uint8_t press_now=0;

	if (!(PINC & (1 <<PORTC0)))
	{
		if(state_now==normal)
		{
			press_now=1;
			if(press_now==1)
			{
				alarm+=1;
			}
			if(alarm>=8)
			{
				alarm=0;
				state_now=alarm_st;
				press_now=0;
			}
		}
		else{
			change_cur+=1;
			if(change_cur>=3)change_cur=0;
		}
	}
	else{
		if(press_now==1)
		{
				state_now=setup_state;
				press_now=0;
				alarm=0;
				change_cur=0;
				
		}
	}
	if(!(PINC & (1 <<PORTC1)))
	{
		if(save_press==0)
		{
			if(state_now==setup_state)date_now[change_cur]+=1;
			else if(state_now==alarm_st)date_alarm[change_cur]+=1;
			for(int i=0;i<3;i++)
			{
				if(i==0)
				{
					if(date_now[i]>=24)date_now[i]=23;
					if(date_alarm[i]>=24)date_alarm[i]=23;
				}
				else{
					if(date_now[i]>=60)date_now[i]=59;
					if(date_alarm[i]>=60)date_alarm[i]=59;
				}
			}
		}
		else{
			if(state_now==setup_state)date_now[change_cur+3]+=1;
			else if(state_now==alarm_st)date_alarm[change_cur+3]+=1;
			
			if(date_now[MONTH]>=13)date_now[MONTH]=12;
			if(date_alarm[MONTH]>=13)date_alarm[MONTH]=12;
			if(date_now[DAY]>=31)date_now[DAY]=30;
			if(date_alarm[DAY]>=31)date_alarm[DAY]=30;
				
			
		}
	}
	else if(!(PINC & (1 <<PORTC2)))
	{
		if(save_press==0)
		{
		
			if(state_now==setup_state &&date_now[change_cur]>0)date_now[change_cur]-=1;
			else if(state_now==alarm_st&&date_alarm[change_cur]>0)date_alarm[change_cur]-=1;
			
		}
		else{
			if(state_now==setup_state&&date_now[change_cur+3]>0)date_now[change_cur+3]-=1;
			else if(state_now==alarm_st&&date_alarm[change_cur+3]>0)date_alarm[change_cur+3]-=1;
		}
	}
	else if(!(PINC & (1 <<PORTC3)))
	{
		save_press+=1;
		if(save_press>=2)
		{
			save_press=0;
			state_now=normal;
		}
	}
	
}

void my_utoa(unsigned int value, char* result, int base) {
	uint8_t i = 0;
	uint8_t numDigits = 0;
	uint8_t digit_2=1;
	if (value == 0) {
		numDigits = 1;
		} else {
		unsigned int temp = value;
		while (temp != 0) {
			temp /= base;
			numDigits++;
		}
	}

	
	if (value < 10) {
		digit_2=0;
		result[i] = '0';
		i++;
		numDigits++;
	}

	
	do {
		int remainder = value % base;
		result[i] = (remainder < 10) ? (remainder + '0') : (remainder - 10 + 'A');
		i++;
		value /= base;
	} while (value != 0);

	result[numDigits] = '\0';

	// Reverse the string
	int start = 0;
	int end = numDigits - 1;
	if(digit_2==1)
	{
		while (start < end) {
			char temp = result[start];
			result[start] = result[end];
			result[end] = temp;
			start++;
			end--;
		}
	}
}
void display_time(uint8_t min,uint8_t sec)
{
	
	my_utoa(min,min_,10);
	my_utoa(sec,sec_,10);
	set_cursor(7,0);
	LCD_Char(space);
	set_cursor(8,0);
	LCD_String(min_);
	set_cursor(10,0);
	LCD_Char(space);
	set_cursor(11,0);
	LCD_String(sec_);
}
uint8_t index_blink;
char space_only[2]="  ";
void display_setup()
{
	
	set_cursor(8,0);
	LCD_String(setup_display);
	if(!save_press)
	{
		set_cursor(0,1);
		LCD_String(data_display);
		set_cursor(5,1);
		my_utoa(date_now[HOUR],hou,10);
		LCD_String(hou);
		my_utoa(date_now[MINUTE],min_,10);
		my_utoa(date_now[SECOND],sec_,10);
		set_cursor(7,1);
		LCD_Char(space);
		set_cursor(8,1);
		LCD_String(min_);
		set_cursor(10,1);
		LCD_Char(space);
		set_cursor(11,1);
		LCD_String(sec_);
	}
	else{
		set_cursor(0,1);
		LCD_String(data_display2);
		set_cursor(5,1);
		my_utoa(date_now[DAY],day_,10);
		LCD_String(day_);
		my_utoa(date_now[MONTH],mon_,10);
		my_utoa(date_now[YEAR],year_,10);
		set_cursor(7,1);
		LCD_Char(space);
		set_cursor(8,1);
		LCD_String(mon_);
		set_cursor(10,1);
		LCD_Char(space);
		set_cursor(11,1);
		LCD_String(year_bonus);
		set_cursor(13,1);
		LCD_String(year_);
	}
	index_blink=change_cur*3+5;
	_delay_ms(100);
	set_cursor(index_blink,1);
	LCD_String(space_only);
	
	
}
void display_alarm()
{
	
	set_cursor(6,0);
	LCD_String(alarm_display);
	if(!save_press)
	{
		set_cursor(0,1);
		LCD_String(data_display);
		set_cursor(5,1);
		my_utoa(date_alarm[HOUR],hou,10);
		LCD_String(hou);
		my_utoa(date_alarm[MINUTE],min_,10);
		my_utoa(date_alarm[SECOND],sec_,10);
		set_cursor(7,1);
		LCD_Char(space);
		set_cursor(8,1);
		LCD_String(min_);
		set_cursor(10,1);
		LCD_Char(space);
		set_cursor(11,1);
		LCD_String(sec_);
	}
	else{
		set_cursor(0,1);
		LCD_String(data_display2);
		set_cursor(5,1);
		my_utoa(date_alarm[DAY],day_,10);
		LCD_String(day_);
		my_utoa(date_alarm[MONTH],mon_,10);
		my_utoa(date_alarm[YEAR],year_,10);
		set_cursor(7,1);
		LCD_Char(space);
		set_cursor(8,1);
		LCD_String(mon_);
		set_cursor(10,1);
		LCD_Char(space);
		set_cursor(11,1);
		LCD_String(year_bonus);
		set_cursor(13,1);
		LCD_String(year_);
	}
	index_blink=change_cur*3+5;
	_delay_ms(100);
	set_cursor(index_blink,1);
	LCD_String(space_only);
	
}
void display_normal()
{
	set_cursor(0,0);
	LCD_String(data_display);
	set_cursor(5,0);
	my_utoa(date_now[HOUR],hou,10);
	LCD_String(hou);
	display_time(date_now[MINUTE],date_now[SECOND]);
	
	set_cursor(0,1);
	LCD_String(data_display2);
	my_utoa(date_now[DAY],day_,10);
	my_utoa(date_now[MONTH],mon_,10);
	my_utoa(date_now[YEAR],year_,10);
	set_cursor(5,1);
	LCD_String(day_);
	set_cursor(7,1);
	LCD_Char(space2);
	set_cursor(8,1);
	LCD_String(mon_);
	set_cursor(10,1);
	LCD_Char(space2);
	set_cursor(11,1);
	LCD_String(year_bonus);
	set_cursor(13,1);
	LCD_String(year_);
}
int main(void)
{
	DDRB=0xFF;
	DDRC &= ~((1 << PORTC0) | (1 << PORTC2) | (1 << PORTC1) | (1 << PORTC3));
	LCD_Init();
	 
	
	ds1307_init();
	ds1307_setdate(12, 12, 31, 23, 59, 35);
	ds1307_getdate(&date_now[YEAR], &date_now[MONTH], &date_now[DAY], &date_now[HOUR], &date_now[MINUTE], &date_now[SECOND]);
	for(int i=0;i<5;i++)
	{
		date_alarm[i]=date_now[i];
	}
	
	uint8_t last_st=state_now;
	uint8_t alarm_flag=1;
	uint8_t last_alarm=0;
	uint8_t delay_count=0;
	timer1_init();
    while(1)
    {
		
		if(last_alarm==0)
		{
			alarm_flag=1;
			for(int i= 0 ;i<6;i++)
			{
				if(date_alarm[i]!=date_now[i])alarm_flag=0;
			}
			last_alarm=alarm_flag;
		}
		if(alarm_flag==1)
		{
			delay_count+=1;
			if(delay_count<=5)PORTB|=(1<<PORTB2);
			else{
				delay_count=0;
				PORTB&=~(1<<PORTB2);
				last_alarm=0;
			}
		}
        //TODO:: Please write your application code 
		
		if(last_st!=state_now)
		{
			LCD_Clear();
			_delay_ms(50);
			if(last_st==setup_state)
			{
				ds1307_Set(date_now);
			}
			last_st=state_now;
			
		}
		switch(state_now)
		{
			case normal:
				ds1307_getdate(&date_now[YEAR], &date_now[MONTH], &date_now[DAY], &date_now[HOUR], &date_now[MINUTE], &date_now[SECOND]);
				display_normal();
				break;
			case alarm_st:
				display_alarm();
				break;
			case setup_state:
				display_setup();
				break;
		}
		_delay_ms(100);
		
		
    }
}
