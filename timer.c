/*
 * timer.c
 *
 *  Created on: 2015��6��6��
 *      Author: LY
 */

#include <msp430g2553.h>
extern unsigned char dryState[];

void timer_init(void)
{
  TACCR0 =2500;  //�Ƶ�2500��Լ20ms
  TACTL |= TASSEL_2+ID_3+MC_1;
 //TASSEL_x������ʱ��Դѡ��x=0,��ʱ��TACLK��1ѡACLK��2ѡSMCLK��3ѡ�ⲿʱ��
// ID_x�Ƿ�Ƶѡ��x=0����Ƶ��1��2��Ƶ��2��4��Ƶ��3��8��Ƶ��125K��
// MC_x�Ǽ���ģʽ��x=0��ֹͣ��1��up��CCR0��2����������0xffff��3����������������CCR0���ټ���0
  TACCTL0 |= CCIE;                             // CCR0 interrupt enabled
}


//--------��ʱ���жϺ���------------
#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer_A(void)
{
  static int cc=0;
  cc++;
  if(cc==3000)  //1s=1000ms=20*50 ��ʱ
  {
	   cc=0;
	  *(unsigned int*)(dryState+4)+=1;
	  *(unsigned int*)(dryState+6)+=1;
  }
}
