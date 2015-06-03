#include <msp430g2553.h>
//**************************************************/
//向FLASH信息区读出指定数量的字节数据
//unsigned char *pc_byte 信息区数据指针
//unsigned char *Dataout :读出数据存放数据数组,8位长
//unsigned char count :读操的数量,范围0-127
//**************************************************/
void FlashRead(unsigned char *pc_byte,unsigned char *Dataout,unsigned int count)
{
  while(count--)
    *Dataout++ = *pc_byte++;
}
/*************************************************
//向FLASH信息区写入指定数量的字节数据
//unsigned char *pc_byte 信息区数据指针
//unsigned char *Datain :读出数据存放数据数组,8位长
//unsigned char count :读操的数量,范围0-127
**************************************************/
void FlashWrite(unsigned char *pc_byte,unsigned char *Datain,unsigned int count)
{
  FCTL2 = FWKEY + FSSEL_1 + FN3 + FN4;//MCLK  16*FN4 + 8*FN3
  FCTL3 = FWKEY;
  FCTL1 = FWKEY + WRT;
  while(FCTL3 & BUSY);                //如果处于忙状态，则等待

  while(count--)
  {
    while(FCTL3 & BUSY);
    *pc_byte++ = *Datain++;
  }

  FCTL1 = FWKEY;
  FCTL3 = FWKEY + LOCK;
  while(FCTL3 & BUSY);
}

