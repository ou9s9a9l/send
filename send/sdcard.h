#ifndef sdcard_H
#define sdcard_H
#endif
extern unsigned char Write_Command_SD(unsigned char *CMD)   ;
extern unsigned char SD_Read_Sector(unsigned long sector,unsigned char *buffer)  ;
extern void SD_get_data(unsigned int Bytes,unsigned char *buffer) ;
extern void Write_Byte_SD(unsigned char add);
extern unsigned char Read_Byte_SD(void);
extern void Init_MCU(void);
extern void Init_SPI(void);
extern unsigned char SD_Init(void)   ;
extern void SD_get_data(unsigned int Bytes,unsigned char *buffer) ;   
extern unsigned char SD_write_sector(unsigned long addr,unsigned char *Buffer)  ; 
extern void Spisoundwrite(unsigned char low,unsigned char high);
extern void Init_SPI_HIGH(void);
