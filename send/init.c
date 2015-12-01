/*
 * init.c
 *
 * Created: 2014/10/21 8:52:45
 *  Author: xiao
 */ 
#include <avr/io.h>
#include "macr.h"
#include "nRF24L01.h"

void Init_SPI(void)
{
	SPCR = (1<<SPE)|(1<<MSTR)|(1<<SPR0);   // ����ģʽ��fck/16, SPI��ʽ0
	DDR_PB6=0;
	//	DDR_PB4=1;
}
void Init_MCU(void)
{
	DDRA=0XFF;
	PORTA=0XFF;
	
	DDRB=0XFf;
	PORTB=0XFF;              //�����λ����
	
	DDRC=0XFF;               //����ܶο���
	PORTC=0Xff;
	
	 DDRD=0Xfc;               //���̽ӿ�
	 PORTD=0XFf;
	//����ر�LED��

}

void Timer_Init(void)
{
	//cli();
	TCCR1B=0x04;// 1024  5devide  3is 4s  4is 4s
	TCNT1H = 0x00;
	TCNT1L = 0x00;
	//TIMSK|=0x04;//1�����ж�
	//sei();
}
void Timer2_Init(void)
{
	//cli();
	TCCR2=0x06;// 1024  5devide  3is 1s  4is 4s
	TCNT2 = 0x80;
	
	TIMSK|=0x40;//1�����ж�
	//sei();
}
/*
void USART0_Init( void )
{
	UBRR0L=51;         //8m 51 1m  12 4800   8888 16��2x�����57600
	//UCSR0A =(1<<U2X0);
	UCSR0B = (1<<RXEN0)|(1<<TXEN0)|(1<<RXCIE0);
	UCSR0C =0x0e;// (1<<USBS0)|(3<<UCSZ0);
}*/
//typedef enum{ TX_MODE, RX_MODE }L01MD;
void Nrf24L01_Init(L01MD mode )
{
	L01_CE_LOW( );
	L01_Init();
	L01_SetTRMode( mode );
	L01_WriteHoppingPoint( 0 );
	L01_CE_LOW();
}
void Ad_Init(void)
{
	ADMUX |= (1 << REFS1)|(1 << REFS0);//����ADC�Ĳο���ѹԴΪAVcc|(1 << MUX0)
	ADCSRA |= (1 << ADEN)|(1 << ADPS2)|(1 << ADPS1);//ʹ��ADC��64��Ƶ
	
	
	
	DDR_PA0=0;
	_PA0 = 0;
	_PA1 = 0;
	_PA2 = 0;
	_PA3 = 0;
	_PA4 = 0;
	_PA5 = 0;
	DDR_PA1 = 0;
	DDR_PA2 = 0;
	DDR_PA3 = 0;
	DDR_PA4 = 0;
	DDR_PA5 = 0;
	
	
}