// ****************************************************************************
//     Filename: CONSOLE.C
// Date Created: 5/19/2025
//
//     Comments: ASCII serial module for the Robertsonics OpenMP3 Player
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
// External variables

extern uint32_t gMsTicks;
extern uint32_t gMaxSdReaduSecs;

extern char gVersion[];

extern volatile uint8_t gSysFlags;
extern volatile bool gAudioPlaying;
extern volatile uint16_t gNumMp3Tracks;

extern char gTxBuffer[];							// Serial transmit buffer
extern uint16_t gTxInPtr;							// Serial transmit buffer input pointer
extern uint16_t gTxOutPtr;							// Serial transmit buffer output pointer
extern bool  gTxIpFlag;								// Serial transmit in-progress flag

extern char gRxBuffer[];							// Serial receive buffer
extern uint16_t gRxInPtr;							// Serial receive input pointer
extern uint16_t gRxOutPtr;							// Serial receive output pointer


// ****************************************************************************
// Global variables

volatile char conLine[MAX_CONSOLE_CMD_LEN];
volatile uint8_t conCount;
volatile uint8_t conPtr;

volatile char conCmd[MAX_CONSOLE_CMD_LEN];
volatile uint8_t conNumParams;
volatile int32_t conParam[MAX_CONSOLE_PARAMS];


//*****************************************************************************
// consoleInit
//*****************************************************************************
void consoleInit(void) {
	
	conCount = 0;
}

//*****************************************************************************
// consoleService
//*****************************************************************************
void consoleService(void) {

uint8_t u8Val;
	
	// Process any bytes in the serial receive buffer
	if (gRxInPtr != gRxOutPtr) {

		// Grab the next byte from the receive buffer
		u8Val = gRxBuffer[gRxOutPtr++];
		if (gRxOutPtr >= RX_BUFFER_SIZE) gRxOutPtr = 0;				
		
		// Add this byte to the command line
		conLine[conCount++] = u8Val;
		if (conCount >= MAX_CONSOLE_CMD_LEN)
			conCount--;
		
		// Check for carriage return
		if (u8Val == 0x0d) {			
			consoleDoCommand();
			conCount = 0;
		}
	}
}

//*****************************************************************************
// consoleDoCommand
//*****************************************************************************
void consoleDoCommand(void) {
		
	if (consoleParseLine()) {
				
		// ==============================================
		// stat
		// ==============================================
		if (strcmp((const char *)conCmd, "stat") == 0)
			consoleSignOn();
/*
		// ==============================================
		// v
		// ==============================================
		else if (strcmp((const char *)conCmd, "v") == 0) {
			consoleSendString("Active voices: ");
			consoleSendInt32(voiceCheck());
			consoleNewLine(1);
		}
		
		// ==============================================
		// sd <0>
		// ==============================================
		else if (strcmp((const char *)conCmd, "sd") == 0) {
			if (conNumParams == 0) {		
				consoleSendString("Max microSD block read = ");
				consoleSendInt32(gMaxSdReaduSecs);
				consoleSendString(" usecs\n\r");
			}
			else if ((conNumParams == 1) && (conParam[0] == 0)) {
				consoleSendString("microSD block read time reset\n\r");
				gMaxSdReaduSecs = 0;
			}
		}

		// ==============================================
		// play t, <gain, bal, attack, pitch, loop, lock>
		// ==============================================
		else if (strcmp((const char *)conCmd, "play") == 0) {
			
			if (conNumParams < 1) {
				consoleSyntaxErr();
				return;
			}
			if (conParam[0] < MAX_NUM_TRACKS) {
				pNa.function = MIDI_ACTION_TRIG1;
				pNa.trackIndex = conParam[0];
				pNa.minGainDb = DEFAULT_MIN_VEL_GAINDB;
				pNa.maxGainDb = 0;
				pNa.balance = 64;
				pNa.pitchOffset = 0;
				pNa.attack = 0;
				pNa.looping = false;
				pNa.locked = false;						
			}
			if (conNumParams >= 2) {
				if ((conParam[1] >= MIN_GAIN_DB) && (conParam[1] < MAX_GAIN_DB))
					pNa.maxGainDb = conParam[1];
			}
			if (conNumParams >= 3) {
				if (conParam[2] <= 127)
					pNa.balance = conParam[2];
			}
			if (conNumParams >= 4) {
				if ((conParam[3] > 0) & (conParam[3] <= 4000))
					pNa.attack = conParam[3];
			}					
			if (conNumParams >= 5) {
				if ((conParam[4] >= -700) & (conParam[4] <= 700))
					pNa.pitchOffset = conParam[4];
			}					
			if (conNumParams >= 6) {
				if (conParam[5] > 0)
					pNa.looping = true;
			}
			if (conNumParams >= 7) {
				if (conParam[6] > 0)
					pNa.locked = true;
			}				
			voicePlayNote(0xff, 0x7f, &pNa, 0);		
		}
		
		// ==============================================
		// stop t, <release>
		// ==============================================
		else if (strcmp((const char *)conCmd, "stop") == 0) {
			
			if (conNumParams < 1) {
				voiceStopAll();
				return;
			}
			if (conParam[0] < MAX_NUM_TRACKS) {
				pNa.function = MIDI_ACTION_STOP;
				pNa.trackIndex = conParam[0];
			}
			if (conNumParams >= 2) {
				if ((conParam[1] > 0) && (conParam[1] < 10000))
					pNa.release = conParam[1];
			}
			voiceStopTrack(&pNa);
		}
			
		// ==============================================
		// load p
		// ==============================================
		else if (strcmp((const char *)conCmd, "load") == 0) {		

			if (conNumParams < 1) {
				consoleSyntaxErr();
				return;
			}
			if (conParam[0] <= 9999) {
				storeReadPreset(conParam[0], true);
			}
		}
		
		// ==============================================
		// gain g
		// ==============================================
		else if (strcmp((const char *)conCmd, "gain") == 0) {		

			if (conNumParams < 1) {
				consoleSyntaxErr();
				return;
			}
			if ((conParam[0] >= MIN_GAIN_DB) && (conParam[0] <= MAX_GAIN_DB)) {
				audioSetGainDb(conParam[0]);
			}
		}
*/		
		// ==============================================
		// help
		// ==============================================
		else if (strcmp((const char *)conCmd, "help") == 0) {

			consoleNewLine(1);
			consoleSendString("Function       Command  Parameters <optional>\n\r");
			consoleSendString("========       =======  =====================\n\r");
			consoleSendString("Status         stat     none\n\r");
			consoleSendString("Play track     play     trackNum<, gainDb, bal, attackMs, cents, loop, lock>\n\r");
			consoleSendString("Stop track     stop     trackNum<, releaseMs>\n\r");
			consoleSendString("Stop all       stop     none\n\r");
			consoleSendString("Output gain    gain     dB (-70 to 0)\n\r");
			consoleSendString("Active voices  v        none\n\r");
			consoleNewLine(1);
		}
	}
	else {
	}
}

//*****************************************************************************
// consoleParseLine
//*****************************************************************************
bool consoleParseLine(void) {

int n = 1;
int32_t p;
	
	conPtr = 0;
	conNumParams = 0;
	for (int i = 0; i < MAX_CONSOLE_PARAMS; i++)
		conParam[i] = 0;
	
	// Parse the command string
	if (consoleGetCommand((char *)conCmd) == 0)
		return false;
	
	// Parse any included command line parameters
	while ((n > 0) && (conPtr < conCount) && (conNumParams < MAX_CONSOLE_PARAMS)){
		n = consoleGetValue((int32_t *)&p);
		if (n != 0)
			conParam[conNumParams++] = p;
	}	
	return true;
}

//*****************************************************************************
// consoleGetCommand
//*****************************************************************************
int consoleGetCommand(char * cmdString) {

int n = 0;
char c;
char * dstPtr;
	
	dstPtr = cmdString;
	while (conPtr < conCount) {
		
		c = conLine[conPtr];
		
		// Ignore initial white space
		if ((n == 0) && ((c == ' ') || (c == 0x09))) {
			conPtr++;
		}
		// Abort on white space or delimeter, and insert a null to
		//  terminate string
		else if ((c == 0x20) || (c == ',') || (c == 0x0d)) {
			*dstPtr = 0;
			return n;
		}		
		// Save allowable lower case letters
		else {
			c |= 0x20;
			if ((c >= 0x61) && (c <= 0x7a)) {
				*dstPtr++ = c;
				conPtr++;
				n++;
			}
			else
				return 0;
		}
	}
	return 0;
}

//*****************************************************************************
// consoleGetValue
//*****************************************************************************
int consoleGetValue(int32_t * val) {

int n = 0;
char c;
int32_t iSign = 1;
int32_t iVal = 0;
	
	while (conPtr < conCount) {
		
		c = conLine[conPtr];
		
		// Ignore initial white space
		if ((n == 0) && ((c == ' ') || (c == 0x09))) {
			conPtr++;
		}
		// Check for negative value
		else if ((n == 0) && (c == '-')) {
			iSign = -1;
			conPtr++;
			n++;	
		}		
		// Check for valid number
		else if ((c >= '0') && (c <= '9')) {
			iVal = (iVal * 10) + (c - 0x30);
			conPtr++;
			n++;
		}				
		// Check for delimiter
		else if (c == ',') {
			*val = (iVal * iSign);
			conPtr++;
			return n;
		}
		// Check for line feed
		else if (c == 0x0d) {
			*val = (iVal * iSign);
			conPtr++;
			return n;
		}		
		// Else we've got an error on our hands
		else {
			*val = 0;
			return 0;
		}
	}
	*val = 0;
	return 0;
}

//*****************************************************************************
// consoleSyntaxErr
//*****************************************************************************
void consoleSyntaxErr(void) {
	consoleSendString("Syntax error!\n\r");
}

//*****************************************************************************
// consoleSignOn
//*****************************************************************************
void consoleSignOn(void) {
	
float fTemp;
uint32_t gCardMB;	

	consoleNewLine(1);
	consoleSendString("Robertsonics ");
	consoleSendBytes(gVersion, VERSION_STRING_LEN);
	consoleSendString(" (c)2024");
	consoleNewLine(1);
	
	if (gSysFlags & SYS_NO_SDCARD)
		consoleSendString("microSD card not detected");
	else if (gSysFlags & SYS_FILESYS_ERROR)
		consoleSendString("File system error");
/*
	else {
		consoleSendString("System Info:\n\r");
		if (isFat32())
			consoleSendString("  Volume format = FAT32\n\r");
		else
			consoleSendString("  Volume format = FAT16\n\r");
		
		fTemp = ((float)getTotalSectors() * 512.0f) / 1048576.0f;
		gCardMB = (uint32_t)(fTemp + 0.5f);	

		consoleSendString("  Volume capacity = ");
		consoleSendInt32(gCardMB);
		consoleSendString(" MB\n\r");		
		consoleSendString("  Usable data sectors = ");
		consoleSendInt32(getTotalSectors());
		consoleSendString("\n\r");
		consoleSendString("  First data sector = ");
		consoleSendInt32(getFirstSector());
		consoleSendString("\n\r");
		consoleSendString("  Sectors per cluster = ");
		consoleSendInt32(getSectorsPerCluster());
		consoleSendString("\n\r");
		consoleSendString("  Sectors per read block = ");
		consoleSendInt32(SD_SECTORS_PER_BLOCK);
		consoleSendString("\n\r");
		consoleSendString("  Number of tracks found = ");
		consoleSendInt32(gNumWavTracks);
		consoleNewLine(1);
		consoleSendString("  Available voices = ");
		consoleSendInt32(MAX_NUM_MP3_VOICES);
		consoleNewLine(1);
		consoleSendString("  Audio buffer size = ");
		consoleSendInt32(MIX_BUFF_FRAMES);
		consoleSendString(" sample frames\n\r");
	}
	if (gSysFlags & SYS_NOTRACKS_ERROR)
		consoleSendString("No wav files found");
*/
	consoleNewLine(1);
}

//*****************************************************************************
// consoleNewLine
//*****************************************************************************
bool consoleNewLine(int nl) {

uint16_t iPtr;
uint8_t i;
	
	if (nl == 0)
		return false;
	
	iPtr = gTxInPtr;
	for (i = 0; i < nl; i++) {
		gTxBuffer[iPtr] = 0x0d;
		if (++iPtr >= TX_BUFFER_SIZE)
			iPtr = 0;
		if (iPtr == gTxOutPtr)
			return false;
		gTxBuffer[iPtr] = 0x0a;
		if (++iPtr >= TX_BUFFER_SIZE)
			iPtr = 0;
		if (iPtr == gTxOutPtr)
			return false;
	}
	gTxInPtr = iPtr;
	biosStartSerialXmt();
	return true;
}

//*****************************************************************************
// consoleSendString
//*****************************************************************************
bool consoleSendString(char * pMsg) {
	
uint16_t iPtr;
uint8_t i;
uint8_t len;
	
	if ((len = strlen(pMsg)) == 0)
		return false;
	
	iPtr = gTxInPtr;
	for (i = 0; i < len; i++) {
		gTxBuffer[iPtr] = pMsg[i];
		if (++iPtr >= TX_BUFFER_SIZE)
			iPtr = 0;
		if (iPtr == gTxOutPtr)
			return false;
	}
	gTxInPtr = iPtr;
	biosStartSerialXmt();
	return true;
}

//*****************************************************************************
// consoleSendBytes
//*****************************************************************************
bool consoleSendBytes(char * pMsg, uint8_t len) {
	
uint16_t iPtr;
uint8_t i;
	
	if (len == 0)
		return false;
	
	iPtr = gTxInPtr;
	for (i = 0; i < len; i++) {
		gTxBuffer[iPtr] = pMsg[i];
		if (++iPtr >= TX_BUFFER_SIZE)
			iPtr = 0;
		if (iPtr == gTxOutPtr)
			return false;
	}
	gTxInPtr = iPtr;
	biosStartSerialXmt();
	return true;
}

//*****************************************************************************
// consoleSendInt32
//*****************************************************************************
bool consoleSendInt32(uint32_t n) {
	
char sBuf[16];
uint32_t val;
uint32_t div = 1000000000;
uint8_t i;
uint8_t j = 0;
uint8_t k = 0;

	if (n > 999999999) return false;
	val = n;
	
	for (i = 0; i < 10; i++) {
		sBuf[j++] = 0x30 + (uint8_t)(val / div);
		val = val % div;
		div = div / 10;
		if ((k == 0) || (k == 3) || (k == 6)) {
			sBuf[j++] = 0x2c;
		}
		k++;		
	}
	i = 0;
	while (((sBuf[i] == 0x30) || (sBuf[i] == 0x2c)) && (i < 13))
		i++;
	sBuf[13] = 0;
	return consoleSendString(&sBuf[i]);
}


