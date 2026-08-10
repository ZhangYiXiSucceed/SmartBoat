/*
 * rc.h
 *
 *  Created on: 2026年8月10日
 *      Author: 13952
 */

#ifndef RC_H_
#define RC_H_


/* ----------------------- Data Struct ------------------------------------- */
typedef struct
{
	struct
	{
		uint16_t ch0;
		uint16_t ch1;
		uint16_t ch2;
		uint16_t ch3;
		uint8_t s1;
		uint8_t s2;
	}rc;
	struct
	{
		int16_t x;
		int16_t y;
		int16_t z;
		uint8_t press_l;
		uint8_t press_r;
	}mouse;
	struct
	{
		uint16_t v;
	}key;
}RC_Ctl_t;
extern RC_Ctl_t RC_Ctl;
#define RC_DATA_RECIVE_LEN  18

void rc_uart_init();
void rc_dma_data_recive();
void print_remote_data();
#endif /* RC_H_ */
