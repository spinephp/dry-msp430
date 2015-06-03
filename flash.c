#include <msp430g2553.h>
//**************************************************/
//��FLASH��Ϣ������ָ���������ֽ�����
//unsigned char *pc_byte ��Ϣ������ָ��
//unsigned char *Dataout :�������ݴ����������,8λ��
//unsigned char count :���ٵ�����,��Χ0-127
//**************************************************/
void FlashRead(unsigned char *pc_byte,unsigned char *Dataout,unsigned int count)
{
  while(count--)
    *Dataout++ = *pc_byte++;
}
/*************************************************
//��FLASH��Ϣ��д��ָ���������ֽ�����
//unsigned char *pc_byte ��Ϣ������ָ��
//unsigned char *Datain :�������ݴ����������,8λ��
//unsigned char count :���ٵ�����,��Χ0-127
**************************************************/
void FlashWrite(unsigned char *pc_byte,unsigned char *Datain,unsigned int count)
{
  FCTL2 = FWKEY + FSSEL_1 + FN3 + FN4;//MCLK  16*FN4 + 8*FN3
  FCTL3 = FWKEY;
  FCTL1 = FWKEY + WRT;
  while(FCTL3 & BUSY);                //�������æ״̬����ȴ�

  while(count--)
  {
    while(FCTL3 & BUSY);
    *pc_byte++ = *Datain++;
  }

  FCTL1 = FWKEY;
  FCTL3 = FWKEY + LOCK;
  while(FCTL3 & BUSY);
}

