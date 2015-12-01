#include <avr/io.h>

#include "macr.h"
#include "sdcard.h"

#ifndef F_CPU
#define F_CPU 8000000
#endif
#include <util/delay.h>
//#include "4432.h"
 //         以上程序将信息装载到一个结构体中，这个结构体的定义如下：   
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
     PORTB=0XFF;              //数码管位控制
	 
	 DDRC=0XFF;               //数码管段控制
	 PORTC=0Xff;             
	 
	 DDRD=0XfF;               //键盘接口
     PORTD=0XFf;   
	             //锁存关闭LED显
//	 DDR_PB6=0;	 
	 


}

void Init_SPI(void)
{
     SPCR = (1<<SPE)|(1<<MSTR)|(1<<SPR0);   // 主机模式，fck/16, SPI方式0
	 DDR_PB6=0;
}



void Init_SPI_NO(void)
{
     SPCR = (0<<SPE)|(0<<MSTR)|(0<<SPR0);   // 主机模式，fck/16, SPI方式0
	 DDR_PB6=0;
}

void Init_SPI_HIGH(void)
{
     SPCR = (1<<SPE)|(1<<MSTR);   // 主机模式，fck/2, 高速模式
	 DDR_PB6=0;
//	 SPSR = (1<<SPI2X);
}
//-----------------------------------------------------------------------------------------------   
//  向SD卡中写入命令，并返回回应的第二个字节   
//-----------------------------------------------------------------------------------------------   
unsigned char Write_Command_SD(unsigned char *CMD)   
{    
   unsigned char tmp;   
   unsigned char retry=0;   
   unsigned char i;   
  
   //禁止SD卡片选   
   SPI_CS=1;   
   //发送8个时钟信号   
   Write_Byte_SD(0xFF);   
   //使能SD卡片选   
   SPI_CS=0;   
  
   //向SD卡发送6字节命令   
   for (i=0;i<0x06;i++)    
   {    
      Write_Byte_SD(*CMD++);   
   }   
      
   //获得16位的回应   
   Read_Byte_SD(); //read the first byte,ignore it.    
   do    
   {  //读取后8位   
      tmp = Read_Byte_SD();   
      retry++;   
   }   
   while((tmp==0xff)&&(retry<100));    
   return(tmp);   
}   

//--------------------------------------------------------------------------   
//    初始化SD卡到SPI模式   
//--------------------------------------------------------------------------   

unsigned char SD_Init(void)   
{     
   unsigned char retry,temp;   
   unsigned char i;   
   unsigned char CMD[] = {0x40,0x00,0x00,0x00,0x00,0x95};   
   unsigned char CMD8[] = {0x48,0x00,0x00,0x01,0xaa,0x87}; 
    unsigned char ACMD41[] = {0x69,0x40,0x00,0x00,0x00,0xff}; 
   Init_MCU(); //初始化驱动端口   
   Init_SPI();   
   Init_Flag=1; //将初始化标志置1   
  
   for (i=0;i<0xff;i++)    
   {   
      Write_Byte_SD(0xff); //发送至少74个时钟信号   
   }   
    
   //向SD卡发送CMD0   
   retry=0;   
   do  
   { //为了能够成功写入CMD0,在这里写200次   
      temp=Write_Command_SD(CMD);   
     retry++;   
     if(retry==200)    
     { //超过200次   
       return(1);//CMD0 Error!   
     }   
   }    
   while(temp!=1);  //回应01h，停止写入   
      
   //发送CMD1到SD卡   
   CMD[0] = 0x48; //CMD1   
   CMD[5] = 0xFF;   
   retry=0;   
   do  
   { //为了能成功写入CMD1,写100次   
     temp=Write_Command_SD(CMD8); 
	 
     retry++;   
     if(retry==200)    
     { //超过100次   
       return(2);//CMD1 Error!   
     }   
   }    
   while(temp!=1);//回应00h停止写入   
///////////////////////////////////////////////////////////////////////////////
   CMD[0] = 0x77; //CMD55   
   CMD[5] = 0xFF;   
   retry=0;   
   do  
   { //为了能成功写入CMD1,写100次   
     temp=Write_Command_SD(CMD); 
     retry++;   
     if(retry==200)    
     { //超过100次   
       return(3);//CMD1 Error!   
     }   
   }    
   while(temp!=1);//回应01h停止写入   

      
   retry=0;   
   do  
   { //为了能成功写入CMD1,写100次 
     Write_Command_SD(CMD);  
     temp=Write_Command_SD(ACMD41); 
     retry++;  
     if(retry==200)    
     { //超过100次   
       return(4);//CMD1 Error!   
     }   
   }    
   while(temp!=0);//回应01h停止写入   
      
   Init_Flag=0; //初始化完毕，初始化标志清零   
      
   SPI_CS=1;  //片选无效   
   return(0); //初始化成功   
}   
//------------------------------------------------------------------------------------   
//    读取SD卡的CID寄存器   16字节   成功返回0   
//------------------------------------------------------------------------------------- 
/*unsigned char Read_CID_SD(unsigned char *Buffer)   
{   
   //读取CID寄存器的命令   
   unsigned char CMD[] = {0x4A,0x00,0x00,0x00,0x00,0xFF};    
   unsigned char temp;   
   temp=SD_Read_Block(CMD,Buffer,16); //read 16 bytes   
   return(temp);   
} */  
//-----------------------------------------------------------------------------------------   
//    读SD卡的CSD寄存器   共16字节    返回0说明读取成功   
//-----------------------------------------------------------------------------------------   
/*unsigned char Read_CSD_SD(unsigned char *Buffer)   
{    
   //读取CSD寄存器的命令   
   unsigned char CMD[] = {0x49,0x00,0x00,0x00,0x00,0xFF};   
   unsigned char temp;   
   temp=SD_Read_Block(CMD,Buffer,16); //read 16 bytes   
   return(temp);   
}   */

 //-----------------------------------------------------------------------------------------------   
//返回   
//  SD卡的容量，单位为M   
//  sector count and multiplier MB are in    
//u08 == C_SIZE / (2^(9-C_SIZE_MULT))   
//  SD卡的名称   
//-----------------------------------------------------------------------------------------------   
/*void SD_get_volume_info()   
{      
    unsigned char i;   
    unsigned char c_temp[5];   
    VOLUME_INFO_TYPE SD_volume_Info,*vinf;   
    vinf=&SD_volume_Info; //Init the pointoer;   
//读取CSD寄存器   
    Read_CSD_SD(sectorBuffer.dat);   
//获取总扇区数   
 vinf->sector_count = sectorBuffer.dat[6] & 0x03;   
 vinf->sector_count <<= 8;   
 vinf->sector_count += sectorBuffer.dat[7];   
 vinf->sector_count <<= 2;   
 vinf->sector_count += (sectorBuffer.dat[8] & 0xc0) >> 6;   
 // 获取multiplier   
 vinf->sector_multiply = sectorBuffer.dat[9] & 0x03;   
 vinf->sector_multiply <<= 1;   
 vinf->sector_multiply += (sectorBuffer.dat[10] & 0x80) >> 7;   
//获取SD卡的容量   
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
//////                              扇区读
/////////////////////////////////////////////////////////////////////////////////////////////////////
unsigned char SD_Read_Sector(volatile unsigned long sector,unsigned char *buffer)   
{     
   unsigned char retry;   
   //命令16   
   unsigned char CMD[] = {0x51,0x00,0x00,0x00,0x00,0xFF};    
   unsigned char temp;   
  
   //地址变换   由逻辑块地址转为字节地址   
 //  sector = sector << 9; //sector = sector * 512   
  
   CMD[1] = ((sector & 0xFF000000) >>24   );   
   CMD[2] = ((sector & 0x00FF0000) >>16   );   
   CMD[3] = ((sector & 0x0000FF00) >>8    );   
   CMD[4] = ((sector & 0x000000FF)        );   
  
   //将命令16写入SD卡   
   retry=0;   
   do  
   {  //为了保证写入命令  一共写100次   
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
  SD_get_data(512,buffer) ;  //512字节被读出到buffer中   
//	Read_Byte_SD();
//	Read_Byte_SD();
	SPI_CS=1;  //SD卡关闭片选 
//	Write_Byte_SD(0xFF);//按照SD卡的操作时序在这里补8个时钟
return 0;
}   
//----------------------------------------------------------------------------   
//    获取数据到buffer中   
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
//   写512个字节到SD卡的某一个扇区中去   返回0说明写入成功   
//--------------------------------------------------------------------------------------------   
unsigned char SD_write_sector(unsigned long addr,unsigned char *Buffer)   
{     
   unsigned char tmp,retry;   
   unsigned int i;   
   //命令24   
   unsigned char CMD[] = {0x58,0x00,0x00,0x00,0x00,0xFF};    
  // addr = addr << 9; //addr = addr * 512   
    
   CMD[1] = ((addr & 0xFF000000) >>24   );   
   CMD[2] = ((addr & 0x00FF0000) >>16   );   
   CMD[3] = ((addr & 0x0000FF00) >>8    );   
   CMD[4] = ((addr & 0x000000FF)        );   
  
  
   //写命令24到SD卡中去   
   retry=0;   
   do  
   {  //为了可靠写入，写100次   
      tmp=Write_Command_SD(CMD);   
      retry++;   
      if(retry==100)    
      {    
        return(tmp); //send commamd Error!   
      }   
   }   
   while(tmp!=0);    
      
  
   //在写之前先产生100个时钟信号   
   for (i=0;i<100;i++)   
   {   
      Read_Byte_SD();   
   }   
    
   //写入开始字节   
   Write_Byte_SD(0xFE);    
    
   //现在可以写入512个字节   
   for (i=0;i<512;i++)   
   {   
       Write_Byte_SD(*Buffer++);    
   }   
  
   //CRC-Byte    
    Write_Byte_SD(0xFF); //Dummy CRC   
    Write_Byte_SD(0xFF); //CRC Code   
      
       
   tmp= Read_Byte_SD();   // read response   
   if((tmp & 0x1F)!=0x05) // 写入的512个字节是未被接受   
   {   
     SPI_CS=1;   
     return(WRITE_BLOCK_ERROR); //Error!   
   }   
   //等到SD卡不忙为止   
//因为数据被接受后，SD卡在向储存阵列中编程数据   
   while ( Read_Byte_SD()!=0xff){};   
    
   //禁止SD卡   
   SPI_CS=1;   
   return(0);//写入成功   
} 



//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////  
  
/*void Spisoundwrite(unsigned char low,unsigned char high)
{
	high^=0x80;
  unsigned char i;
    nSET_CLR;
	for(i=0;i<4;i++)           //再写数据
    {
        
        SDI_CLR; //DIN=0
        _delay_us(1);
        SCLK_SET;    //CLK=1
        _delay_us(2);
        SCLK_CLR;    //CLK=0
        low = low << 1;
        _delay_us(1);  
    }
    for(i=0;i<8;i++)           //先写地址
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
    for(i=0;i<4;i++)           //再写数据
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
//b=0;//读取的是块
Init_SPI_HIGH();
SD_Read_Sector(b,lux);
a=1;
a=0;
b++;

}
}
*/
