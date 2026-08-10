/*
 * rc.c
 *
 *  Created on: 2026年8月10日
 *      Author: 13952
 */
#include "fsl_lpuart_edma.h"
#include "fsl_debug_console.h"
#include "board.h"
#include "rc.h"
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
AT_NONCACHEABLE_SECTION_INIT(uint8_t g_rxBuffer[ECHO_BUFFER_LENGTH]) = {0};
AT_NONCACHEABLE_SECTION_INIT(uint8_t sbus_rx_buffer[ECHO_BUFFER_LENGTH]) = {0};
volatile bool rxOnGoing                                              = false;
#if (defined(DEMO_EDMA_HAS_CHANNEL_CONFIG) && DEMO_EDMA_HAS_CHANNEL_CONFIG)
extern edma_config_t config;
#else
edma_config_t config;
#endif

RC_Ctl_t RC_Ctl;
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
lpuart_transfer_t receiveXfer;
uint32_t count;
void LPUART_UserCallback(LPUART_Type *base, lpuart_edma_handle_t *handle, status_t status, void *userData)
{
    userData = userData;

    if (kStatus_LPUART_RxIdle == status)
    {
        rxOnGoing     = false;
        data_process();

         LPUART_TransferGetReceiveCountEDMA(DEMO_LPUART, &g_lpuartEdmaHandle,&count);
        //LPUART_TransferAbortReceiveEDMA(DEMO_LPUART, &g_lpuartEdmaHandle);
        //LPUART_ReceiveEDMA(DEMO_LPUART, &g_lpuartEdmaHandle, &receiveXfer);

    }
}

void print_remote_data()
{
	/* 读取到的数据通过串口5错位输出 */
	PRINTF("%4d %4d %4d %4d %4d %4d\r\n", RC_Ctl.rc.ch0, RC_Ctl.rc.ch1, RC_Ctl.rc.ch2, RC_Ctl.rc.ch3, \
				RC_Ctl.rc.s1,RC_Ctl.rc.s2);
}

void rc_uart_init()
{
	CLOCK_EnableClock(kCLOCK_Dma0);

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

	receiveXfer.data     = sbus_rx_buffer;
	receiveXfer.dataSize = RC_DATA_RECIVE_LEN;
}

void rc_dma_data_recive()
{
	/* If RX is idle and g_rxBuffer is empty, start to read data to g_rxBuffer. */
	if ((!rxOnGoing))
	{
		rxOnGoing = true;
		LPUART_ReceiveEDMA(DEMO_LPUART, &g_lpuartEdmaHandle, &receiveXfer);
	}
}

