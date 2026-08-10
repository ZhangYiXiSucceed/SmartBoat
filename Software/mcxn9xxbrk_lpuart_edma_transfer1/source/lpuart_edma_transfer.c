/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "stdio.h"
#if defined(FSL_FEATURE_SOC_DMAMUX_COUNT) && FSL_FEATURE_SOC_DMAMUX_COUNT
#include "fsl_dmamux.h"
#endif
#include "fsl_debug_console.h"
#include "fsl_ctimer.h"
#include "pwm.h"
#include "timer.h"
#include "rc.h"
#include "temperature.h"
/*!
 * @brief Main function
 */
int main(void)
{
    /* attach FRO 12M to FLEXCOMM4 (debug console) */
    CLOCK_SetClkDiv(kCLOCK_DivFlexcom4Clk, 1u);
    CLOCK_AttachClk(BOARD_DEBUG_UART_CLK_ATTACH);

    BOARD_InitBootPins();
    BOARD_PowerMode_OD();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();

    CLOCK_EnableClock(kCLOCK_Gpio3);
    LED_RED_INIT(LOGIC_LED_ON);
    LED_BLUE_INIT(LOGIC_LED_ON);
    LED_GREEN_INIT(LOGIC_LED_ON);

    timer_init();
    pwm_motor_init();
    pwm_servo_init();
    rc_uart_init();
    temperature_init();

    while (1)
    {
    	rc_dma_data_recive();
        //print_remote_data();
        update_pwm_duty_cycle(CTIMER_MOTOR, RC_Ctl.rc.ch0, CTIMER_MAT0_OUT);
        update_pwm_duty_cycle(CTIMER_MOTOR, RC_Ctl.rc.ch1, CTIMER_MAT1_OUT);
        update_pwm_duty_cycle(CTIMER_MOTOR, RC_Ctl.rc.ch3, CTIMER_MAT2_OUT);

        update_pwm_duty_cycle(CTIMER_SERVO, RC_Ctl.rc.ch2, CTIMER_MAT0_OUT);


        print_temperature();
    }
}
