// ****************************************************************************
//     Filename: MP3DECODE.C
// Date Created: 5/19/2025
//
//     Comments: Decoder interface module for the Robertsonics OpenMP3 Player
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
#include "spiritMP3Dec.h"


// ****************************************************************************
// External variables

extern MP3_VOICE_STRUCTURE mp3[];		// Our voice structure array


// ****************************************************************************
// Global variables

uint8_t gMP3VoiceNum[MAX_NUM_MP3_VOICES];

bool mp3StreamErrorFlag;

TSpiritMP3Info mp3Info;

// Decoder structure array - one for each voice
TSpiritMP3Decoder g_MP3Decoder[MAX_NUM_MP3_VOICES];

// One buffer for decoder output since they are processed serially
q15_t gDecodeOutputBuffer[MP3_FRAME_SIZE_IN_SAMPLES] __attribute__((aligned (32)));


//*****************************************************************************
// mp3DecodeCallback
//*****************************************************************************
unsigned int mp3DecodeCallback(void * pMP3CompressedData,
							 unsigned int nMP3DataSizeInChars,
							 void * token) {
uint8_t v;
uint16_t numBytes;

	// Grab the voice number for this stream
	v = *(uint8_t *)token;
								 
	// Make sure we have a full buffer of SD MP3 data
	while (mp3CheckSdMp3Space(v)) {
		mp3ReadSdMp3Data(v);
	}
	
	// Copy the request number of MP3 bytes to the decoders destination buffer
	numBytes = mp3FetchMp3Data(v, (uint8_t *)pMP3CompressedData, nMP3DataSizeInChars);
	
	return numBytes;
}

//*****************************************************************************
// mp3DecodeInit
//*****************************************************************************
void mp3DecodeInit(void) {
	
uint8_t v;
	
	mp3StreamErrorFlag = false;
			
	for (v = 0; v < MAX_NUM_MP3_VOICES; v++) {
		gMP3VoiceNum[v] = v;
		SpiritMP3DecoderInit( &g_MP3Decoder[v], mp3DecodeCallback, NULL, &gMP3VoiceNum[v]);
	}
}

//*****************************************************************************
// mp3DecodeNewData
//*****************************************************************************
int16_t mp3DecodeNewData(uint8_t v) {
	
uint16_t numFrames;
	
	//DEBUG0_ON;
	numFrames = SpiritMP3Decode(&g_MP3Decoder[v],
					(short *)gDecodeOutputBuffer,
					MP3_FRAME_SIZE_IN_FRAMES,
					&mp3Info);
					
	if (mp3Info.nSamplesLeftInFrame == 0) {
		if (mp3Info.IsGoodStream == 0) {
			mp3StreamErrorFlag = true;
		}
	}
			
	mp3PutWavData(v, (q15_t *)&gDecodeOutputBuffer, numFrames);
	
	//DEBUG0_OFF;
	return numFrames;
}

	
