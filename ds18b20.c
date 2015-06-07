/*
 * ds18b20.c
 *
 *  Created on: 2015年6月7日
 *      Author: LY
 */

#include <msp430g2553.h>
extern void DelayNus(unsigned int n);
extern char LCDBuf2[];
extern char temperature[];

#define   DQ_OUT     P1DIR|=BIT0
#define   DQ_IN      P1DIR&=~BIT0
#define   DQ_H       P1OUT|=BIT0
#define   DQ_L       P1OUT&=~BIT0
#define   DQ_DATA    (P1IN&BIT0)

//-----------------------------------
//功能：写18B20
//-----------------------------------
void Write_18B20(unsigned char n)
{
	unsigned int i=8;
	while(i--)
	{
		DQ_H;
		_NOP();
		DQ_L;
		DelayNus(2);//延时13us 左右
		if((n&0X01)==0X01)
			DQ_H;
		else
			DQ_L;
		n>>=1;
		DelayNus(5);//延时50us 以上
		DQ_H;
	}
}

//-----------------------------------
//功能：读18B20
//-----------------------------------
unsigned char Read_18B20(void)
{
	unsigned int i=8;
	unsigned char temp;
	while(i--)
	{
		DQ_H;
		_NOP();
		temp >>= 1;
		DQ_L;
		_NOP();
		DQ_H;
		_NOP();
		DQ_IN;
		if(DQ_DATA == 0)
		{
			temp &= 0x7F;
		}
		else
		{
			temp |= 0x80;
		}
		DelayNus(1);
		DQ_OUT;
		DQ_H;
	}
	return temp;
}

//-----------------------------------
//功能：初始化
//-----------------------------------
void Init(void)
{
	DQ_H;
	_NOP();
	DQ_L;
	DelayNus(110);
	DQ_H;
	DelayNus(19);
	DQ_IN;
	if(DQ_DATA == BIT0)
	{
		*LCDBuf2 = 'F';//Error = 1;
		DQ_OUT;
	}
	else
	{
		*LCDBuf2 = 'T';//Error = 0;
		DQ_OUT;
		DQ_H;
		DelayNus(91);
	}

}

//----------------------------------

void ReadTemp(void)
{
	temperature[0] = Read_18B20();
	temperature[1] = Read_18B20();
}

void GetTemp(void)
{
	Init();
	Write_18B20(0xcc);//Skip();
	Write_18B20(0x44);//Convert();
	DelayNus(59090);	DelayNus(59090);	DelayNus(59090);
	Init();
	Write_18B20(0xcc);//Skip();
	Write_18B20(0xbe);//ReadDo();
	ReadTemp();

}

