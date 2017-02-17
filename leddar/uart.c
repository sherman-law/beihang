/*
 * uart.c
 *
 *  Created on: Feb 13, 2017
 *      Author: user
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <sys/select.h>
#include <termios.h>
#include <errno.h>
#include <pthread.h>
#include <limits.h>

#define DEV_NAME  "/dev/ttyLP2"
#define MAX_SIZE  200

#define LEDDAR_ADDRESS           0x01
#define LEDDAR_DETECTIONS_CMD    0x41
#define LEDDAR_DETECTIONS_NUM    2
#define LEDDAR_FRAME_LEN_NO_DATA 12
#define LEDDAR_FRAME_MIN_LEN     54
#define LEDDAR_FRAME_MAX_LEN     66

#define LEDDAR_LED_PWR           6

int iFd = 0;
int bytes_received = 0;
int frame_size     = 0;

unsigned char rev_buf[MAX_SIZE];

/**
 * @brief 计算crc校验
 *
 * @parma[1] CRC_Buf :要计算crc的数组
 * @parma[2] CRC_Leni:数组的长度
 *
 * @retval 计算出来的crc校验码（小端）
 */
unsigned int crc16_calc (unsigned char *CRC_Buf, unsigned char CRC_Leni)
{
	unsigned char i, j;
	unsigned int CRC_Sumx;
	CRC_Sumx = 0xFFFF;

	for (i = 0; i < CRC_Leni; i++) {
		CRC_Sumx ^= *(CRC_Buf + i);
		for (j = 0; j < 8; j++) {
			if (CRC_Sumx & 0x01) {
				CRC_Sumx >>= 1;
				CRC_Sumx ^= 0xA001;
			} else {
				CRC_Sumx >>= 1;
			}
		}
	}
	return (CRC_Sumx);
}


/**
 * @brief:测试函数，读取部分数据并打印
 */
void read_some_of_data (void)
{
	int i   = 0;

	/* 如果接受的数据达到了目标数值 */
	if (bytes_received == (rev_buf[2] * 6 + 3)) {
		printf("Have received %d char\n", bytes_received);

		/* 打印接受到的指定数目的数据 */
		for (i = 0; i < bytes_received; i++) {
			printf("%.2x ", rev_buf[i]);
		}
		printf("\n");
		bzero(rev_buf, MAX_SIZE);
		bytes_received = 0;
	}
}

/**
 * @brief:  读取全部数据函数
 * @detail: 读取数据，并将数据整理放入buf中，并判断数据有效性
 */
unsigned int read_all_data (void)
{
	unsigned int i = 0;
	unsigned int frame_head_found = 0;
	unsigned int frame_crc        = 0;

	/* 接收到数据的话 */
	while (bytes_received > 0) {

		/* 接收到大于2的数据的话，就开始检测帧头 */
		if (bytes_received >= 2) {

			printf("searching head of frame\n");

			for (i = 0; i < bytes_received -1; i++) {
				if ((rev_buf[i]      == LEDDAR_ADDRESS)
				 && (rev_buf[i + 1]) == LEDDAR_DETECTIONS_CMD) {
					if (i > 0) {
						memmove(rev_buf, rev_buf + i, bytes_received - i);
						bytes_received = bytes_received - i;
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
			if (bytes_received < 3) {
				printf("bytes_received < 3\n");
				return 0;
			}

			/* 计算数据应有的长度 */
			frame_size = rev_buf[LEDDAR_DETECTIONS_NUM] * 6 + LEDDAR_FRAME_LEN_NO_DATA;

			/* 如果数据不在有效的范围内，则继续接受数据 */
			if ((frame_size >= LEDDAR_FRAME_MIN_LEN)
			 && (frame_size <= LEDDAR_FRAME_MAX_LEN)) {

				printf("get a right num of detections\n");
				/* 接收到探测数据的数值正确，但是数据还没有接收完全，则继续接收 */
				if (frame_size > bytes_received) {
					printf("not enough data\n");
					return 0;
				}
				printf("get crc code\n");
				frame_crc = (rev_buf[frame_size - 1] << 8) | rev_buf[frame_size - 2];

				for (i = 0; i < frame_size; i++) {
					printf("%x ", rev_buf[i]);
				}

				/* crc校验 */
				if (frame_crc == crc16_calc(rev_buf, frame_size - 2)) {
//					for (i = 0; i < frame_size; i++) {
//						printf("%x ", rev_buf[i]);
//					}
					printf("crc code is right..^^..\n");

				} else {  /* 如果crc校验错误，清空buf */
					printf("crc code is wrong\n");

					memmove(rev_buf, rev_buf + frame_size, bytes_received - frame_size);
					bytes_received -= frame_size;
					frame_size = 0;
					frame_head_found = 0;
				}
			} else {
				printf("get a wrong head\n");
				memmove(rev_buf, rev_buf + 2, bytes_received - 2);
				bytes_received = bytes_received - 2;
				frame_size = 0;
				frame_head_found = 0;
			}

		} else { /* 接收到长度小于2的数据，而且接收到了雷达的地址，就将其放在接收数组的开头 */
			printf("get a byte\n");
			if (rev_buf[bytes_received - 1] == LEDDAR_ADDRESS) {
				rev_buf[0] = LEDDAR_ADDRESS;
				bytes_received = 1;
			} else { /* 否则，就直接放弃接收到的数据 */
				bytes_received = 0;
			}
			return 0;
		}
	}
	return 0;
}

/**
 * @brief:串口接收数据的线程函数
 */
static void *PrintHello ()
{
	int num = 0;
	int len = 0;

	fd_set rfd;
	struct timeval tv;

	while (1) {
		FD_ZERO(&rfd);
		FD_SET(iFd, &rfd);
		tv.tv_sec  = 1;
		tv.tv_usec = 0;

		num = select(1 + iFd, &rfd, NULL, NULL, &tv);
		if (num > 0) {
			printf("num > 0!!!\n");
			if (FD_ISSET(iFd, &rfd)) {
				len = read(iFd, rev_buf + bytes_received, MAX_SIZE - bytes_received);
				bytes_received += len;
				read_all_data();
			}
		}
	}
	return 0;
}

int main (int argc, char *argv[])
{
	int rc  = 0;

	char command_buf[] = {0x01, 0x41, 0xC0, 0x10};

	pthread_t threads;
	struct termios opt;

	/* 打开串口设备 */
	iFd = open(DEV_NAME, O_RDWR | O_NOCTTY);
	if (iFd < 0) {
		perror(DEV_NAME);
		return -1;
	}

	/* 得到串口设备属性 */
	tcgetattr(iFd, &opt);
	if (tcgetattr(iFd, &opt) < 0) {
		return -1;
	}

	/* 设置串口波特率 */
	cfsetispeed(&opt, B115200);
	cfsetospeed(&opt, B115200);

	/* 配置串口参数 */
	opt.c_lflag &= ~(ICANON | ECHO | ECHOE |ECHONL | IEXTEN | ISIG);
	opt.c_oflag &= ~OPOST;
	opt.c_iflag &= ~(IXON | IXOFF);
	opt.c_iflag &= ~(BRKINT | ICRNL | INLCR  | IXANY |IGNCR);
	opt.c_oflag &= ~(ONLCR|OCRNL);
	opt.c_cflag &= ~(CSIZE | PARENB);
	opt.c_cflag |= CS8;

	opt.c_cc[VMIN]  = 1;
	opt.c_cc[VTIME] = 10;

	/* 设置串口属性 */
	if (tcsetattr(iFd, TCSANOW, &opt) < 0) {
		return -1;
	}

	/* 清空缓存区数据 */
	tcflush(iFd, TCIOFLUSH);

	/* 创建串口接受线程 */
	rc = pthread_create(&threads, NULL, PrintHello, NULL); /* 创建线程 */
	if (rc) {
		printf("ERROR; return code from pthread_create() is %d\n", rc);
		exit(-1);
	}

	/* 向雷达发送操作命令 */
	write(iFd, command_buf, sizeof(command_buf));
	printf("write command successfully!\n");

	/* 主线程，观察程序在运行 */
	while (1) {
		printf("main thread---\n");
		sleep(10);
	}

	return 0;
}
