/*
 * 1602.h
 *
 *  Created on: 2013-7-27
 *      Author: Qinkai
 */

#ifndef _1602_H_
#define _1602_H_
	extern void writecom(unsigned char com);
	extern void writedata(unsigned char dat);
	extern void lcdinit();
	extern void LCD_write_string(unsigned char x,unsigned char y,char *s);
	extern void LCD_write_int(unsigned char x,unsigned char y,unsigned char data);

#endif /* 1602_H_ */
