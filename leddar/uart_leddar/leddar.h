/*
 * leddar.h
 *
 *  Created on: Feb 20, 2017
 *      Author: user
 */

#ifndef LEDDAR_H_
#define LEDDAR_H_

#define LEDDAR_DEV_NAME  "/dev/ttyLP2"

/* 串口的文件描述符 */
int leddar_fd;

void leddar_write_command(unsigned char address, unsigned char code, unsigned char sub_code);
int leddar_open_uart();

#endif /* LEDDAR_H_ */
