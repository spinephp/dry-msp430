/*
 * flash.h
 *
 *  Created on: 2015��6��3��
 *      Author: LY
 */

#ifndef FLASH_H_
#define FLASH_H_

	extern void FlashRead(unsigned char *pc_byte,unsigned char *Dataout,unsigned int count);
	extern void FlashWrite(unsigned char *pc_byte,unsigned char *Datain,unsigned int count);


#endif /* FLASH_H_ */
