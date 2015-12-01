/*
 * send.c
 *
 * Created: 2014/10/29 10:16:56
 *  Author: xiao
 */ 


#include <avr/io.h>
#include "send.h"

#define LEDR_ON _PB3=0
#define LEDR_OFF _PB3=1
#define LEDR_READ _PB3
#define LEDG_ON _PD3=0
#define LEDG_OFF _PD3=1
#define LEDG_READ _PD3
extern unsigned char testbuffer[32];
 unsigned char cache[32];
 volatile unsigned char temp=0;
unsigned int Ad_Get()
{
	ADCSRA |= (1 << ADSC);//AD开始转换
	while(!(ADCSRA&(1<<ADIF)));//等待转换完成
	ADCSRA |= (1 << ADIF);//清零ADC中断标志位，未用中断，不能由硬件清零，只能软件清零
	return ADC;//返回ADC值
}
unsigned int Pos_get()
{
	Ad_Init();
	DDR_PA1 = 1;
_delay_ms(1);
	if (Ad_Get() != 1023)
	return 1;
	{DDR_PA1 = 0;DDR_PA2 = 1;}
	if (Ad_Get() != 1023)
	return 2;
	{DDR_PA2 = 0;DDR_PA3 = 1;}
	if (Ad_Get() != 1023)
	return 3;
	{DDR_PA3 = 0;DDR_PA4 = 1;}
	if (Ad_Get() != 1023)
	return 4;
	{DDR_PA4 = 0;DDR_PA5 = 1;}
	if (Ad_Get() != 1023)
	return 5;
}

#define Conn(x,y) x##y
void SELECT(int num)
{
	DDRA=0XFF;
	switch(num){
		case 1:DDR_PA1=0;break;
		case 2:DDR_PA2=0;break;
		case 3:DDR_PA3=0;break;
		case 4:DDR_PA4=0;break;
		case 5:DDR_PA5=0;break;
		default:printf("error\n");
	}
	
}
unsigned int b=0,d=0,startover=0;
char flag=0;
volatile unsigned char Channel=0;
extern unsigned char a;
int main(void)
{
	Init_MCU( );
	
	
	Init_SPI( );

	Timer_Init();
	//USART0_Init();
	Timer2_Init();
	Ad_Init();
	LEDR_OFF;LEDG_OFF;
	
	Nrf24L01_Init(TX_MODE);
	set_sleep_mode(SLEEP_MODE_IDLE);
	cli();                                                        //关全局中断
	sleep_enable();
	sei();
	//for(a=0;a<50;a++)
	//sleep_cpu();
	LEDR_ON;                                 //CPU睡眠允许  
	Channel = Pos_get();                                                   //开全局中断
	startover=1;
    while(1)
    {
        //TODO:: Please write your application code 
	/*	_delay_ms(1000);
		b = send_int();
		if(_PB3==1)
		LEDR_ON;
		else
		LEDR_OFF;*/
		a=rand();
		
		//sleep_cpu();
    }
}
unsigned char count=0; 
unsigned char sleep_count = 0;
ISR(TIMER1_OVF_vect){
	LEDG_OFF;
	if(sleep_count>10)
	if(send_int()==0)
		{sleep_count = 0;TIMSK = 0x40;}
	else
	{
	
	LEDR_ON;
	_delay_ms(10);
	LEDR_OFF;
	}
	
}
ISR(TIMER2_OVF_vect)
{
	/**/
	if(sleep_count>10)
		TIMSK = 0x04;
	TCNT2 = 0x80;
	volatile unsigned char high,low;
	if (startover)
	{
	b=Ad_Get();
	if((b>950||(b<350&&Channel!=1))&&count==0)
	{Channel = Pos_get(); b=Ad_Get();} 
	
	low=b>>2;
	cache[count++]=low;
	//high=b>>8;
	//cache[count++]=high;
//	if(count==30&&flag==1)
	//LEDR_OFF;
	if (count==30)//&&flag==0)
	{
		for(int c=0;c<32;c++)
		testbuffer[c]=cache[c];
		testbuffer[30]=Channel;
	//	if (a==0x27)
	//	if(flag==0)
		
		
		
		if(send_int()!=0)
		{	
			_delay_ms(12);
			if(send_int()!=0)
				{
				_delay_ms(12);
				if(send_int()==0)
				{LEDR_OFF;sleep_count=0;}
				else
				{LEDR_ON;sleep_count++;}
				}
		}
		else
		LEDR_OFF;
		//else
		//{LEDR_OFF;Nrf24L01_Init(TX_MODE);}
		if (READ_PD3==0)
		LEDG_OFF;
		else
		LEDG_ON;
		count=0;
		flag=1;
	}
//	if(flag==1)
//	flag=send_int();
	
	TCNT1H = 0x80;
	TCNT1L = 0x00;
	}
}
