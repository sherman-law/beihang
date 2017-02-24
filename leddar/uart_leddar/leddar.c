/*
 * leddar.c
 *
 *  Created on: Feb 20, 2017
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
#include "uart.h"
#include "leddar_protocol.h"

/* 用来缓存接收数据的buf */
unsigned char leddar_buf[MAX_SIZE];

/* 接收到数据的信息（数据的长度，帧长度） */
frame_info frame_info_leddar = {0,0};

static void *serial_rev_thread ();

/**
 * \brief  打开leddar使用的串口，创建接收数据的线程
 */
int leddar_open_uart ()
{
	int rc = 0;

	pthread_t threads;

	leddar_fd = serial_open(LEDDAR_DEV_NAME, 115200, 0, 1);

	/* 创建串口接受线程 */
	rc = pthread_create(&threads, NULL, serial_rev_thread, NULL); /* 创建线程 */
	if (rc) {
		printf("ERROR; return code from pthread_create() is %d\n", rc);
		exit(-1);
	}

	return leddar_fd;
}

/**
 * \brief 向雷达发送操作码
 *
 * \parma[in] address  设备的地址
 * \parma[in] code     设备的功能码
 * \parma[in] sub_code 副功能码
 */
void leddar_write_command (unsigned char address,
						   unsigned char code,
						   unsigned char sub_code)
{
	unsigned int  crc_sumx = 0;
	unsigned char send_command[6];

	/* 将地址和操作码放入数组中 */
	send_command[0] = address;
	send_command[1] = code;

	if (sub_code == 0) {
		/* 计算crc校验码 */
		crc_sumx = crc16_calc(send_command, 2);

		/* 将crc校验码放入数组中 */
		send_command[2] = crc_sumx & 0xff;
		send_command[3] = crc_sumx >> 8;

		/* 发送操作命令 */
		serial_write(leddar_fd, send_command, 4);

	} else {
		send_command[2] = sub_code;

		/* 计算crc校验码 */
		crc_sumx = crc16_calc(send_command, 3);

		/* 将crc校验码放入数组中 */
		send_command[3] = crc_sumx & 0xff;
		send_command[4] = crc_sumx >> 8;

		/* 发送操作命令 */
		serial_write(leddar_fd, send_command, 5);
	}
}

/**
 * \brief 取出有效的数据
 *
 * \parma[out] leddar_frame 雷达数据的结构体
 */
void leddar_get_advance (leddar_detection *leddar_frame)
{
	/* 解析buf中的数据，并提取至结构体中去 */
	leddar_parse_detection(leddar_frame, &frame_info_leddar, leddar_buf);
}

/**
 * \brief  串口接收数据的线程函数
 * \detail 线程中调用leddar_read_detection（）函数以接收一个完整的数据帧
 *
 */
static void *serial_rev_thread ()
{
	int num = 0;
	int i   = 0;

	fd_set rfd;        /* 文件描述符监视器 */
	struct timeval tv; /* select函数阻塞时间 */

	leddar_detection test[8];

	while (1) {
		FD_ZERO(&rfd);           /* 清空文件描述符监视器 */
		FD_SET(leddar_fd, &rfd); /* 将leddar的文件描述符绑定到监视器上面 */
		tv.tv_sec  = 0;
		tv.tv_usec = 80000;      /* 设置select函数阻塞时间 */

		/* 开启文件描述符监听器 */
		num = select(1 + leddar_fd, &rfd, NULL, NULL, &tv);
		if (num > 0) {
			printf("num > 0!!!\n");
			if (FD_ISSET(leddar_fd, &rfd)) {

				/* 读取有效的数据帧 */
				if (leddar_read_detection(&frame_info_leddar, leddar_buf)) {
					for (i = 0; i < frame_info_leddar.frame_size; i++) {
						printf("%.2x ", leddar_buf[i]);
					}
					printf("\n");

					leddar_get_advance(test);

					for (i = 0; i < 8; i++) {
						printf("****************************\n");
						printf("distance:%d\n",  test[i].distance);
						printf("amplitude:%d\n", test[i].amplitude);
						printf("seg:%d\n",       test[i].seg);
						printf("flag:%d\n",      test[i].flag);
						printf("****************************\n");
					}

					bzero(leddar_buf, frame_info_leddar.frame_size);
					frame_info_leddar.frame_size = 0;
					frame_info_leddar.bytes_received -= frame_info_leddar.frame_size;
					printf("clean complete\n");
				}
			}
		}
	}
	return 0;
}
