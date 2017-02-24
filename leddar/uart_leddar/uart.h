/*
 * uart.h
 *
 *  Created on: Feb 20, 2017
 *      Author: user
 */

#ifndef UART_H_
#define UART_H_

int serial_open(char *port_name, int speed,int vtime,int vmin);
int serial_write(int fd, unsigned char *buf, int buf_size);
void serial_close(int fd);

#endif /* UART_H_ */
