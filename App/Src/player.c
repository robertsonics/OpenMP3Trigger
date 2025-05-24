// ****************************************************************************
//     Filename: PLAYER.C
// Date Created: 5/19/2025
//
//     Comments: Top level module for the Robertsonics OpenMP3 Player
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

#include "player.h"


// ****************************************************************************
// Externals

extern uint8_t gSdBuff[];					// Our SD read buffer

extern MP3_VOICE_STRUCTURE mp3[];	// Our mp3 structure array


// ****************************************************************************
// Globals

volatile bool gCardInstalledFlag = false;	// MicroSD card installed flag
volatile uint8_t gSysFlags = 0;				// System init error flags
volatile uint8_t gNumMP3Voices;

volatile uint16_t gNumMp3Tracks = 0;		// Number of MP3 tracks found on SD card
volatile uint32_t gMsTicks = 0;				// our global system tick variable
volatile uint16_t gLED0_timeout = 0;		// LED timeout counter
volatile uint32_t lastHeartBeatTicks = 0;
volatile uint32_t lastSdCardCheckTicks = 0;

char gVersion[] = {VERSION_STRING};			// Our version string
char gNewLine[] = {0x0d, 0x0a, 0x00};		// New line character string


// ****************************************************************************
// sdTestInit
// ****************************************************************************
void sdTestInit(void) {
		
	// Initialize the low level hardware
	biosSystemInit();
	biosSerialInit();
	biosSdInit();
	
	// Check for installed microSD card
	if (biosIsSdCardInstalled()) {
		gCardInstalledFlag = true;
	}
	else {
		gCardInstalledFlag = false;
		gSysFlags |= SYS_NO_SDCARD;
	}

	// Initialize our MP3 voices
	for (int v = 0; v < MAX_NUM_MP3_VOICES; v++) {
		memset((uint8_t *)&mp3[v], 0, sizeof(MP3_VOICE_STRUCTURE));
		mp3[v].state = VOICE_STATE_AVAIL;
		mp3[v].fader.active = false;
	}
	gNumMP3Voices = MAX_NUM_MP3_VOICES;
	mp3DecodeInit();

	// Initialize our tracks
	if (trackInit(&gNumMp3Tracks))
		gSysFlags |= SYS_FILESYS_ERROR;

	// Initialize the ASCII serial console interface
	consoleInit();

	// Blink appropriately
	if (gSysFlags == 0)
		doBlink(BLINK_OK);
	else
		doBlink(BLINK_ERR);
		
	lastHeartBeatTicks = gMsTicks;
}


// ****************************************************************************
// sdTestProcess
// ****************************************************************************
void sdTestProcess(void)
{

	// ================== MAIN LOOP TASK 1 ===================
	// Service the ASCII serial console
	consoleService();

	// ================== MAIN LOOP TASK 2 ===================
	// Service the heartbeat LED
	if ((gMsTicks - lastHeartBeatTicks) > LED_FLASH_PERIOD_LONG) {
		lastHeartBeatTicks = gMsTicks;
		gLED0_timeout = 25;
		biosLED(0, true);
	}

	// ================== MAIN LOOP TASK 3 ===================
	// Check to make sure microSD card installed
	if ((gMsTicks - lastSdCardCheckTicks) > SD_CARD_CHECK_PERIOD) {
		lastSdCardCheckTicks = gMsTicks;
		if (gCardInstalledFlag) {
			if (biosIsSdCardInstalled() == false) {
				gCardInstalledFlag = false;
				gSysFlags |= SYS_NO_SDCARD;
			}
		}
	}

}

//*****************************************************************************
// Delay
//*****************************************************************************
void delayMs(uint32_t dlyTicks) {                                              

uint32_t curTicks;

	curTicks = gMsTicks;
	while ((gMsTicks - curTicks) < dlyTicks);
}

// *****************************************************************************
// doBlink
// *****************************************************************************
void doBlink(uint8_t p) {

uint8_t i;
	
	switch (p) {
		case BLINK_OK:
			for (i = 0; i < 3; i++) {
				biosLED(1, LED_ON);
				delayMs(125);
				biosLED(1, LED_OFF);
				delayMs(150);
			}
		break;
		case BLINK_ERR:
			biosLED(1, LED_ON);
			delayMs(2000);
			biosLED(1, LED_OFF);
			doFlash(1);
			delayMs(1000);
		break;
		default:
		break;
	}
}

// *****************************************************************************
// doFlash
// *****************************************************************************
void doFlash(uint8_t repeat) {
	
int r;
	
	for (r = 0; r < repeat; r++) {
		delayMs(500);
		biosLED(1, LED_ON);
		delayMs(250);
		biosLED(1, LED_OFF);
	}
}

// *****************************************************************************
// rogueTrap
// *****************************************************************************
void rogueTrap(uint8_t p)
{

UNUSED(p);

	while(1) {
		biosLED(0, LED_ON);
		delayMs(150);
		biosLED(0, LED_OFF);
		delayMs(250);
	}
}

//*****************************************************************************
// MyTick_Handler - Our 1ms Interrupt Routine
//*****************************************************************************
void mySysTick_Handler( void )
{
	gMsTicks++;
	
	// Check to see if we're timing out any of the LEDs and turn if off if time.
	if (gLED0_timeout > 0) {
		if (--gLED0_timeout == 0) {
			biosLED(0, LED_OFF);
		}
	}
}

