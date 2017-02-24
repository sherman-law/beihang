/*
 * leddar_protocol.c
 *
 *  Created on: Feb 21, 2017
 *      Author: user
 */
#include <stddef.h>
#include <pthread.h>
#include <sys/select.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "leddar.h"
#include "crc.h"
#include "leddar_protocol.h"

/**
 * \brief 雷达数据解析函数
 *
 *	\parma[out] leddar_data 雷达数据的结构体
 *	\parma[in]  frame_info  数据帧长度
 *	\parma[in]  rev_buf     串口数据缓存器
 */
void leddar_parse_detection (leddar_detection *leddar_data,
							 frame_info       *frame_info,
							 unsigned char    *rev_buf)
{
	int i = 0;
	int j = 0;

	for (i = 3; i < frame_info->frame_size - 12; i += 6) {
		leddar_data[j].distance  = (unsigned int)(rev_buf[i] |
				                                 (rev_buf[i + 1] << 8));
		leddar_data[j].amplitude = (unsigned int)(rev_buf[i + 2] |
												 (rev_buf[i + 3] << 8));
		leddar_data[j].flag      = rev_buf[i + 4];
		leddar_data[j].seg       = rev_buf[i + 5];
		j++;
	}
}

/**
 * \brief  读取全部数据函数
 * \detail 读取数据，并将数据整理放入buf中，并判断数据有效性
 *
 * \parma[in，out] frame_info 数据数量记录
 * \parma[out]     rev_buf    串口缓存器
 *
 * \retval 0 表示需要继续接收数据
 *         1 接受到有效数据帧
 */
unsigned int leddar_read_detection (frame_info *frame_info, unsigned char *rev_buf)
{
	unsigned int i = 0;
	unsigned int frame_head_found = 0;
	unsigned int frame_crc        = 0;

	int len = 0;

	len = read(leddar_fd,
			   rev_buf  + frame_info->bytes_received,
			   MAX_SIZE - frame_info->bytes_received);
	frame_info->bytes_received += len;

	/* 接收到数据的话 */
	while (frame_info->bytes_received > 0) {

		/* 接收到大于2的数据的话，就开始检测帧头 */
		if (frame_info->bytes_received >= 2) {

			printf("searching head of frame\n");

			for (i = 0; i < frame_info->bytes_received -1; i++) {
				if ((rev_buf[i]      == LEDDAR_ADDRESS) &&
					(rev_buf[i + 1]) == LEDDAR_DETECTIONS_CMD) {
					if (i > 0) {
						memmove(rev_buf, rev_buf + i, frame_info->bytes_received - i);
						frame_info->bytes_received = frame_info->bytes_received - i;
					}
					/* 表示已经接收到帧头 */
					frame_head_found = 1;
					printf("get head of frame\n");
					break;
				}
			}
		} else {  /* 检测不到帧头，就继续接收 */
			printf("no head of frame\n");
			return 0;
		}

		/* 接收到帧头的话，就开始判断数据的有效性 */
		if (frame_head_found) {

			/* 如果接收到的数据的长度小于3，就继续接收 */
			if (frame_info->bytes_received < 3) {
				printf("bytes_received < 3\n");
				return 0;
			}

			/* 计算数据应有的长度 */
			frame_info->frame_size = rev_buf[LEDDAR_DETECTIONS_NUM] * 6 + LEDDAR_FRAME_LEN_NO_DATA;

			/* 如果数据不在有效的范围内，则继续接受数据 */
			if ((frame_info->frame_size >= LEDDAR_FRAME_MIN_LEN) &&
				(frame_info->frame_size <= LEDDAR_FRAME_MAX_LEN)) {

				printf("get a right num of detections\n");
				/* 接收到探测数据的数值正确，但是数据还没有接收完全，则继续接收 */
				if (frame_info->frame_size > frame_info->bytes_received) {
					printf("not enough data\n");
					return 0;
				}
				printf("get crc code\n");
				frame_crc = (rev_buf[frame_info->frame_size - 1] << 8) | rev_buf[frame_info->frame_size - 2];

				/* crc校验 */
				if (frame_crc == crc16_calc(rev_buf, frame_info->frame_size - 2)) {
					/* 清空数据 */
					printf("crc code is right..^^..\n");

					return 1;

				} else {  /* 如果crc校验错误，清空buf */
					printf("crc code is wrong\n");

					memmove(rev_buf, rev_buf + frame_info->frame_size, frame_info->bytes_received - frame_info->frame_size);
					frame_info->bytes_received -= frame_info->frame_size;
					frame_info->frame_size = 0;
					frame_head_found = 0;
				}
			} else { /* 接收到错误的头部，将头部删去， */
				printf("get a wrong head\n");
				memmove(rev_buf, rev_buf + 2, frame_info->bytes_received - 2);
				frame_info->bytes_received = frame_info->bytes_received - 2;
				frame_info->frame_size = 0;
				frame_head_found = 0;
			}

		} else { /* 接收到长度小于2的数据，而且接收到了雷达的地址，就将其放在接收数组的开头 */
			printf("get a byte\n");
			if (rev_buf[frame_info->bytes_received - 1] == LEDDAR_ADDRESS) {
				rev_buf[0] = LEDDAR_ADDRESS;
				frame_info->bytes_received = 1;
			} else { /* 否则，就直接放弃接收到的数据 */
				frame_info->bytes_received = 0;
			}
			return 0;
		}
	}
	return 0;
}


