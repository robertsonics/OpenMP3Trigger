// ****************************************************************************
//     Filename: BIOS.C
// Date Created: 5/19/2025
//
//     Comments: WAV Trigger hardware low level interface module
//
// Build Environment: Visual Studio Code
//                    STM32CubeMx
//
// COPYRIGHT (c) 2025, Robertsonics
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.
//
// ****************************************************************************

#include "main.h"
#include "player.h"
#include "stm32f4xx_hal_rcc.h"

void rogueTrap(uint8_t code);


// ****************************************************************************
// External variables

extern volatile uint32_t gMsTicks;			// 1ms global System tick
extern volatile uint16_t gNumWavTracks;		// Number of WAV tracks found on SD card
extern volatile uint8_t gSysFlags;			// System init error flags

extern SD_HandleTypeDef hsd;

extern uint8_t gSectorBuff[];


// ****************************************************************************
// Global variables

bool gLongSdReadFlag = false;
uint32_t gMaxSdReaduSecs = 0;
uint32_t gLastSdReaduSecs = 0;
volatile bool gMmcDoneFlag;

HAL_SD_CardCIDTypeDef pCID;
HAL_SD_CardCSDTypeDef pCSD;
HAL_SD_CardInfoTypeDef pCardInfo;

// Variables related to the serial console interface

char  gTxBuffer[TX_BUFFER_SIZE];				// Serial transmit buffer
uint16_t gTxInPtr = 0;							// Serial transmit buffer input pointer
uint16_t gTxOutPtr = 0;							// Serial transmit buffer output pointer
bool  gTxIpFlag = false;						// Serial transmit in-progress flag

char gRxBuffer[RX_BUFFER_SIZE];					// Serial receive buffer
uint16_t gRxInPtr = 0;							// Serial receive input pointer
uint16_t gRxOutPtr = 0;							// Serial receive output pointer


// ****************************************************************************
// biosSystemInit
// ****************************************************************************
bool biosSystemInit(void) {

	LED0_OFF;
	
	// Start the hi-res timer
	LL_TIM_EnableCounter(TIM2);
		
	return true;
}

// ****************************************************************************
// biosSDInit
// ****************************************************************************
bool biosSdInit(void) {

	HAL_SD_GetCardCID(&hsd, &pCID);
	HAL_SD_GetCardCSD(&hsd, &pCSD);
	HAL_SD_GetCardInfo(&hsd, &pCardInfo);
	return true;	
}

// ****************************************************************************
// biosIsSdCardInstalled
// *****************************************************************************
bool biosIsSdCardInstalled(void) {

#ifdef __WT_DEV_BOARD__
	if (HAL_GPIO_ReadPin(SD_CD_GPIO_Port, SD_CD_Pin) != 0)
		return false;
	return true;
#else
	if (HAL_GPIO_ReadPin(SD_CD_GPIO_Port, SD_CD_Pin) != 1)
		return false;
	return true;
#endif
}

// ****************************************************************************
// biosGetHiResTimer
// *****************************************************************************
uint32_t biosGetHiResTimer(void) {
	
uint32_t tim;
	
	tim = LL_TIM_GetCounter(TIM2);
	return tim;
}

// ****************************************************************************
// biosLED
// *****************************************************************************
void biosLED(int led, bool state) {
	
UNUSED(led);

	if (state) {
		LED0_ON;
	}
	else {
		LED0_OFF;
	}
}

// ****************************************************************************
// biosDebug
// *****************************************************************************
void biosDebug(bool state) {

UNUSED(state);

}

// ****************************************************************************
// biosSdReadSector
// *****************************************************************************
bool biosSdReadSector(uint8_t *pDst, uint32_t addr) {

	return (biosSdReadSectors(pDst, addr, 1));
}

// ****************************************************************************
// biosReadSdBlock
// *****************************************************************************
bool biosSdReadBlock(uint8_t *pDst, uint32_t addr) {

uint32_t tmp32;
bool fResult;

	tmp32 = LL_TIM_GetCounter(TIM2);
	fResult = biosSdReadSectors(pDst, addr, SD_SECTORS_PER_BLOCK);
	
	// Calculate how many microseconds this read took and update the max
	//  time accordingly. If we exceed our defined long read, set the 
	//  global flag to alert the user that it happened at least once
	gLastSdReaduSecs = LL_TIM_GetCounter(TIM2) - tmp32;
	if (gLastSdReaduSecs > gMaxSdReaduSecs) {
		gMaxSdReaduSecs = gLastSdReaduSecs;
		if (gMaxSdReaduSecs > MAX_SD_READ_USECS)
			gLongSdReadFlag = true;
	}
	return fResult;
}

// ****************************************************************************
// biosSdReadSectors
// *****************************************************************************
bool biosSdReadSectors(uint8_t *pDst, uint32_t addr, uint16_t nsecs) {

HAL_StatusTypeDef err;

	gMmcDoneFlag = false;
	//SCB_InvalidateDCache_by_Addr((void *)pDst, (nsecs * 512));
	err = HAL_SD_ReadBlocks_DMA(&hsd, pDst, addr, nsecs);
	while (!gMmcDoneFlag);
	if (err != HAL_OK) {
		rogueTrap(3);
		return false;
	}
	//SCB_InvalidateDCache_by_Addr((void *)pDst, (nsecs * 512));
	return true;
}
 
// ****************************************************************************
// biosSdWriteSectors
// *****************************************************************************
bool biosSdWriteSectors(uint8_t *pDst, uint32_t addr, uint16_t nsecs) {

HAL_StatusTypeDef err;

	gMmcDoneFlag = false;
	//SCB_InvalidateDCache_by_Addr((void *)pDst, (nsecs * 512));
	err = HAL_SD_WriteBlocks_DMA(&hsd, pDst, addr, nsecs);
	while (!gMmcDoneFlag);
	if (err != HAL_OK) {
		rogueTrap(3);
		return false;
	}
	//SCB_InvalidateDCache_by_Addr((void *)pDst, (nsecs * 512));
	return true;
}
	
// ****************************************************************************
// biosSerialInit
// ****************************************************************************
void biosSerialInit(void) {
	
	for (int i = 0; i < RX_BUFFER_SIZE; i++)
		gRxBuffer[i] = 0;
	for (int i = 0; i < TX_BUFFER_SIZE; i++)
		gTxBuffer[i] = 0;
	gRxInPtr = 0;
	gRxOutPtr = 0;
	gTxInPtr = 0;
	gTxOutPtr = 0;
	
	LL_USART_EnableIT_RXNE(USART1);
}

// ****************************************************************************
// biosStartSerialXmt
// ****************************************************************************
void biosStartSerialXmt(void) {
	
	if (!gTxIpFlag) {
		gTxIpFlag = true;
		LL_USART_TransmitData8(USART1, gTxBuffer[gTxOutPtr++]);
		if (gTxOutPtr >= TX_BUFFER_SIZE)
			gTxOutPtr = 0;
		LL_USART_EnableIT_TXE(USART1);
	}
}

// ****************************************************************************
// HAL_SD_RxCpltCallback
// *****************************************************************************
void HAL_SD_RxCpltCallback(SD_HandleTypeDef *hsd) {

UNUSED(hsd);

	gMmcDoneFlag = true;	
}

// ****************************************************************************
// HAL_SD_TxCpltCallback
// *****************************************************************************
void HAL_SD_TxCpltCallback(SD_HandleTypeDef *hsd) {
	
UNUSED(hsd);

	gMmcDoneFlag = true;	
}

// ****************************************************************************
// biosUSART1_IRQHandler
// *****************************************************************************
void biosUSART1_IRQHandler(void)
{
	
uint8_t rxData;
uint16_t tmp16;

	// If this is a receive character interrupt
	if (LL_USART_IsActiveFlag_RXNE(USART1)) {
		rxData = LL_USART_ReceiveData8(USART1);
		
		tmp16 = gRxInPtr;
		gRxBuffer[tmp16++] = rxData;
		if (tmp16 >= RX_BUFFER_SIZE) tmp16 = 0;
		if (tmp16 != gRxOutPtr)
			gRxInPtr = tmp16;
	}
	else if (LL_USART_IsActiveFlag_ORE(USART1)) {
		rxData = LL_USART_ReceiveData8(USART1);
		LL_USART_ClearFlag_ORE(USART1);
	}
	
	// If this is a transmitter interrupt
	if (LL_USART_IsActiveFlag_TXE(USART1)) {
		
		if (gTxOutPtr != gTxInPtr) {
			LL_USART_TransmitData8(USART1, gTxBuffer[gTxOutPtr++]);
			if (gTxOutPtr >= TX_BUFFER_SIZE)
				gTxOutPtr = 0;
		}
		else {
			LL_USART_DisableIT_TXE(USART1);
			gTxIpFlag = false;
		}
	}
}

