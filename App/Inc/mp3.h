// ****************************************************************************
//     Filename: MP3.H
// Date Created: 5/19/2025
//
//     Comments: MP3 Voice module header for the Robertsonics OpenMP3 Player
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

#include "player.h"				// For our type definitions

#define VOICE_ERR_NOERROR		0
#define VOICE_ERR_BADINDEX		1
#define VOICE_ERR_BADOPEN		2

#define VOICE_STATE_AVAIL		0
#define VOICE_STATE_PLAYING		1
#define VOICE_STATE_PAUSED		2
#define VOICE_STATE_STOPPED		3

#define WAV_FORMAT_FIXED		0
#define WAV_FORMAT_FLOAT		1

#define MP3_BUFFER_SIZE			((2 * BYTES_PER_BLOCK) + (BYTES_PER_BLOCK / 2))

#define MP3_WAV_BUFFER_SIZE		((4 * MP3_FRAME_SIZE_IN_SAMPLES) + (MP3_FRAME_SIZE_IN_SAMPLES / 2))

#define UNITY_PITCH_INC			0x00010000

typedef struct {
	bool active;				// Active flag
	bool stopFlag;				// Stop on fade end flag
	bool upFlag;				// Fade up flag
	uint8_t shortFade;			// Short fade flag
	bool shortFadeDone;			// Short faded done flag	
	uint8_t targGainIdx;		// Target gain index
	uint32_t delta;				// Fader delta
	U32_UNION accum;			// Fader accumulator
} FADER_STRUCTURE;

// The following structure defines a voice.

#pragma pack(1)
typedef struct {
	
	uint8_t buff[MP3_BUFFER_SIZE];// Ouf voice mp3 buffer
	
	q15_t wavBuff[MP3_WAV_BUFFER_SIZE];	// Our voice wav buffer

	uint8_t state;					// Voice state
	uint8_t noteNum;				// MIDI Note number or 0xff if none
	bool loopFlag;
	bool lockFlag;					// Voice lock flag
	bool stopReqFlag;				// Stop request flag
	bool eofFlag;					// End of file flag
	
	FILE_SIZE size;					// File size in bytes
	uint32_t bytesSdRead;			// Number of bytes read from SD file
	uint32_t bytesFetched;			// Number of bytes fetched by decoder
	
	uint32_t framesPlayed;
		
	uint8_t currGainIdx;			// Current gain index
	q15_t currGain;					// Current linear gain
	uint16_t releaseMs;				// Release time in ms
		
	uint16_t track;					// Track number
	uint32_t time;					// Trigger timestamp
	
	uint16_t mp3InPtr;				// Mp3 buffer input pointer
	uint16_t mp3OutPtr;				// Mp3 buffer output pointer

	uint16_t wavInPtr;				// Wav buffer input pointer
	uint16_t wavOutPtr;				// Wav buffer output pointer
			
	FADER_STRUCTURE fader;			// Our fader
			
} MP3_VOICE_STRUCTURE;
#pragma pack()

// Function prototypes for this module

uint16_t mp3OpenFile(uint8_t v, uint16_t t, int16_t gainDb);
void mp3SetCurrentGain(uint8_t v, int16_t gain);
void mp3SetState(uint8_t v, uint8_t s);
uint8_t mp3GetState(uint8_t v);
void mp3MarkTime(uint8_t v);
void mp3Stop(uint8_t v);
void mp3StopNote(uint8_t note);

void mp3StartFader(uint8_t v, int16_t gainDb, uint16_t mTime, bool fStop);
void mp3StopFader(uint8_t v);
bool mp3ServiceFader(uint8_t v);

bool mp3CheckSdMp3Space(uint8_t v);
int16_t mp3ReadSdMp3Data(uint8_t v);
uint16_t mp3FetchMp3Data(uint8_t v, uint8_t *pDest, uint16_t reqBytes);

bool mp3CheckWavSpace(uint8_t v);
int16_t mp3DecodeWavData(uint8_t v);
bool mp3PutWavData(uint8_t v, q15_t *pSrc, uint16_t numFrames);

bool mp3GetAudio(uint8_t v, q15_t * pDest, uint16_t reqFrames);
