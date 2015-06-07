/*
 * timer.c
 *
 *  Created on: 2015年6月6日
 *      Author: LY
 */

#include <msp430g2553.h>
extern unsigned char dryState[];

void timer_init(void)
{
  TACCR0 =2500;  //计到2500，约20ms
  TACTL |= TASSEL_2+ID_3+MC_1;
 //TASSEL_x计数器时钟源选择，x=0,外时钟TACLK，1选ACLK，2选SMCLK，3选外部时钟
// ID_x是分频选择，x=0不分频，1是2分频，2是4分频，3是8分频后（125K）
// MC_x是计数模式，x=0，停止，1是up到CCR0，2是连续到满0xffff，3增减计数，先增到CCR0，再减到0
  TACCTL0 |= CCIE;                             // CCR0 interrupt enabled
}


//--------定时器中断函数------------
#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer_A(void)
{
  static int cc=0;
  cc++;
  if(cc==3000)  //1s=1000ms=20*50 定时
  {
	   cc=0;
	  *(unsigned int*)(dryState+4)+=1;
	  *(unsigned int*)(dryState+6)+=1;
  }
}
