/*
 * pwm.h
 *
 *  Created on: 2026年8月10日
 *      Author: 13952
 */


#ifndef __PWM_H___
#define __PWM_H__
#include "fsl_ctimer.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define CTIMER_MAT_PWM_PERIOD_CHANNEL kCTIMER_Match_3

#define CTIMER_MAT0_OUT  kCTIMER_Match_0 /* Match output 0 */
#define CTIMER_MAT1_OUT  kCTIMER_Match_1 /* Match output 1 */
#define CTIMER_MAT2_OUT  kCTIMER_Match_2 /* Match output 1 */
#define CTIMER_MAT_MAX   4

#define CTIMER_MOTOR     CTIMER0
#define CTIMER_SERVO     CTIMER2

#define CTIMER_MOTOR_FREQ    1000
#define CTIMER_SERVO_FREQ    330


void pwm_motor_init();
void pwm_servo_init();
void update_pwm_duty_cycle(CTIMER_Type *timer, uint32_t ch0, uint32_t channel);
#endif
