#include  "msp430g2553.h"
#include  "1602.h"
#include  "flash.h"
#include  "adc.h"
#include "stdint.h"
#include <stdio.h>

typedef unsigned char uchar;
typedef unsigned int  uint;

#define RXBUF_SIZE  16

char RX_BUFF[RXBUF_SIZE] = { 0}; 									//receive buffer

uchar TX_BUFF[13] = { 0xfd}; 									//

uint TX_IndexR = 0; 						//FIFO pointer for RX

char temperature[2]={0};
uchar Error = 0;
int tmp;
char *p;

uchar dryState[8]; // 0,1-curDryLineNo, 2,3-settingTemperature 4,5-curLineTime 6,7-totalTime
uint dryStateChange= 1;

#define   DQ_OUT     P1DIR|=BIT0
#define   DQ_IN      P1DIR&=~BIT0
#define   DQ_H       P1OUT|=BIT0
#define   DQ_L       P1OUT&=~BIT0
#define   DQ_DATA    (P1IN&BIT0)


#define SegmentStart 0x01000  //Segment = 0x01000~0x010FF
#define SegmentSize 255 //SegmentA + SegmentB = 256

char LCDBuf1[]={"S-99 00:00 00:00"};   //第一行要显示的内容
char LCDBuf2[]={"M-99 00:00 00:00"};     //第二行要显示的内容
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

void temptoa(int i,char *s,char *p)
{
	if(i<0) {
		*s++ = '-';
		i = -i;
	}
	ntoa(i>>4,s,p);
}

void ntoa(int i,char *s,char *p)
{
	while(i){
		*p-- = (char)((i %10) +0x30);
		i /= 10;
	}
	while(s<=p)
		*s++ = 0x30;
}
//-----------------------------------
//功能：写18B20
//-----------------------------------
void Write_18B20(unsigned char n)
{
	uint i=8;
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
		n=n>>1;
		DelayNus(5);//延时50us 以上
		DQ_H;
	}
}

//-----------------------------------
//功能：读18B20
//-----------------------------------
uchar Read_18B20(void)
{
	uint i=8;
	uchar temp;
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
		Error = 1;
		DQ_OUT;
	}
	else
	{
		Error = 0;
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

/*
* Put data into buffer and enable the interruption
int UART0_PutFrame(unsigned char *Ptr, unsigned int Lenth) {
	int i;
	if (IE2 & UCA0TXIE) {
		return (0);
	}
	if (Lenth > TXBUF_SIZE) {
		return (0);
	}
	for (i = 0; i < Lenth; i++) {
		TX_BUFF[i] = Ptr[i];
	}
	TX_IndexR = 0;
	UART_OutLen = Lenth;
	IFG2 |= UCA0TXIFG;
	IE2 |= UCA0TXIE;
	return (1);
}*/


/*
* try to send one char
*/
/*
void UART_sent(uchar Chr) {
	IFG2 &= ~UCA0TXIFG;                            // 清除发送标志位
	UCA0TXBUF = Chr;
	while ((IFG2 & UCA0TXIFG) == 0);				// USCI_A0 TX buffer ready?
}
uchar UART_read(void){
	uchar c;
	while(!(IFG2 & UCA0RXIFG));
	c = UCA0RXBUF;
	IFG2 &= ~UCA0TXIFG;
	return c;
}
*/
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
	//P1DIR |= BIT2;
	//P1DIR &= ~BIT1;
	//UCA0CTL0 &= ~UC7BIT;                       // 字符长度为 8
	UCA0CTL1 |= UCSSEL_2;                     // SMCLK
	UCA0BR0 = 104;                            // 1MHz 9600
	UCA0BR1 = 0;                              // 1MHz 9600
	UCA0MCTL = UCBRS0;                        // Modulation UCBRSx = 1
	UCA0CTL1 &= ~UCSWRST;                     // **Initialize USCI state machine**
	IE2 |= UCA0RXIE;                          // Enable USCI_A0 RX interrupt

	_EINT();
	//__bis_SR_register(LPM1_bits+GIE);

	P1DIR |= BIT6;
	P1OUT &= ~BIT6;
	DQ_OUT;
	DQ_H;
	FlashRead((uchar*)0x1000,dryState,8);
	//P2SEL=0x00;
	//P2DIR = 0xff;
	//P1DIR = 0x38;
	lcdinit();
	//LCD_prints(0,1,LCDBuf2);
	while(1)
	{
		int t;
		P1OUT |= BIT6;
		t = (*(int*)temperature);
		GetTemp();              //温度转换函数。
		P1OUT &= ~BIT6;
		if(t!=(*(int*)temperature)){
			temptoa(*(int*)temperature,LCDBuf2+1,LCDBuf2+3);
			dryStateChange = 1;
		}
		if(dryStateChange){
			dryStateChange = 0;
			FlashWrite((uchar*)0x1000,dryState,8);
			temptoa(*(int*)(dryState+2),LCDBuf1+1,LCDBuf1+3);
			LCD_write_string(0,0,LCDBuf1);
			LCD_write_string(0,1,LCDBuf2);
		}

		DelayNus(23000);
	}
}

#pragma vector=USCIAB0RX_VECTOR
__interrupt void usart0_rx(void)
{
	//_DINT();
	while(!(IFG2&UCA0TXIFG));
	//_DINT();
	//IFG2&=~UCA0RXIFG;
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
				//TX_BUFF[1] = 0x0;
				*(int*)(TX_BUFF+2) = *(int*)(temperature);
				TX_IndexR=5;
				break;
			case 0x01: // 传送430芯片内实际温度
				adcinit();
				/*ADC10CTL0 = SREF_1+ADC10SHT_2+REF2_5V+REFON+ADC10ON+ADC10IE+MSC;
				ADC10CTL1 = INCH_10+CONSEQ_2;
				ADC10DTC1 = Num_of_Results;
				ADC10SA = (short)(IN_temper);
		    	ADC10CTL0 |= ENC+ADC10SC;//使能转换并且开始转换
				*/
		 		break;
				/*
			case 0x10: // 传送设定温度
				TX_BUFF[1] = 0x10;
				TX_BUFF[2] = dryState[2];
				TX_BUFF[3] = dryState[3];
				TX_IndexR=5;
				break;
*/
			case 0x11:
				dryState[2] = RX_BUFF[2];
				dryState[3] = RX_BUFF[3];
				dryStateChange = 1;
				//FlashWrite((uchar*)0x1002,dryState+2,2);
				break;
				/*
			case 0x20:// 取当前短号
				TX_BUFF[1] = 0x20;
				TX_BUFF[2] = dryState[0];
				TX_BUFF[3] = dryState[1];
				TX_IndexR=5;
				break;
*/
			case 0x21: // 置当前短号
				dryState[0] = RX_BUFF[2];
				dryState[1] = RX_BUFF[3];
				dryState[4] = 0x0;
				dryState[5] = 0x0;
				dryStateChange = 1;
				//FlashWrite((uchar*)0x1000,dryState,6);
				break;
				/*
			case 0x40: // 当前段时间
				TX_BUFF[1] = 0x40;
				TX_BUFF[2] = dryState[4];
				TX_BUFF[3] = dryState[5];
				TX_IndexR=5;
				break;
			case 0x41:
				dryState[4] = RX_BUFF[2];
				dryState[5] = RX_BUFF[3];
				FlashWrite((uchar*)0x1000,dryState+4,2);
				break;
*/
/*			case 0x50: // 总时间
				TX_BUFF[1] = 0x50;
				TX_BUFF[2] = dryState[6];
				TX_BUFF[3] = dryState[7];
				TX_IndexR=5;
				break;
			case 0x51:
				dryState[6] = RX_BUFF[2];
				dryState[7] = RX_BUFF[3];
				FlashWrite((uchar*)0x1000,dryState+6,2);
				break;
*/
			case 0xf0: // 取所有数据
				//TX_BUFF[1] = 0xf0;
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
			//UART_sent(TX_BUFF[i]);
			TX_IndexR=0;

		}
	}
	//_EINT();
}

