/*
 * leddar_protocol.h
 *
 *  Created on: Feb 21, 2017
 *      Author: user
 */

#ifndef LEDDAR_PROTOCOL_H_
#define LEDDAR_PROTOCOL_H_

#define MAX_SIZE  200

#define LEDDAR_ADDRESS           0x01
#define LEDDAR_DETECTIONS_CMD    0x41
#define LEDDAR_DETECTIONS_NUM    2
#define LEDDAR_FRAME_LEN_NO_DATA 12
#define LEDDAR_FRAME_MIN_LEN     54
#define LEDDAR_FRAME_MAX_LEN     66

#define LEDDAR_LED_PWR           6

/**
 * \brief 数据帧的数量
 */
typedef struct frame_info
{
	unsigned int bytes_received; /* 串口接收到的数据 */
	unsigned int frame_size;     /* 有效数据帧的长度 */

}frame_info;

/**
 * \brief leddar数据的数据结构
 */
typedef struct leddar_det
{
	unsigned char flag;          /* 标志位 */
	unsigned char seg;           /* 扇区号 */
	unsigned int  distance;      /* 距离 */
	unsigned int  amplitude;     /* 振幅 */

}leddar_detection;


unsigned int leddar_read_detection(frame_info *frame_info, unsigned char *rev_buf);
void leddar_parse_detection(leddar_detection *leddar_data, frame_info *frame_info, unsigned char *rev_buf);

#endif /* LEDDAR_PROTOCOL_H_ */
