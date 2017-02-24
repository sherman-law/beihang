/*
 * crc.c
 *
 *  Created on: Feb 20, 2017
 *      Author: user
 */

#include "crc.h"

/**
 * \brief 计算crc校验
 *
 * \parma[in] CRC_Buf  要计算crc的数组
 * \parma[in] CRC_Leni 数组的长度
 *
 * \retval 计算出来的crc校验码（小端）
 */
unsigned int crc16_calc (unsigned char *crc_buf, unsigned char crc_leni)
{
	unsigned char i, j;
	unsigned int crc_sumx;
	crc_sumx = 0xFFFF;

	for (i = 0; i < crc_leni; i++) {
		crc_sumx ^= *(crc_buf + i);
		for (j = 0; j < 8; j++) {
			if (crc_sumx & 0x01) {
				crc_sumx >>= 1;
				crc_sumx ^= 0xA001;
			} else {
				crc_sumx >>= 1;
			}
		}
	}
	return (crc_sumx);
}

