/*
 *  液晶1602.c--4线驱动
 *  Created on: 2013-7-27
 *  Author: Qinkai
 *  P2.0->D4
 *  P2.1->D5
 *  P2.2->D6
 *  P2.3->D7
 *  P2.5->E
 *  P2.4->RS
 *  vo  ->GND
 *  rw  ->GND
 *
 */
#include <msp430g2553.h>
extern void DelayNus(unsigned int n);

#define RS_1 P2OUT|=BIT4
#define RS_0 P2OUT&=~BIT4
#define EN_1 P2OUT|=BIT5
#define EN_0 P2OUT&=~BIT5

void LCD_en1(void)
{
	EN_0;
	DelayNus(10);
	EN_1;
}

void LCD_en2(void)
{
	EN_1;
	DelayNus(10);
	EN_0;
}
void writeany(unsigned char com)
{
	LCD_en1();
	P2OUT &=0xf0;//清低四位
	P2OUT |=(com>>4);//&0x0f;//写高四位
	DelayNus(16);
	LCD_en2();
	LCD_en1();
	P2OUT &=0xf0;//清低四位
	P2OUT |=com&0x0f;//写低四位
	LCD_en2();

}
//写指令
void writecom(unsigned char com)
{
	//DelayNus(16);
	RS_0;
	writeany(com);
}

//写数据
void writedata(unsigned char  dat)
{
	//DelayNus(16);
	RS_1;
	writeany(dat);
}

//lcd初始化程序
void lcdinit()
{
	DelayNus(3000);
	P2DIR |= BIT0|BIT1|BIT2|BIT3|BIT4|BIT5;
	writecom(0x02); // 数据指针清零
	//DelayNus(1000);
	//writecom(0x28);
	//DelayNus(1000);
	//writecom(0x28);
	//DelayNus(1000);
	writecom(0x28);
	//DelayNus(1000);
	writecom(0x0c);
	//DelayNus(1000);
	writecom(0x01);
	//DelayNus(1000);
	writecom(0x06);
	//DelayNus(1000);
	//writecom(0x80);

}

//LCD在任意位置写字符串
//列x=0~15,行y=0,1
void LCD_write_string(unsigned char x,unsigned char y,char *s)
{
	writecom("\x80\xc0"[y] +x);
	while (*s) // 显示字符
	{
		writedata( *s++ );
	}
}
