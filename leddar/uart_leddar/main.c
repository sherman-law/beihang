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
#include <termios.h>
#include <errno.h>
#include <pthread.h>
#include <limits.h>

#include "crc.h"
#include "uart.h"
#include "leddar.h"

int main (int argc, char *argv[])
{
	leddar_open_uart();
	leddar_write_command(0x01, 0x41, 0);

	/* 主线程，观察程序在运行 */
	while (1) {
		printf("main thread---\n");
		sleep(2);
	}

	return 0;
}
