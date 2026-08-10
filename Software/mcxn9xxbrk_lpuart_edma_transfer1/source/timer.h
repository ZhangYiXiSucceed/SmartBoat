/*
 * timer.h
 *
 *  Created on: 2026年8月10日
 *      Author: 13952
 */

#ifndef TIMER_H_
#define TIMER_H_
#include "fsl_ctimer.h"
#define BOARD_HAS_NO_CTIMER_OUTPUT_PIN_CONNECTED_TO_LED

#define CTIMER_MAT0_OUT kCTIMER_Match_0 /* Match output 0 */
#define CTIMER_EMT0_OUT (1u << kCTIMER_Match_0)
#define CTIMER_MAT1_OUT kCTIMER_Match_1 /* Match output 1 */
#define CTIMER_EMT1_OUT (1u << kCTIMER_Match_1)
#define CTIMER_CLK_FREQ CLOCK_GetCTimerClkFreq(4U)

#define CTIMER_THREAD    CTIMER4
#define CTIMER_THREAD_FREQ   1000

extern uint8_t freq_1Hz_flag_g;
void timer_init();
#endif /* TIMER_H_ */
