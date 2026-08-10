/*
 * timer.c
 *
 *  Created on: 2026年8月10日
 *      Author: 13952
 */
#include "timer.h"
#include "board.h"

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
void ctimer_match0_callback(uint32_t flags);
void ctimer_match1_callback(uint32_t flags);

/* Array of function pointers for callback for each channel */
ctimer_callback_t ctimer_callback_table[] = {
    ctimer_match0_callback, NULL, NULL, NULL, NULL, NULL, NULL, NULL};

uint32_t freq_count_g = 0;
uint8_t freq_1Hz_flag_g;
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
    freq_count_g ++ ;
    if(freq_count_g >= 1000)
    {
    	freq_count_g = 0;
    	freq_1Hz_flag_g = 1;
    }
}

void timer_init()
{
	/*******************************************************************************
	 * Variables
	 ******************************************************************************/
	/* Match Configuration for Channel 0 */
	ctimer_match_config_t matchConfig0;
	/* Match Configuration for Channel 1 */
	ctimer_match_config_t matchConfig1;

	ctimer_config_t config;
    /* Use FRO HF clock for some of the Ctimers */
    CLOCK_SetClkDiv(kCLOCK_DivCtimer4Clk, 1u);
    CLOCK_AttachClk(kFRO_HF_to_CTIMER4);

    CTIMER_GetDefaultConfig(&config);
    //config.prescale = 1000;

    CTIMER_Init(CTIMER_THREAD, &config);

    /* Configuration 0 */
    matchConfig0.enableCounterReset = true;
    matchConfig0.enableCounterStop  = false;
    matchConfig0.matchValue         = CTIMER_CLK_FREQ /  CTIMER_THREAD_FREQ;
    matchConfig0.outControl         = kCTIMER_Output_Toggle;
    matchConfig0.outPinInitState    = false;
    matchConfig0.enableInterrupt    = true;

    /* Configuration 1 */
    matchConfig1.enableCounterReset = true;
    matchConfig1.enableCounterStop  = false;
    matchConfig1.matchValue         = CTIMER_CLK_FREQ /  CTIMER_THREAD_FREQ;
    matchConfig1.outControl         = kCTIMER_Output_Toggle;
    matchConfig1.outPinInitState    = true;
    matchConfig1.enableInterrupt    = true;

    CTIMER_RegisterCallBack(CTIMER_THREAD, &ctimer_callback_table[0], kCTIMER_MultipleCallback);
    CTIMER_SetupMatch(CTIMER_THREAD, CTIMER_MAT0_OUT, &matchConfig0);
    CTIMER_StartTimer(CTIMER4);
}
