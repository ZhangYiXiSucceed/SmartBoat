#include "clock_config.h"
#include "pwm.h"

void pwm_motor_init()
{
    ctimer_config_t config;
    uint32_t srcClock_Hz;
    uint32_t timerClock;

    /* Use FRO HF clock for some of the Ctimers */
	CLOCK_SetClkDiv(kCLOCK_DivCtimer0Clk, 1u);
	CLOCK_AttachClk(kFRO_HF_to_CTIMER0);

    /* CTimer0 counter uses the AHB clock, some CTimer1 modules use the Aysnc clock */
    srcClock_Hz = CLOCK_GetCTimerClkFreq(0U);

    CTIMER_GetDefaultConfig(&config);
    timerClock = srcClock_Hz / (config.prescale + 1);

    CTIMER_Init(CTIMER_MOTOR, &config);

    /* Calculate PWM period match value */
    uint32_t pwmPeriod = (timerClock / CTIMER_MOTOR_FREQ) - 1U;

   /* Calculate pulse width match value */
    uint32_t pulsePeriod = (pwmPeriod + 1U) * (100 - 20) / 100;

    CTIMER_SetupPwmPeriod(CTIMER_MOTOR, CTIMER_MAT_PWM_PERIOD_CHANNEL, CTIMER_MAT0_OUT, pwmPeriod, pulsePeriod, false);
    CTIMER_SetupPwmPeriod(CTIMER_MOTOR, CTIMER_MAT_PWM_PERIOD_CHANNEL, CTIMER_MAT1_OUT, pwmPeriod, pulsePeriod, false);
    CTIMER_SetupPwmPeriod(CTIMER_MOTOR, CTIMER_MAT_PWM_PERIOD_CHANNEL, CTIMER_MAT2_OUT, pwmPeriod, pulsePeriod, false);
    CTIMER_StartTimer(CTIMER_MOTOR);
}


void pwm_servo_init()
{
	ctimer_config_t config;
	uint32_t srcClock_Hz;
	uint32_t timerClock;

	CLOCK_SetClkDiv(kCLOCK_DivCtimer2Clk, 1u);
	CLOCK_AttachClk(kFRO_HF_to_CTIMER2);

	/* CTimer0 counter uses the AHB clock, some CTimer1 modules use the Aysnc clock */
	srcClock_Hz = CLOCK_GetCTimerClkFreq(0U);

	CTIMER_GetDefaultConfig(&config);
	timerClock = srcClock_Hz / (config.prescale + 1);

	CTIMER_Init(CTIMER_SERVO, &config);

	/* Calculate PWM period match value */
	uint32_t pwmPeriod = (timerClock / CTIMER_SERVO_FREQ) - 1U;

   /* Calculate pulse width match value */
	uint32_t pulsePeriod = (pwmPeriod + 1U) * (100 - 20) / 100;

	CTIMER_SetupPwmPeriod(CTIMER_SERVO, CTIMER_MAT_PWM_PERIOD_CHANNEL, CTIMER_MAT0_OUT, pwmPeriod, pulsePeriod, false);
	CTIMER_SetupPwmPeriod(CTIMER_SERVO, CTIMER_MAT_PWM_PERIOD_CHANNEL, CTIMER_MAT1_OUT, pwmPeriod, pulsePeriod, false);
	CTIMER_SetupPwmPeriod(CTIMER_SERVO, CTIMER_MAT_PWM_PERIOD_CHANNEL, CTIMER_MAT2_OUT, pwmPeriod, pulsePeriod, false);
	CTIMER_StartTimer(CTIMER_SERVO);
}

void update_pwm_duty_cycle(CTIMER_Type *timer, uint32_t ch0, uint32_t channel)
{
	uint8_t duty_cycle = 0;
	static uint8_t last_duty_cycle[CTIMER_MAT_MAX*2];
	uint8_t offset = (timer == CTIMER_SERVO)?1:0;
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
