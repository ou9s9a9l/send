#include <avr/io.h>

#include "macr.h"
#include "sdcard.h"

#ifndef F_CPU
#define F_CPU 8000000
#endif
#include <util/delay.h>
//#include "4432.h"
 //         ���ϳ�����Ϣװ�ص�һ���ṹ���У�����ṹ��Ķ������£�   
/*typedef struct SD_VOLUME_INFO   
{ //SD/SD Card info   
  unsigned int  size_MB;   
  unsigned char sector_multiply;   
  unsigned int  sector_count;   
  unsigned char name[6];   
} VOLUME_INFO_TYPE; */  
volatile unsigned char Init_Flag,readPos/*,value*/,big,small; 
//unsigned char lux[512]={1,1,1,1,1,1,1,1,1,11,1,1,1,1,1,1,1,1,1,11,1,1,1,1,1,1,1,11,1,1};
//unsigned char lux[512]={0};
#define WRITE_BLOCK_ERROR 1
#define INIT_CMD0_ERROR 1
#define INIT_CMD1_ERROR 2
#define length 512
 void Write_Byte_SD(unsigned char add)
{
unsigned char clear;
SPDR=add;
while(!(SPSR&(1<<SPIF)));
clear=SPSR;  
//return SPDR ;
}
unsigned char Read_Byte_SD(void)
{
unsigned char clear;
SPDR=0xff;
while(!(SPSR&(1<<SPIF)));
clear=SPSR;
return SPDR;
}   

void Init_MCU(void)
{
     DDRA=0XFF;
	 PORTA=0XFF;
	 
	 DDRB=0XFf;             
     PORTB=0XFF;              //�����λ����
	 
	 DDRC=0XFF;               //����ܶο���
	 PORTC=0Xff;             
	 
	 DDRD=0XfF;               //���̽ӿ�
     PORTD=0XFf;   
	             //����ر�LED��
//	 DDR_PB6=0;	 
	 


}

void Init_SPI(void)
{
     SPCR = (1<<SPE)|(1<<MSTR)|(1<<SPR0);   // ����ģʽ��fck/16, SPI��ʽ0
	 DDR_PB6=0;
}



void Init_SPI_NO(void)
{
     SPCR = (0<<SPE)|(0<<MSTR)|(0<<SPR0);   // ����ģʽ��fck/16, SPI��ʽ0
	 DDR_PB6=0;
}

void Init_SPI_HIGH(void)
{
     SPCR = (1<<SPE)|(1<<MSTR);   // ����ģʽ��fck/2, ����ģʽ
	 DDR_PB6=0;
//	 SPSR = (1<<SPI2X);
}
//-----------------------------------------------------------------------------------------------   
//  ��SD����д����������ػ�Ӧ�ĵڶ����ֽ�   
//-----------------------------------------------------------------------------------------------   
unsigned char Write_Command_SD(unsigned char *CMD)   
{    
   unsigned char tmp;   
   unsigned char retry=0;   
   unsigned char i;   
  
   //��ֹSD��Ƭѡ   
   SPI_CS=1;   
   //����8��ʱ���ź�   
   Write_Byte_SD(0xFF);   
   //ʹ��SD��Ƭѡ   
   SPI_CS=0;   
  
   //��SD������6�ֽ�����   
   for (i=0;i<0x06;i++)    
   {    
      Write_Byte_SD(*CMD++);   
   }   
      
   //���16λ�Ļ�Ӧ   
   Read_Byte_SD(); //read the first byte,ignore it.    
   do    
   {  //��ȡ��8λ   
      tmp = Read_Byte_SD();   
      retry++;   
   }   
   while((tmp==0xff)&&(retry<100));    
   return(tmp);   
}   

//--------------------------------------------------------------------------   
//    ��ʼ��SD����SPIģʽ   
//--------------------------------------------------------------------------   

unsigned char SD_Init(void)   
{     
   unsigned char retry,temp;   
   unsigned char i;   
   unsigned char CMD[] = {0x40,0x00,0x00,0x00,0x00,0x95};   
   unsigned char CMD8[] = {0x48,0x00,0x00,0x01,0xaa,0x87}; 
    unsigned char ACMD41[] = {0x69,0x40,0x00,0x00,0x00,0xff}; 
   Init_MCU(); //��ʼ�������˿�   
   Init_SPI();   
   Init_Flag=1; //����ʼ����־��1   
  
   for (i=0;i<0xff;i++)    
   {   
      Write_Byte_SD(0xff); //��������74��ʱ���ź�   
   }   
    
   //��SD������CMD0   
   retry=0;   
   do  
   { //Ϊ���ܹ��ɹ�д��CMD0,������д200��   
      temp=Write_Command_SD(CMD);   
     retry++;   
     if(retry==200)    
     { //����200��   
       return(1);//CMD0 Error!   
     }   
   }    
   while(temp!=1);  //��Ӧ01h��ֹͣд��   
      
   //����CMD1��SD��   
   CMD[0] = 0x48; //CMD1   
   CMD[5] = 0xFF;   
   retry=0;   
   do  
   { //Ϊ���ܳɹ�д��CMD1,д100��   
     temp=Write_Command_SD(CMD8); 
	 
     retry++;   
     if(retry==200)    
     { //����100��   
       return(2);//CMD1 Error!   
     }   
   }    
   while(temp!=1);//��Ӧ00hֹͣд��   
///////////////////////////////////////////////////////////////////////////////
   CMD[0] = 0x77; //CMD55   
   CMD[5] = 0xFF;   
   retry=0;   
   do  
   { //Ϊ���ܳɹ�д��CMD1,д100��   
     temp=Write_Command_SD(CMD); 
     retry++;   
     if(retry==200)    
     { //����100��   
       return(3);//CMD1 Error!   
     }   
   }    
   while(temp!=1);//��Ӧ01hֹͣд��   

      
   retry=0;   
   do  
   { //Ϊ���ܳɹ�д��CMD1,д100�� 
     Write_Command_SD(CMD);  
     temp=Write_Command_SD(ACMD41); 
     retry++;  
     if(retry==200)    
     { //����100��   
       return(4);//CMD1 Error!   
     }   
   }    
   while(temp!=0);//��Ӧ01hֹͣд��   
      
   Init_Flag=0; //��ʼ����ϣ���ʼ����־����   
      
   SPI_CS=1;  //Ƭѡ��Ч   
   return(0); //��ʼ���ɹ�   
}   
//------------------------------------------------------------------------------------   
//    ��ȡSD����CID�Ĵ���   16�ֽ�   �ɹ�����0   
//------------------------------------------------------------------------------------- 
/*unsigned char Read_CID_SD(unsigned char *Buffer)   
{   
   //��ȡCID�Ĵ���������   
   unsigned char CMD[] = {0x4A,0x00,0x00,0x00,0x00,0xFF};    
   unsigned char temp;   
   temp=SD_Read_Block(CMD,Buffer,16); //read 16 bytes   
   return(temp);   
} */  
//-----------------------------------------------------------------------------------------   
//    ��SD����CSD�Ĵ���   ��16�ֽ�    ����0˵����ȡ�ɹ�   
//-----------------------------------------------------------------------------------------   
/*unsigned char Read_CSD_SD(unsigned char *Buffer)   
{    
   //��ȡCSD�Ĵ���������   
   unsigned char CMD[] = {0x49,0x00,0x00,0x00,0x00,0xFF};   
   unsigned char temp;   
   temp=SD_Read_Block(CMD,Buffer,16); //read 16 bytes   
   return(temp);   
}   */

 //-----------------------------------------------------------------------------------------------   
//����   
//  SD������������λΪM   
//  sector count and multiplier MB are in    
//u08 == C_SIZE / (2^(9-C_SIZE_MULT))   
//  SD��������   
//-----------------------------------------------------------------------------------------------   
/*void SD_get_volume_info()   
{      
    unsigned char i;   
    unsigned char c_temp[5];   
    VOLUME_INFO_TYPE SD_volume_Info,*vinf;   
    vinf=&SD_volume_Info; //Init the pointoer;   
//��ȡCSD�Ĵ���   
    Read_CSD_SD(sectorBuffer.dat);   
//��ȡ��������   
 vinf->sector_count = sectorBuffer.dat[6] & 0x03;   
 vinf->sector_count <<= 8;   
 vinf->sector_count += sectorBuffer.dat[7];   
 vinf->sector_count <<= 2;   
 vinf->sector_count += (sectorBuffer.dat[8] & 0xc0) >> 6;   
 // ��ȡmultiplier   
 vinf->sector_multiply = sectorBuffer.dat[9] & 0x03;   
 vinf->sector_multiply <<= 1;   
 vinf->sector_multiply += (sectorBuffer.dat[10] & 0x80) >> 7;   
//��ȡSD��������   
 vinf->size_MB = vinf->sector_count >> (9-vinf->sector_multiply);   
 // get the name of the card   
 Read_CID_SD(sectorBuffer.dat);   
 vinf->name[0] = sectorBuffer.dat[3];   
 vinf->name[1] = sectorBuffer.dat[4];   
 vinf->name[2] = sectorBuffer.dat[5];   
 vinf->name[3] = sectorBuffer.dat[6];   
 vinf->name[4] = sectorBuffer.dat[7];   
 vinf->name[5] = 0x00; //end flag    
}   
*/
////////////////////////////////////////////////////////////////////////////////////////////////////
//////                              ������
/////////////////////////////////////////////////////////////////////////////////////////////////////
unsigned char SD_Read_Sector(volatile unsigned long sector,unsigned char *buffer)   
{     
   unsigned char retry;   
   //����16   
   unsigned char CMD[] = {0x51,0x00,0x00,0x00,0x00,0xFF};    
   unsigned char temp;   
  
   //��ַ�任   ���߼����ַתΪ�ֽڵ�ַ   
 //  sector = sector << 9; //sector = sector * 512   
  
   CMD[1] = ((sector & 0xFF000000) >>24   );   
   CMD[2] = ((sector & 0x00FF0000) >>16   );   
   CMD[3] = ((sector & 0x0000FF00) >>8    );   
   CMD[4] = ((sector & 0x000000FF)        );   
  
   //������16д��SD��   
   retry=0;   
   do  
   {  //Ϊ�˱�֤д������  һ��д100��   
      temp=Write_Command_SD(CMD);   
      retry++;   
      if(retry==100)    
      {   
        return(1); //block write Error!   
      }   
   }   
   while(temp!=0);    
         
   //Read Start Byte form MMC/SD-Card (FEh/Start Byte)   
   //Now data is ready,you can read it out.   
   while (Read_Byte_SD() != 0xfe);   
   readPos=0;   
  SD_get_data(512,buffer) ;  //512�ֽڱ�������buffer��   
//	Read_Byte_SD();
//	Read_Byte_SD();
	SPI_CS=1;  //SD���ر�Ƭѡ 
//	Write_Byte_SD(0xFF);//����SD���Ĳ���ʱ�������ﲹ8��ʱ��
return 0;
}   
//----------------------------------------------------------------------------   
//    ��ȡ���ݵ�buffer��   
//----------------------------------------------------------------------------   
void SD_get_data(unsigned int Bytes,unsigned char *buffer)    
{   
   unsigned int j;   
   for (j=0;j<Bytes;j++)   
  // 	{ small=Read_Byte_SD();
//	 big=Read_Byte_SD();
//	 Spisoundwrite(small,big);}
     *buffer++ = Read_Byte_SD();   
}   
//--------------------------------------------------------------------------------------------   
//   д512���ֽڵ�SD����ĳһ��������ȥ   ����0˵��д��ɹ�   
//--------------------------------------------------------------------------------------------   
unsigned char SD_write_sector(unsigned long addr,unsigned char *Buffer)   
{     
   unsigned char tmp,retry;   
   unsigned int i;   
   //����24   
   unsigned char CMD[] = {0x58,0x00,0x00,0x00,0x00,0xFF};    
  // addr = addr << 9; //addr = addr * 512   
    
   CMD[1] = ((addr & 0xFF000000) >>24   );   
   CMD[2] = ((addr & 0x00FF0000) >>16   );   
   CMD[3] = ((addr & 0x0000FF00) >>8    );   
   CMD[4] = ((addr & 0x000000FF)        );   
  
  
   //д����24��SD����ȥ   
   retry=0;   
   do  
   {  //Ϊ�˿ɿ�д�룬д100��   
      tmp=Write_Command_SD(CMD);   
      retry++;   
      if(retry==100)    
      {    
        return(tmp); //send commamd Error!   
      }   
   }   
   while(tmp!=0);    
      
  
   //��д֮ǰ�Ȳ���100��ʱ���ź�   
   for (i=0;i<100;i++)   
   {   
      Read_Byte_SD();   
   }   
    
   //д�뿪ʼ�ֽ�   
   Write_Byte_SD(0xFE);    
    
   //���ڿ���д��512���ֽ�   
   for (i=0;i<512;i++)   
   {   
       Write_Byte_SD(*Buffer++);    
   }   
  
   //CRC-Byte    
    Write_Byte_SD(0xFF); //Dummy CRC   
    Write_Byte_SD(0xFF); //CRC Code   
      
       
   tmp= Read_Byte_SD();   // read response   
   if((tmp & 0x1F)!=0x05) // д���512���ֽ���δ������   
   {   
     SPI_CS=1;   
     return(WRITE_BLOCK_ERROR); //Error!   
   }   
   //�ȵ�SD����æΪֹ   
//��Ϊ���ݱ����ܺ�SD�����򴢴������б������   
   while ( Read_Byte_SD()!=0xff){};   
    
   //��ֹSD��   
   SPI_CS=1;   
   return(0);//д��ɹ�   
} 



//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////  
  
/*void Spisoundwrite(unsigned char low,unsigned char high)
{
	high^=0x80;
  unsigned char i;
    nSET_CLR;
	for(i=0;i<4;i++)           //��д����
    {
        
        SDI_CLR; //DIN=0
        _delay_us(1);
        SCLK_SET;    //CLK=1
        _delay_us(2);
        SCLK_CLR;    //CLK=0
        low = low << 1;
        _delay_us(1);  
    }
    for(i=0;i<8;i++)           //��д��ַ
    {
        if((high&0x80)==0x80)
            SDI_SET; //DIN=1
        else
            SDI_CLR; //DIN=0
        _delay_us(1);
        SCLK_SET;    //CLK=1
        _delay_us(2);
        SCLK_CLR;    //CLK=0
        high = high << 1;
        _delay_us(1);  
    }
    for(i=0;i<4;i++)           //��д����
    {
        if((low&0x80)==0x80)
            SDI_SET; //DIN=1
        else
            SDI_CLR; //DIN=0
        _delay_us(1);
        SCLK_SET;    //CLK=1
        _delay_us(2);
        SCLK_CLR;    //CLK=0
        low = low << 1;
        _delay_us(1);  
    }
    nSET_SET;
_delay_us(30);
}



  int main(void)
{
volatile unsigned int a,c;
volatile unsigned long b=1;
value=SD_Init();
value=SD_Read_Sector(0,lux);
//value=SD_Init();
//value=SD_write_sector(0,lux);
//value=SD_Init();
//lux[0]=0;
while(1){
//b=0;//��ȡ���ǿ�
Init_SPI_HIGH();
SD_Read_Sector(b,lux);
a=1;
a=0;
b++;

}
}
*/
