#include  "msp430g2553.h"
#include  "ds18b20.h"
#include  "1602.h"
#include  "flash.h"
#include  "adc.h"
#include  "timer.h"
#include "stdint.h"
#include <stdio.h>

typedef unsigned char uchar;
typedef unsigned int  uint;

#define RXBUF_SIZE  16

char RX_BUFF[RXBUF_SIZE] = { 0}; 									//receive buffer

uchar TX_BUFF[13] = { 0xfd}; 									//

uint TX_IndexR = 0; 						//FIFO pointer for RX

char temperature[2]={0x00,0x80};
int tmp;
char *p;

uchar dryState[8]; // 0,1-curDryLineNo, 2,3-settingTemperature 4,5-curLineTime 6,7-totalTime

#define   DQ_OUT     P1DIR|=BIT0
#define   DQ_H       P1OUT|=BIT0

#define SegmentStart 0x01000  //Segment = 0x01000~0x010FF
#define SegmentSize 255 //SegmentA + SegmentB = 256

char LCDBuf1[]={"S-99 00:00 00:00"};   //第一行要显示的内容
char LCDBuf2[]={"T-99 00:00 00:00"};     //第二行要显示的内容
//----------------------------------
//功能：us 级别延时
// n=10，则延时10*5+6=56uS
//----------------------------------
void DelayNus(uint n)
{
	while(n--)_NOP();
}
//#define CPU_F ((double)8000000)
//#define delay_us(x) DelayNus((long)(CPU_F*(double)x/1000000.0))
//#define delay_ms(x) __delay_cycles((long)(CPU_F*(double)x/1000.0))

/*************************************
 * 功能:把正整数转换为字符串，供显示
 * 参数：i 正整数
 *     s 字符串指针，指定转换后字符串的起始位置
 *     p 字符串指针，指定转换后字符串的结束位置
 */
void ntoa(uint num,char *s,char *p)
{
	while(num){
		*p-- = (char)((num %10) +0x30);
		num /= 10;
	}
	while(s<=p)
		*s++ = 0x30;
}
/*************************************
 * 功能:把从 DS18B20 中读取的温度转换为字符串，供显示
 * 参数：i 整数，指定温度
 *     s 字符串指针，指定转换后字符串的起始位置
 *     p 字符串指针，指定转换后字符串的结束位置
 */
void timetoa(uint num,char *s,char *p)
{
	ntoa(num/60,s,s+1);
	ntoa(num%60,p-1,p);
}
/*************************************
 * 功能:把从 DS18B20 中读取的温度转换为字符串，供显示
 * 参数：i 整数，指定温度
 *     s 字符串指针，指定转换后字符串的起始位置
 *     p 字符串指针，指定转换后字符串的结束位置
 */
void temptoa(int num,char *s,char *p)
{
	if(num<0) {
		*s++ = '-';
		num = -num;
	}
	ntoa((uint)num>>4,s,p);
}

void main( void )
{

	WDTCTL = WDTPW + WDTHOLD;         // Stop watchdog timer to prevent time out reset

	if (CALBC1_1MHZ==0xFF)					// If calibration constant erased
	{
	   while(1);                               // do not load, trap CPUz!!
	}
	DCOCTL = 0;                               // Select lowest DCOx and MODx settings
	BCSCTL1 = CALBC1_1MHZ;                    // Set DCO
	DCOCTL = CALDCO_1MHZ;
	UCA0CTL1 |= UCSWRST;                     // SMCLK
	P1SEL |= BIT1 + BIT2 ;                     // P1.1 = RXD, P1.2=TXD
	P1SEL2 |= BIT1 + BIT2 ;                    // P1.1 = RXD, P1.2=TXD
	UCA0CTL1 |= UCSSEL_2;                     // SMCLK
	UCA0BR0 = 104;                            // 1MHz 9600
	UCA0BR1 = 0;                              // 1MHz 9600
	UCA0MCTL = UCBRS0;                        // Modulation UCBRSx = 1
	UCA0CTL1 &= ~UCSWRST;                     // **Initialize USCI state machine**
	IE2 |= UCA0RXIE;                          // Enable USCI_A0 RX interrupt

	timer_init();

	_EINT();

	P1DIR |= BIT6;
	P1OUT &= ~BIT6;
	DQ_OUT;
	DQ_H;
	FlashRead((uchar*)0x1000,dryState,8);
	lcdinit();
	while(1)
	{
		int t,t1;
		P1OUT |= BIT6;
		t = (*(int*)temperature);
		GetTemp();              //温度转换函数。
		P1OUT &= ~BIT6;
		t1 = t-(*(int*)temperature);
		if(t!=-32768 && (t1<-16 || t1>16))
			(*(int*)temperature) = t;
		temptoa(*(int*)temperature,LCDBuf2+1,LCDBuf2+3);
		if(*dryState!=0xff){
			 FlashWrite((uchar*)0x1000,dryState,8);
			temptoa(*(int*)(dryState+2),LCDBuf1+1,LCDBuf1+3);
			timetoa(*(uint*)(dryState+6),LCDBuf1+5,LCDBuf1+9);
			timetoa(*(uint*)(dryState+4),LCDBuf2+5,LCDBuf2+9);
		}
		LCD_write_string(0,0,LCDBuf1);
		LCD_write_string(0,1,LCDBuf2);

		DelayNus(23000);
	}
}

#pragma vector=USCIAB0RX_VECTOR
__interrupt void usart0_rx(void)
{
	while(!(IFG2&UCA0TXIFG));
	RX_BUFF[TX_IndexR++] = UCA0RXBUF;
	if(UCA0RXBUF==0xfe){ // 接收完毕
		TX_IndexR=0;
		if(RX_BUFF[0]==0xfd){ // 帧头正确
			uchar i;
			TX_BUFF[1] = RX_BUFF[1];
			TX_BUFF[4] = 0xfe;
			switch(RX_BUFF[1])
			{
			case 0x00: // 传送实际温度
				*(int*)(TX_BUFF+2) = *(int*)(temperature);
				TX_IndexR=5;
				break;
			case 0x01: // 传送430芯片内实际温度
				adcinit();
		 		break;
			case 0x11:
				*(int*)(dryState+2) = *(int*)(RX_BUFF+2);
				break;
			case 0x21: // 置当前短号
				*(int*)dryState = *(int*)(RX_BUFF+2);
				*LCDBuf1 = "SURDP"[dryState[1]];
				*(int*)(dryState+4) = 0; // 清段时间
				if(*dryState==1){ // 如当前段号为1 ,清总时间
					*(int*)(dryState+6) = 0;
				}
				break;
			case 0xf0: // 取所有数据
				for(i=0;i<8;i++)
					TX_BUFF[i+2] = dryState[i];
				*(int*)(TX_BUFF+10) = *(int*)(temperature);
				TX_BUFF[12] = 0xfe;
				TX_IndexR=13;
				break;
			}
			for(i=0;i<TX_IndexR;i++){
				while (!(IFG2 & UCA0TXIFG)); // 判断发送缓冲区是否为空
				UCA0TXBUF = TX_BUFF[i];
			}
			TX_IndexR=0;

		}
	}
}
