// ****************************************************************************
//     Filename: BIOS.H
// Date Created: 5/19/2025
//
//     Comments: WAV Trigger hardware low level interface header
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

#ifndef WT_BIOS_20250519
#define WT_BIOS_20250519

#include "stm32f4xx_hal.h"

#define LED_ON			true
#define LED_OFF 		false
	
#define LED0_ON		HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_RESET)
#define LED0_OFF	HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_SET)

#define MAX_SD_READ_USECS	20000

#define SD_SECTORS_PER_BLOCK	4
#define BYTES_PER_BLOCK			(SD_SECTORS_PER_BLOCK * 512)
#define SAMPLES_PER_BLOCK		(SD_SECTORS_PER_BLOCK * 256)

#define TX_BUFFER_SIZE			1024
#define RX_BUFFER_SIZE			256

// Public function prototypes for this module

bool biosSystemInit(void);

bool biosIsSdCardInstalled(void);
bool biosSdInit(void);
bool biosSdReadSector(uint8_t *dst, uint32_t s);
bool biosSdReadBlock(uint8_t *dst, uint32_t s);
bool biosSdReadSectors(uint8_t *pDst, uint32_t addr, uint16_t nsecs);
bool biosSdWriteSectors(uint8_t *pDst, uint32_t addr, uint16_t nsecs);

uint32_t biosGetHiResTimer(void);

void biosLED(int led, bool state);
void biosDebug(bool state);

void biosSerialInit(void);
void biosStartSerialXmt(void);

void biosUSART1_IRQHandler(void);

#endif
