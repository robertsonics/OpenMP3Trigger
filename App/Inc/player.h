// ****************************************************************************
//     Filename: PLAYER.H
// Date Created: 5/19/2025
//
//     Comments: Header file for the Robertsonics OpenMP3 Player
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

#ifndef PLAYER_20250519
#define PLAYER_20250519

#define __WT_DEV_BOARD__

#include "main.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

// Define our version string here
//                      "123456789012"
#define VERSION_STRING	"OpenMP3 v0.1"

#define VERSION_STRING_LEN		12
#define MAX_STRING_LEN			32

#define MAX_NUM_MP3_VOICES		2

#define MIX_BUFF_FRAMES			128
#define MIX_BUFF_SAMPLES		(MIX_BUFF_FRAMES * 2)
#define AUDIO_BUFF_SAMPLES		(MIX_BUFF_SAMPLES * 2)

// Error code definitions

#define SYS_OK					0x00
#define SYS_NO_SDCARD			0x01
#define SYS_SDCARD_ERROR		0x02
#define SYS_FILESYS_ERROR		0x04
#define SYS_NOTRACKS_ERROR		0x08
#define SYS_FATAL_ERROR			0x10

// LED Blink pattern definitions

#define BLINK_OK				0
#define BLINK_ERR				1

#define LED_FLASH_PERIOD_LONG	1000		// Heartbeat flash period in msecs
#define LED_FLASH_PERIOD_SHORT	500			// Warning flash period in msecs
#define SD_CARD_CHECK_PERIOD	100			// Cycle to check for SD card install

// Helper type for dealing with endiness

typedef union {
	uint32_t u32;
	uint16_t u16[2];
	uint8_t u8[4];
} U32_UNION;

typedef union
{
	uint8_t	bSize[4];
	uint32_t	lSize;

} FILE_SIZE;

#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include "arm_math.h"
#include "bios.h"
#include "voice.h"
#include "mp3decode.h"
#include "mp3.h"
#include "track.h"
#include "dsp.h"
#include "ffdisk.h"
#include "console.h"

// ****************************************************************************
// Function prototypes
void sdTestInit(void);
void sdTestProcess(void);
void delayMs(uint32_t dlyTicks);
void doBlink(uint8_t p);
void doFlash(uint8_t repeat);
void rogueTrap(uint8_t code);
void mySysTick_Handler(void);
	
#endif
