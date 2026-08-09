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
#include "fsl_lpuart_edma.h"
#if defined(FSL_FEATURE_SOC_DMAMUX_COUNT) && FSL_FEATURE_SOC_DMAMUX_COUNT
#include "fsl_dmamux.h"
#endif
#include "fsl_debug_console.h"
#include "fsl_ctimer.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

#ifndef CTIMER_MAT_PWM_PERIOD_CHANNEL
#define CTIMER_MAT_PWM_PERIOD_CHANNEL kCTIMER_Match_3
#endif

#define CTIMER_MAT0_OUT  kCTIMER_Match_0 /* Match output 0 */
#define CTIMER_MAT1_OUT  kCTIMER_Match_1 /* Match output 1 */
#define CTIMER_MAT2_OUT  kCTIMER_Match_2 /* Match output 1 */
#define CTIMER_MAT_MAX   4


#define CTIMER_PMW2   CTIMER2

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define DEMO_LPUART                    LPUART4
#define DEMO_LPUART_CLK_FREQ           CLOCK_GetLPFlexCommClkFreq(4u)
#define LPUART_TX_DMA_CHANNEL          0U
#define LPUART_RX_DMA_CHANNEL          1U
#define DEMO_LPUART_TX_EDMA_CHANNEL    kDmaRequestMuxLpFlexcomm4Tx
#define DEMO_LPUART_RX_EDMA_CHANNEL    kDmaRequestMuxLpFlexcomm4Rx
#define EXAMPLE_LPUART_DMAMUX_BASEADDR DMAMUX
#define EXAMPLE_LPUART_DMA_BASEADDR    DMA0
#define ECHO_BUFFER_LENGTH 18

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/* LPUART user callback */
void LPUART_UserCallback(LPUART_Type *base, lpuart_edma_handle_t *handle, status_t status, void *userData);

/*******************************************************************************
 * Variables
 ******************************************************************************/


lpuart_edma_handle_t g_lpuartEdmaHandle;
edma_handle_t g_lpuartTxEdmaHandle;
edma_handle_t g_lpuartRxEdmaHandle;
AT_NONCACHEABLE_SECTION_INIT(uint8_t g_tipString[]) =
    "LPUART EDMA example\r\nSend back received data\r\nEcho every 8 characters\r\n";
AT_NONCACHEABLE_SECTION_INIT(uint8_t g_txBuffer[128]) = {0};
AT_NONCACHEABLE_SECTION_INIT(uint8_t g_rxBuffer[ECHO_BUFFER_LENGTH]) = {0};
AT_NONCACHEABLE_SECTION_INIT(uint8_t sbus_rx_buffer[ECHO_BUFFER_LENGTH]) = {0};
volatile bool rxBufferEmpty                                          = true;
volatile bool txBufferFull                                           = false;
volatile bool txOnGoing                                              = false;
volatile bool rxOnGoing                                              = false;
#if (defined(DEMO_EDMA_HAS_CHANNEL_CONFIG) && DEMO_EDMA_HAS_CHANNEL_CONFIG)
extern edma_config_t config;
#else
edma_config_t config;
#endif
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

#define BOARD_HAS_NO_CTIMER_OUTPUT_PIN_CONNECTED_TO_LED

#define CTIMER_MAT0_OUT kCTIMER_Match_0 /* Match output 0 */
#define CTIMER_EMT0_OUT (1u << kCTIMER_Match_0)
#define CTIMER_MAT1_OUT kCTIMER_Match_1 /* Match output 1 */
#define CTIMER_EMT1_OUT (1u << kCTIMER_Match_1)
#define CTIMER_CLK_FREQ CLOCK_GetCTimerClkFreq(4U)

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
void ctimer_match0_callback(uint32_t flags);
void ctimer_match1_callback(uint32_t flags);

/* Array of function pointers for callback for each channel */
ctimer_callback_t ctimer_callback_table[] = {
    ctimer_match0_callback, NULL, NULL, NULL, NULL, NULL, NULL, NULL};

/*******************************************************************************
 * Variables
 ******************************************************************************/
/* Match Configuration for Channel 0 */
static ctimer_match_config_t matchConfig0;
/* Match Configuration for Channel 1 */
static ctimer_match_config_t matchConfig1;

/*******************************************************************************
 * Code
 ******************************************************************************/

void ctimer_match0_callback(uint32_t flags)
{
#if defined(BOARD_HAS_NO_CTIMER_OUTPUT_PIN_CONNECTED_TO_LED)
    /* No timer match output pin connected to a LED
     * toggle LED manually according to match status
     */
    if (CTIMER_GetOutputMatchStatus(CTIMER4, CTIMER_EMT0_OUT))
    {
    	LED_GREEN_ON();
    }
    else
    {
    	LED_GREEN_OFF();
    }
#endif
}

RC_Ctl_t RC_Ctl;
int len = 0;
void data_process(void)
{
		RC_Ctl.rc.ch0 = (sbus_rx_buffer[0]| (sbus_rx_buffer[1] << 8)) & 0x07ff; //!< Channel 0
		RC_Ctl.rc.ch1 = ((sbus_rx_buffer[1] >> 3) | (sbus_rx_buffer[2] << 5)) & 0x07ff; //!< Channel 1
		RC_Ctl.rc.ch2 = ((sbus_rx_buffer[2] >> 6) | (sbus_rx_buffer[3] << 2) | (sbus_rx_buffer[4] << 10)) & 0x07ff; //!< Channel 2
		RC_Ctl.rc.ch3 = ((sbus_rx_buffer[4] >> 1) | (sbus_rx_buffer[5] << 7)) & 0x07ff; //!< Channel 3
		RC_Ctl.rc.s1 = ((sbus_rx_buffer[5] >> 4)& 0x000C) >> 2; //!< Switch left
		RC_Ctl.rc.s2 = ((sbus_rx_buffer[5] >> 4)& 0x0003); //!< Switch right9 / 9
		RC_Ctl.mouse.x = sbus_rx_buffer[6] | (sbus_rx_buffer[7] << 8); //!< Mouse X axis
		RC_Ctl.mouse.y = sbus_rx_buffer[8] | (sbus_rx_buffer[9] << 8); //!< Mouse Y axis
		RC_Ctl.mouse.z = sbus_rx_buffer[10] | (sbus_rx_buffer[11] << 8); //!< Mouse Z axis
		RC_Ctl.mouse.press_l = sbus_rx_buffer[12]; //!< Mouse Left Is Press ?
		RC_Ctl.mouse.press_r = sbus_rx_buffer[13]; //!< Mouse Right Is Press ?
		RC_Ctl.key.v = sbus_rx_buffer[14] | (sbus_rx_buffer[15] << 8); //!< KeyBoard value



}
/*******************************************************************************
 * Code
 ******************************************************************************/
/* LPUART user callback */
lpuart_config_t lpuartConfig;
   lpuart_transfer_t xfer;
   lpuart_transfer_t sendXfer;
   lpuart_transfer_t receiveXfer;
   int count=0;
void LPUART_UserCallback(LPUART_Type *base, lpuart_edma_handle_t *handle, status_t status, void *userData)
{
    userData = userData;

    if (kStatus_LPUART_TxIdle == status)
    {
        txBufferFull = false;
        txOnGoing    = false;
    }

    if (kStatus_LPUART_RxIdle == status)
    {
        rxBufferEmpty = false;
        rxOnGoing     = false;
        data_process();

                	LPUART_TransferGetReceiveCountEDMA(DEMO_LPUART, &g_lpuartEdmaHandle,&count);
        //LPUART_TransferAbortReceiveEDMA(DEMO_LPUART, &g_lpuartEdmaHandle);
        //LPUART_ReceiveEDMA(DEMO_LPUART, &g_lpuartEdmaHandle, &receiveXfer);

    }

}



void uart_write(uint8_t* data, int len)
{
	int i=0;
	while(i<len)
	{
		DbgConsole_Putchar(data[i]);
		i++;
	}
}


void print_remote_data()
{
    {
    	/* 读取到的数据通过串口5错位输出 */
    	len = sprintf(g_txBuffer, "%4d %4d %4d %4d %4d %4d\r\n", RC_Ctl.rc.ch0, RC_Ctl.rc.ch1, RC_Ctl.rc.ch2, RC_Ctl.rc.ch3, \
    				RC_Ctl.rc.s1,RC_Ctl.rc.s2);
    	uart_write(g_txBuffer, len);
    }
}

void pwm1_init()
{
    ctimer_config_t config;
    uint32_t srcClock_Hz;
    uint32_t timerClock;

    /* CTimer0 counter uses the AHB clock, some CTimer1 modules use the Aysnc clock */
    srcClock_Hz = CLOCK_GetCTimerClkFreq(0U);

    PRINTF("CTimer example to generate a PWM1 signal\r\n");

    CTIMER_GetDefaultConfig(&config);
    timerClock = srcClock_Hz / (config.prescale + 1);

    CTIMER_Init(CTIMER0, &config);

    /* Calculate PWM period match value */
    uint32_t pwmPeriod = (timerClock / 1000) - 1U;

   /* Calculate pulse width match value */
    uint32_t pulsePeriod = (pwmPeriod + 1U) * (100 - 20) / 100;

    CTIMER_SetupPwmPeriod(CTIMER0, CTIMER_MAT_PWM_PERIOD_CHANNEL, CTIMER_MAT0_OUT, pwmPeriod, pulsePeriod, false);
    CTIMER_SetupPwmPeriod(CTIMER0, CTIMER_MAT_PWM_PERIOD_CHANNEL, CTIMER_MAT1_OUT, pwmPeriod, pulsePeriod, false);
    CTIMER_SetupPwmPeriod(CTIMER0, CTIMER_MAT_PWM_PERIOD_CHANNEL, CTIMER_MAT2_OUT, pwmPeriod, pulsePeriod, false);
    CTIMER_StartTimer(CTIMER0);
}



void pwm2_init()
{
	ctimer_config_t config;
	uint32_t srcClock_Hz;
	uint32_t timerClock;

	/* CTimer0 counter uses the AHB clock, some CTimer1 modules use the Aysnc clock */
	srcClock_Hz = CLOCK_GetCTimerClkFreq(0U);

	PRINTF("CTimer example to generate a PWM2 signal\r\n");

	CTIMER_GetDefaultConfig(&config);
	timerClock = srcClock_Hz / (config.prescale + 1);

	CTIMER_Init(CTIMER_PMW2, &config);

	/* Calculate PWM period match value */
	uint32_t pwmPeriod = (timerClock / 330) - 1U;

   /* Calculate pulse width match value */
	uint32_t pulsePeriod = (pwmPeriod + 1U) * (100 - 20) / 100;

	CTIMER_SetupPwmPeriod(CTIMER_PMW2, CTIMER_MAT_PWM_PERIOD_CHANNEL, CTIMER_MAT0_OUT, pwmPeriod, pulsePeriod, false);
	CTIMER_SetupPwmPeriod(CTIMER_PMW2, CTIMER_MAT_PWM_PERIOD_CHANNEL, CTIMER_MAT1_OUT, pwmPeriod, pulsePeriod, false);
	CTIMER_SetupPwmPeriod(CTIMER_PMW2, CTIMER_MAT_PWM_PERIOD_CHANNEL, CTIMER_MAT2_OUT, pwmPeriod, pulsePeriod, false);
	CTIMER_StartTimer(CTIMER_PMW2);
}


void update_pwm_duty_cycle(CTIMER_Type *timer, uint32_t ch0, uint32_t channel)
{
	uint8_t duty_cycle = 0;
	static uint8_t last_duty_cycle[CTIMER_MAT_MAX*2];
	uint8_t offset = (timer == CTIMER_PMW2)?1:0;
	if((ch0 > 1000) && (ch0 < 1048))
	{
		duty_cycle = 50;
	}
	else if((ch0 > 1048))
	{
		duty_cycle = 50 + (ch0 - 1048)*50/636 - 1;
	}
	else
	{
		duty_cycle = 50 - (1000 - ch0)*50/636 + 1;
	}

	if(last_duty_cycle[offset*4 + channel] == duty_cycle)
	{
		return;
	}

	CTIMER_StopTimer(timer);
	CTIMER_UpdatePwmDutycycle(timer, CTIMER_MAT_PWM_PERIOD_CHANNEL, channel, duty_cycle);
	CTIMER_StartTimer(timer);
	last_duty_cycle[offset*4 + channel] = duty_cycle;
}


void timer_init()
{
	ctimer_config_t config;
    /* Use FRO HF clock for some of the Ctimers */
    CLOCK_SetClkDiv(kCLOCK_DivCtimer4Clk, 1u);
    CLOCK_AttachClk(kFRO_HF_to_CTIMER4);

    BOARD_InitPins();
    BOARD_PowerMode_OD();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();

    PRINTF("CTimer match example to toggle the output. \r\n");
    PRINTF("This example uses interrupt to change the match period. \r\n");

    CTIMER_GetDefaultConfig(&config);
    //config.prescale = 1000;

    CTIMER_Init(CTIMER4, &config);

    /* Configuration 0 */
    matchConfig0.enableCounterReset = true;
    matchConfig0.enableCounterStop  = false;
    matchConfig0.matchValue         = CTIMER_CLK_FREQ / 2000;
    matchConfig0.outControl         = kCTIMER_Output_Toggle;
    matchConfig0.outPinInitState    = false;
    matchConfig0.enableInterrupt    = true;

    /* Configuration 1 */
    matchConfig1.enableCounterReset = true;
    matchConfig1.enableCounterStop  = false;
    matchConfig1.matchValue         = CTIMER_CLK_FREQ / 2000;
    matchConfig1.outControl         = kCTIMER_Output_Toggle;
    matchConfig1.outPinInitState    = true;
    matchConfig1.enableInterrupt    = true;

    CTIMER_RegisterCallBack(CTIMER4, &ctimer_callback_table[0], kCTIMER_MultipleCallback);
    CTIMER_SetupMatch(CTIMER4, CTIMER_MAT0_OUT, &matchConfig0);
    CTIMER_StartTimer(CTIMER4);
}

/*!
 * @brief Main function
 */
int main(void)
{
    /* attach FRO 12M to FLEXCOMM4 (debug console) */
    CLOCK_SetClkDiv(kCLOCK_DivFlexcom4Clk, 1u);
    CLOCK_AttachClk(BOARD_DEBUG_UART_CLK_ATTACH);

    /* Use FRO HF clock for some of the Ctimers */
	CLOCK_SetClkDiv(kCLOCK_DivCtimer0Clk, 1u);
	CLOCK_AttachClk(kFRO_HF_to_CTIMER0);

	CLOCK_SetClkDiv(kCLOCK_DivCtimer2Clk, 1u);
	CLOCK_AttachClk(kFRO_HF_to_CTIMER2);

    CLOCK_EnableClock(kCLOCK_Dma0);

    BOARD_InitBootPins();
    BOARD_PowerMode_OD();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();

    CLOCK_EnableClock(kCLOCK_Gpio3);
    LED_RED_INIT(LOGIC_LED_ON);
    LED_BLUE_INIT(LOGIC_LED_ON);
    LED_GREEN_INIT(LOGIC_LED_ON);

    timer_init();
    pwm1_init();
    pwm2_init();
    /* Initialize the LPUART. */
    /*
     * lpuartConfig.baudRate_Bps = 115200U;
     * lpuartConfig.parityMode = kLPUART_ParityDisabled;
     * lpuartConfig.stopBitCount = kLPUART_OneStopBit;
     * lpuartConfig.txFifoWatermark = 0;
     * lpuartConfig.rxFifoWatermark = 0;
     * lpuartConfig.enableTx = false;
     * lpuartConfig.enableRx = false;
     */
    LPUART_GetDefaultConfig(&lpuartConfig);
    lpuartConfig.baudRate_Bps = BOARD_DEBUG_UART_BAUDRATE;
    lpuartConfig.parityMode = kLPUART_ParityEven;
    lpuartConfig.enableTx     = true;
    lpuartConfig.enableRx     = true;

    LPUART_Init(DEMO_LPUART, &lpuartConfig, DEMO_LPUART_CLK_FREQ);

#if defined(FSL_FEATURE_SOC_DMAMUX_COUNT) && FSL_FEATURE_SOC_DMAMUX_COUNT
    /* Init DMAMUX */
    DMAMUX_Init(EXAMPLE_LPUART_DMAMUX_BASEADDR);
    /* Set channel for LPUART */
    DMAMUX_SetSource(EXAMPLE_LPUART_DMAMUX_BASEADDR, LPUART_TX_DMA_CHANNEL, LPUART_TX_DMA_REQUEST);
    DMAMUX_SetSource(EXAMPLE_LPUART_DMAMUX_BASEADDR, LPUART_RX_DMA_CHANNEL, LPUART_RX_DMA_REQUEST);
    DMAMUX_EnableChannel(EXAMPLE_LPUART_DMAMUX_BASEADDR, LPUART_TX_DMA_CHANNEL);
    DMAMUX_EnableChannel(EXAMPLE_LPUART_DMAMUX_BASEADDR, LPUART_RX_DMA_CHANNEL);
#endif
    /* Init the EDMA module */
#if (!defined(DEMO_EDMA_HAS_CHANNEL_CONFIG) || (defined(DEMO_EDMA_HAS_CHANNEL_CONFIG) && !DEMO_EDMA_HAS_CHANNEL_CONFIG))
    EDMA_GetDefaultConfig(&config);
#endif
    EDMA_Init(EXAMPLE_LPUART_DMA_BASEADDR, &config);
    EDMA_CreateHandle(&g_lpuartTxEdmaHandle, EXAMPLE_LPUART_DMA_BASEADDR, LPUART_TX_DMA_CHANNEL);
    EDMA_CreateHandle(&g_lpuartRxEdmaHandle, EXAMPLE_LPUART_DMA_BASEADDR, LPUART_RX_DMA_CHANNEL);
#if defined(FSL_FEATURE_EDMA_HAS_CHANNEL_MUX) && FSL_FEATURE_EDMA_HAS_CHANNEL_MUX
    EDMA_SetChannelMux(EXAMPLE_LPUART_DMA_BASEADDR, LPUART_TX_DMA_CHANNEL, DEMO_LPUART_TX_EDMA_CHANNEL);
    EDMA_SetChannelMux(EXAMPLE_LPUART_DMA_BASEADDR, LPUART_RX_DMA_CHANNEL, DEMO_LPUART_RX_EDMA_CHANNEL);
#endif
    /* Create LPUART DMA handle. */
    LPUART_TransferCreateHandleEDMA(DEMO_LPUART, &g_lpuartEdmaHandle, LPUART_UserCallback, NULL, &g_lpuartTxEdmaHandle,
                                    &g_lpuartRxEdmaHandle);

//    /* Send g_tipString out. */
//    xfer.data     = g_tipString;
//    xfer.dataSize = sizeof(g_tipString) - 1;
//    txOnGoing     = true;
//    LPUART_SendEDMA(DEMO_LPUART, &g_lpuartEdmaHandle, &xfer);
//
//    /* Wait send finished */
//    while (txOnGoing)
//    {
//    }

    /* Start to echo. */
    sendXfer.data        = g_txBuffer;
    sendXfer.dataSize    = 16;
    //uart_write(g_tipString, sizeof(g_tipString));
    receiveXfer.data     = sbus_rx_buffer;
    receiveXfer.dataSize = 18;

    //LPUART_ReceiveEDMA(DEMO_LPUART, &g_lpuartEdmaHandle, &receiveXfer);
    while (1)
    {
        /* If RX is idle and g_rxBuffer is empty, start to read data to g_rxBuffer. */
        if ((!rxOnGoing))
        {
            rxOnGoing = true;
        	LPUART_ReceiveEDMA(DEMO_LPUART, &g_lpuartEdmaHandle, &receiveXfer);
        }
        //print_remote_data();
        update_pwm_duty_cycle(CTIMER0, RC_Ctl.rc.ch0, CTIMER_MAT0_OUT);
        update_pwm_duty_cycle(CTIMER0, RC_Ctl.rc.ch1, CTIMER_MAT1_OUT);
        update_pwm_duty_cycle(CTIMER0, RC_Ctl.rc.ch3, CTIMER_MAT2_OUT);

        update_pwm_duty_cycle(CTIMER_PMW2, RC_Ctl.rc.ch2, CTIMER_MAT0_OUT);

    }
}
