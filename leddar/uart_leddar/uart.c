/**
 * \file       gpio.c
 * \brief      GPIO management
 *
 * \version    1.0
 * \date       Jan 1 2011
 * \author     Miguel Luis
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>

#include "uart.h"

/* 波特率的设置数组 */
static int speed_arr[] = {B115200, B38400, B19200, B9600, B4800, B2400, B1200, B300,
	                      B115200, B38400, B19200, B9600, B4800, B2400, B1200, B300};
static int name_arr[]  = {115200, 38400, 19200, 9600, 4800, 2400, 1200, 300,
	                      115200, 38400, 19200, 9600, 4800, 2400, 1200,  300,};

/**
 * \brief  设置串口波特率
 * \detail 根据用户设置在上面的数组中查询并设置相应的波特率
 *
 * \parma[in] fd    文件描述符
 * \parma[in] speed 串口波特率
 */
static void set_speed (int fd, int speed)
{
	int i;
	int status;
	struct termios Opt;

	tcgetattr(fd, &Opt);

	/* 在数组中搜索相应的波特率 */
	for (i = 0; i < sizeof(speed_arr) / sizeof(int); i++) {
		if (speed == name_arr[i]) {
			tcflush(fd, TCIOFLUSH);

			/* 设置串口波特率 */
			cfsetispeed(&Opt, speed_arr[i]);
			cfsetospeed(&Opt, speed_arr[i]);
			status = tcsetattr(fd, TCSANOW, &Opt);
			if (status != 0) {
				perror("tcsetattr fd1");
				return;
			}
			tcflush(fd,TCIOFLUSH);
		}
	}
}

/**
 * \brief  设置串口标志参数
 * \detail 标志设置为通用模式，主要修改的是阻塞时间和最小接收字符数
 *
 * \parma[in] fd    文件描述符
 * \parma[in] vmin  串口接收的最少字符数
 * \parma[in] vtime 串口接收数据阻塞时间
 *
 * \retval  0 设置成功
 * \retval -1 设置失败
 */
static int set_mode (int fd,int vmin,int vtime)
{
	struct termios options;

    if (tcgetattr(fd, &options) != 0) {
	    perror("SetupSerial 2");
	    return -1;
    }
    tcflush(fd, TCIFLUSH);

    /* 配置串口标志参数 */
	options.c_cc[VTIME] = vtime;
	options.c_cc[VMIN]  = vmin;

	options.c_lflag  &= ~(ICANON | ECHO | ECHOE | ECHONL | IEXTEN | ISIG);
	options.c_oflag  &= ~OPOST;
    options.c_iflag  &= ~(IXON | IXOFF);
    options.c_iflag  &= ~(BRKINT | ICRNL | INLCR  | IXANY | IGNCR);
    options.c_oflag  &= ~(ONLCR | OCRNL);

    /* 设置配置好的参数 */
    if (tcsetattr(fd, TCSANOW, &options) != 0) {
    	perror("SetupSerial 4");
    	return -1;
    }
    tcflush(fd, TCIFLUSH);
    return 0;
}

/**
 * \brief  设置串口数据格式参数
 *
 * \parma[in] fd       文件描述符
 * \parma[in] databits 串口数据长度
 * \parma[in] stopbits 串口停止位
 * \parma[in] parity   串口奇偶校验位
 *
 * \retval  0 设置成功
 * \retval -1 设置失败
 */
static int set_parity (int fd, int databits, int stopbits, int parity)
{
	struct termios options;

	/* 得到当前串口的参数 */
	if (tcgetattr(fd, &options) != 0) {
		perror("SetupSerial 1");
		return -1;
	}

	options.c_cflag &= ~CSIZE;

	/* 设置数据长度 */
	switch (databits) {
	case 7:
		options.c_cflag |= CS7;
		break;
	case 8:
		options.c_cflag |= CS8;
		break;
	default:
		fprintf(stderr, "Unsupported data size\n");
		return -1;
	}

	/* 设置奇偶校验位 */
	switch (parity) {
	case 'n':
	case 'N':
		options.c_cflag &= ~PARENB;
		options.c_iflag &= ~INPCK;
		break;
	case 'o':
	case 'O':
		options.c_cflag |= (PARODD | PARENB);
		options.c_iflag |= INPCK;
		break;
	case 'e':
	case 'E':
		options.c_cflag |= PARENB;
		options.c_cflag &= ~PARODD;
		options.c_iflag |= INPCK;
		break;
	case 'S':
	case 's':
		options.c_cflag &= ~PARENB;
		options.c_cflag &= ~CSTOPB;
		break;
	default:
		fprintf(stderr, "Unsupported parity\n");
		return -1;
	}

	/* 设置停止标志位 */
	switch (stopbits) {
	case 1:
		options.c_cflag &= ~CSTOPB;
		break;
	case 2:
		options.c_cflag |= CSTOPB;
		break;
	default:
		fprintf(stderr,"Unsupported stop bits\n");
		return -1;
	}
	if (parity != 'n') {
		options.c_iflag |= INPCK;
	}
	tcflush(fd, TCIFLUSH);

	/* 设置配置好的参数 */
	if (tcsetattr(fd, TCSANOW, &options) != 0) {
		perror("SetupSerial 3");
		return -1;
	}
	return 0;
}

/**
 * \brief  用户使用的打开串口函数
 *
 * \parma[in] port_name 文件描述符
 * \parma[in] speed     串口波特率
 * \parma[in] vtime     串口阻塞时间
 * \parma[in] vmin      串口最少接收字符
 *
 * \retval  fd 文件描述符
 */
int serial_open (char *port_name, int speed, int vtime, int vmin)
{
	int fd = -1;

	fd = open(port_name, O_RDWR);
	if (fd < 0) {
		printf("Failed to open %s\n", port_name);
		return -1;
	}
	set_speed(fd, speed);
	if (set_parity(fd, 8, 1,'N') < 0) {
		printf("Set Parity Error\n");
		return -1;
	}
    if (set_mode(fd, vmin, vtime) < 0) {
    	printf("Set mode Error\n");
		return -1;
	}
	return fd;
}

/**
 * \brief  用户使用的关闭串口函数
 *
 * \parma[in] fd 文件描述符
 */
void serial_close (int fd)
{
	if (fd > 0) {
		close(fd);
	}
}

/**
 * \brief  用户使用的发送数据函数（使用前必须打开串口）
 *
 * \parma[in] fd       文件描述符
 * \parma[in] buf      发送数据的数组收地址
 * \parma[in] buf_size 发送的数据长度
 */
int serial_write (int fd, unsigned char *buf, int buf_size)
{
	return write(fd, buf, buf_size);
}



