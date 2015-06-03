/*
 * adc.c
 *
 *  Created on: 2013-7-29
 *      Author: Qinkai
 */

#include <msp430g2553.h>
extern char TX_BUFF[];
#define   Num_of_Results   16
unsigned short IN_temper[16] = {0};

void adcinit(void)
{
	ADC10CTL0 = SREF_1+ADC10SHT_2+REF2_5V+REFON+ADC10ON+ADC10IE+MSC;
	ADC10CTL1 = INCH_10+CONSEQ_2;
	ADC10DTC1 = Num_of_Results;
	ADC10SA = (short)(IN_temper);
	ADC10CTL0 |= ENC+ADC10SC;//ʹ��ת�����ҿ�ʼת��
}

#pragma vector=ADC10_VECTOR
__interrupt void ADC10_ISR(void)
{
	ADC10CTL0 &=~ADC10ON;
	//ADC10CTL0 &= ~(ENC+ADC10SC);//ʹ��ת�����ҿ�ʼת��
	//ADC10CTL0&=~ADC10IFG;//����жϱ�־λ
    unsigned int sum = 0,i = 0;
	for(i=0;i<Num_of_Results;i++)
	{
		sum += IN_temper[i];                            //����32���ƽ��ֵ
	}
	sum >>= 4;
		//IN_temper=((1.5*sum/1023)-0.986)/0.00355;//����1000��
	//TX_BUFF[4] = 0xfe;
	//TX_BUFF[1] = 0x01;
	*(int*)(TX_BUFF+2) = (int)(sum);
	for(i=0;i<5;i++){
		while (!(IFG2 & UCA0TXIFG)); // �жϷ��ͻ������Ƿ�Ϊ��
		UCA0TXBUF = TX_BUFF[i];
	}
}

