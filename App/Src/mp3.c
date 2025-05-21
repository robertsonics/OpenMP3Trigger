// ****************************************************************************
//     Filename: MP3.C
// Date Created: 5/19/2025
//
//     Comments: MP3 Voice module for the Robertsonics OpenMP3 Player
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

extern uint32_t gMsTicks;			// Our 1ms global system tick

extern uint8_t gVoiceSdBuff[];		// Our working SD buffer
extern MP3_VOICE_STRUCTURE mp3[];	// Our MP3 voice structure array

extern q15_t gain_tble[];			// Our gain table

//extern TRACK_STRUCTURE track[];	// Our track structure array

extern uint8_t gNumMP3Voices;		//


// ****************************************************************************
// Global variables

q15_t gMP3VoiceBuff[(MIX_BUFF_SAMPLES * 2) + 4] __attribute__((aligned (4)));


//*****************************************************************************
// mp3Stop
//*****************************************************************************
void mp3Stop(uint8_t v) {	

	if (mp3[v].state == VOICE_STATE_PLAYING)
		mp3[v].stopReqFlag = true;
	else
		mp3[v].state = VOICE_STATE_AVAIL;	
}


//*****************************************************************************
// mp3StopNote
//*****************************************************************************
void mp3StopNote(uint8_t note) {
	
uint8_t v;

	for (v = 0; v < gNumMP3Voices; v++) {
		if ((mp3[v].state == VOICE_STATE_PLAYING) && (mp3[v].noteNum == note)) {
			if (mp3[v].releaseMs == 0)
				mp3[v].stopReqFlag = true;
			else 
				mp3StartFader(v, -80, mp3[v].releaseMs, true);
		}
	}
}


//*****************************************************************************
// mp3SetState
//*****************************************************************************
void mp3SetState(uint8_t v, uint8_t s) {
	
	mp3[v].state = s;
}


//*****************************************************************************
// mp3GetState
//*****************************************************************************
uint8_t mp3GetState(uint8_t v) {
	
	return mp3[v].state;
}


//*****************************************************************************
// mp3SetCurrentGain
//*****************************************************************************
void mp3SetCurrentGain(uint8_t v, int16_t gain) {
	
	// If our fader is active, stop it
	mp3[v].fader.active = false;
	mp3[v].currGainIdx = dBtoIndex(gain);
	mp3[v].currGain = gain_tble[mp3[v].currGainIdx];
}


//*****************************************************************************
// mp3MarkTime
//*****************************************************************************
void mp3MarkTime(uint8_t v) {
	
	mp3[v].time = gMsTicks;
}


//*****************************************************************************
// mp3StartFader
//*****************************************************************************
void mp3StartFader(uint8_t v, int16_t gain, uint16_t mTime, bool fStop) {

uint32_t tmp32;

	// Handle times < 3m separately
	if (mTime < 3) {
		mp3[v].fader.shortFade = mTime;
		mp3[v].fader.targGainIdx = dBtoIndex(gain);
		mp3[v].fader.stopFlag = fStop;
		mp3[v].fader.shortFadeDone = false;
		mp3[v].fader.active = true;
		return;
	}
	
	// Calculate the delta gain index for the fade. Return now
	//  if we're already there
	mp3[v].fader.targGainIdx = dBtoIndex(gain);
	if (mp3[v].fader.targGainIdx == mp3[v].currGainIdx)
		return;

	// Determine if up or down and calculate the delta
	if (mp3[v].fader.targGainIdx > mp3[v].currGainIdx) {
		mp3[v].fader.upFlag = true;
		tmp32 = mp3[v].fader.targGainIdx - mp3[v].currGainIdx;
	}
	else {
		mp3[v].fader.upFlag = false;
		tmp32 = mp3[v].currGainIdx - mp3[v].fader.targGainIdx;
	}
	mp3[v].fader.delta = tmp32 << 16;
	mp3[v].fader.delta = (mp3[v].fader.delta * 3) / mTime;
	
	// Preset the accumulator and start the fader.
	mp3[v].fader.accum.u32 = mp3[v].currGainIdx;
	mp3[v].fader.accum.u32 <<= 16;
	mp3[v].fader.stopFlag = fStop;
	mp3[v].fader.active = true;
}


//*****************************************************************************
// mp3StopFader
//*****************************************************************************
void mp3StopFader(uint8_t v) {
	
	mp3[v].fader.active = false;
}


//*****************************************************************************
// mp3ServiceFader
//*****************************************************************************
// This routine is called for active voices once per audio buffer interrupt,
//  which is 128 samples (256 stereo) / 44100 samples/sec = 2.9 msecs.
//  Returns TRUE when fade is complete and stopRequest is TRUE.

bool mp3ServiceFader(uint8_t v)
{	
	// If the fader is active
	if (mp3[v].fader.active) {

		// If doing a short fade
		if (mp3[v].fader.shortFade) {
			if (mp3[v].fader.shortFadeDone) {
				mp3[v].fader.active = false;
				if (mp3[v].fader.stopFlag)
					return true;
				else
					return false;
			}
			else {
				mp3[v].currGainIdx = mp3[v].fader.targGainIdx;
				return false;
			}
		}
		
		// Fading up
		if (mp3[v].fader.upFlag) {
		
			// If we're at the target, deactivate the fader
			if (mp3[v].currGainIdx >= mp3[v].fader.targGainIdx) {
				mp3[v].fader.active = false;
				if (mp3[v].fader.stopFlag)
					return true;
				else
					return false;
			}
			mp3[v].fader.accum.u32 += mp3[v].fader.delta;
			
			if (mp3[v].fader.accum.u8[2] <= mp3[v].fader.targGainIdx)
				mp3[v].currGainIdx = mp3[v].fader.accum.u8[2];
			else
				mp3[v].currGainIdx = mp3[v].fader.targGainIdx;	
		}
		
		// Fading down
		else {

			// If we're at the target, deactivate the fader
			if (mp3[v].currGainIdx <= mp3[v].fader.targGainIdx) {
				mp3[v].fader.active = false;
				if (mp3[v].fader.stopFlag)
					return true;
				else
					return false;
			}
			if (mp3[v].fader.accum.u32 >= mp3[v].fader.delta)
				mp3[v].fader.accum.u32 -= mp3[v].fader.delta;
			else
				mp3[v].fader.accum.u32 = 0;

			if (mp3[v].fader.accum.u8[2] >= mp3[v].fader.targGainIdx)
				mp3[v].currGainIdx = mp3[v].fader.accum.u8[2];
			else
				mp3[v].currGainIdx = mp3[v].fader.targGainIdx;	
		}
		
	} // if (mp3[v].fader.active)
		
	return false;						
}


//*****************************************************************************
// mp3OpenFile
//*****************************************************************************
uint16_t mp3OpenFile(uint8_t v, uint16_t t, int16_t gainDb) {
/*
uint8_t *sdBuff;
q15_t newGain;
						   					   
	// Do some sanity checking						   
	if ((v >= MAX_NUM_MP3_VOICES) || (t >= MAX_NUM_TRACKS))
		return VOICE_ERR_BADINDEX;	
	if ((track[t].flags & TRACK_FLAG_EXISTS) == 0)
		return VOICE_ERR_BADINDEX;	
		
	// We'll read the first DOUBLE block directly into our wav buffer	
	sdBuff = (uint8_t *)&mp3[v].buff[0];
	
	if (!openFileByIndex(t, v, sdBuff))
		return VOICE_ERR_BADINDEX;

	mp3[v].mp3OutPtr = 0;
	mp3[v].mp3InPtr = BYTES_PER_BLOCK * 2;
	mp3[v].bytesSdRead = BYTES_PER_BLOCK * 2;
	mp3[v].bytesFetched = 0;
	mp3[v].framesPlayed = 0;
	
	mp3[v].wavInPtr = 0;
	mp3[v].wavOutPtr = 0;
			
	mp3[v].stopReqFlag = false;
	mp3[v].loopFlag = false;
	mp3[v].lockFlag = false;
	mp3[v].eofFlag = false;
	mp3[v].track = t;
	mp3[v].size = track[t].fileSize;
	
	// Set initial gain	
	mp3[v].currGainIdx = dBtoIndex(gainDb);
	newGain = gain_tble[mp3[v].currGainIdx];
	mp3[v].currGain = newGain;
	
	while (mp3CheckWavSpace(v)) {
		mp3DecodeWavData(v);
	}
*/	
	return VOICE_ERR_NOERROR;
}


//*****************************************************************************
// mp3CheckSdMp3Space
//*****************************************************************************
// This function returns true if there's room in the voice's MP3 data buffer
//  to read another block from the microSD card.
//*****************************************************************************
bool mp3CheckSdMp3Space(uint8_t v) {
	
uint16_t tp;
	
	tp = mp3[v].mp3OutPtr;
	if (mp3[v].mp3InPtr >= tp ) {
		if ((MP3_BUFFER_SIZE - (mp3[v].mp3InPtr - tp)) > BYTES_PER_BLOCK)
			return true;
	}
	else if ((tp - mp3[v].mp3InPtr) > BYTES_PER_BLOCK)
		return true;
	return false;
}

//*****************************************************************************
// mp3ReadSdMp3Data
//*****************************************************************************
// This function reads the next block from the SD MP3 file into the voice's
//  MP3 buffer. It should only be called after checking that there's enough
//  room by calling mp3CheckSdMp3Space(). It returns the number of bytes
//  read, which will be less than a block if we reach the end of the file.
//*****************************************************************************
int16_t mp3ReadSdMp3Data(uint8_t v)
{
	
uint32_t b;
uint16_t tmp16;
uint8_t * srcPtr;
uint8_t * dstPtr;
	
	if (mp3[v].eofFlag)
		return 0;
	
	if ((b = mp3[v].size.lSize - mp3[v].bytesSdRead) > BYTES_PER_BLOCK)
		b = BYTES_PER_BLOCK;

	// Set up our source and destination pointers for the copy
	dstPtr = &mp3[v].buff[mp3[v].mp3InPtr];
	
	//if (!readNextBlock(v, (uint8_t *)gVoiceSdBuff))
	//	return 0;
	
	srcPtr = &gVoiceSdBuff[0];

	// Copy the buffer
	if ((tmp16 = MP3_BUFFER_SIZE - mp3[v].mp3InPtr) >= b) {
		memcpy(dstPtr, srcPtr, b);
		mp3[v].mp3InPtr += b;
		if (mp3[v].mp3InPtr >= MP3_BUFFER_SIZE)
			mp3[v].mp3InPtr -= MP3_BUFFER_SIZE;
	}
	else {
		memcpy(dstPtr, srcPtr, tmp16);
		srcPtr = &gVoiceSdBuff[tmp16];
		dstPtr = &mp3[v].buff[0];
		tmp16 = b - tmp16;
		memcpy(dstPtr, srcPtr, tmp16);
		mp3[v].mp3InPtr = tmp16;
	}

	if (mp3[v].mp3InPtr == mp3[v].mp3OutPtr)
		return 0;
	
	mp3[v].bytesSdRead += b;
	if (mp3[v].bytesSdRead >= mp3[v].size.lSize) {
		mp3[v].eofFlag = true;
		tmp16 = BYTES_PER_BLOCK - (mp3[v].bytesSdRead - mp3[v].size.lSize);
		mp3[v].bytesSdRead = mp3[v].size.lSize;
		return tmp16;
	}
	return (BYTES_PER_BLOCK);
}


//*****************************************************************************
// mp3FetchMp3Data
//*****************************************************************************
// This function is used by the MP3 Decoder to fetch the specified number of 
//  bytes from the voice's MP3 buffer. It returns either the number requested
//  or the actual number if there are less than requested available.
//*****************************************************************************
uint16_t mp3FetchMp3Data(uint8_t v, uint8_t *pDest, uint16_t reqBytes) {

uint32_t b;
uint32_t numBytes;
uint32_t tmp32;
uint16_t nReturn;

uint8_t *ptrSrc = &mp3[v].buff[mp3[v].mp3OutPtr];
uint8_t *ptrDst = pDest;
	
	if ((numBytes = mp3[v].size.lSize - mp3[v].bytesFetched) > reqBytes)
		numBytes = reqBytes;
	mp3[v].bytesFetched += numBytes;
	nReturn = numBytes;
	if (numBytes == 0)
		return 0;

	// If this read doesn't wrap the end of the mp3 buffer, just do
	//  a block copy of the requested number of bytes
	if ((tmp32 = (MP3_BUFFER_SIZE - mp3[v].mp3OutPtr)) >= numBytes) {
		memcpy(ptrDst, ptrSrc, numBytes);
		mp3[v].mp3OutPtr += numBytes;
		if (mp3[v].mp3OutPtr >= MP3_BUFFER_SIZE)
			mp3[v].mp3OutPtr -= MP3_BUFFER_SIZE;
	}
	
	// Else we have to wrap. Tmp32
	//  contains the number of samples to the end of the buffer
	else {	
		b = tmp32;		
		memcpy(ptrDst, ptrSrc, b);
		ptrDst += b;
		ptrSrc = &mp3[v].buff[0];
		tmp32 = numBytes - tmp32;
		b = tmp32;
		memcpy(ptrDst, ptrSrc, b);
		ptrDst += b;
		mp3[v].mp3OutPtr = b;
	}	
	return nReturn;
}

//*****************************************************************************
// mp3CheckWavSpace
//*****************************************************************************
bool mp3CheckWavSpace(uint8_t v) {
	
uint16_t tp;
	
	tp = mp3[v].wavOutPtr;
	if (mp3[v].wavInPtr >= tp ) {
		if ((MP3_WAV_BUFFER_SIZE - (mp3[v].wavInPtr - tp)) > MP3_FRAME_SIZE_IN_SAMPLES)
			return true;
	}
	else if ((tp - mp3[v].wavInPtr) > MP3_FRAME_SIZE_IN_SAMPLES)
		return true;
	return false;
}


// ****************************************************************************
// voiceFetchWavData
// ****************************************************************************
int16_t mp3DecodeWavData(uint8_t v) {

	return mp3DecodeNewData(v);
}


//*****************************************************************************
// mp3PutWavData
//*****************************************************************************
// This function is called to put the decoded wav output into the voice's wav
//  buffer. It should always be preceded by a call to mp3CheckWavSpace to
//  first ensure that there's sufficient room to hold one MP3 frame's worth
//  of samples.
//*****************************************************************************
bool mp3PutWavData(uint8_t v, q15_t *pSrc, uint16_t numFrames){

uint16_t numSamples;
uint16_t n;
uint16_t tmp16;
	
	q15_t *ptrSrc;
	q15_t *ptrDst;
	
	ptrSrc = pSrc;
	ptrDst = &mp3[v].wavBuff[mp3[v].wavInPtr];
	numSamples = numFrames * 2;
	
	if ((tmp16 = (MP3_WAV_BUFFER_SIZE - mp3[v].wavInPtr)) >= numSamples) {
		arm_copy_q15(ptrSrc, ptrDst, numSamples);
		mp3[v].wavInPtr += numSamples;
	}
	
	// Else we have to wrap. Tmp16 contains the number of samples to the
	// 	end of the buffer
	else {	
		n = tmp16;		
		arm_copy_q15(ptrSrc, ptrDst, n);
		ptrSrc += n;
		ptrDst = &mp3[v].wavBuff[0];
		n = numSamples - tmp16;
		arm_copy_q15(ptrSrc, ptrDst, n);
		mp3[v].wavInPtr = n;
	}
	return true;
}


//*****************************************************************************
// Function:    mp3GetAudio
//*****************************************************************************
// This function is called by the audio callback to mix the requested number
//  sample frames from the voice's decoded wav buffer. Because of the way the
//  code is structure, there should always be this many samples available UN-
//  LESS we have reached the end of the mp3 file.
//*****************************************************************************
bool mp3GetAudio(uint8_t v, q15_t * pDest, uint16_t reqFrames) {

uint16_t reqSamples;
uint16_t numSamples;
uint16_t n;
uint32_t samplesInBuffer;
uint32_t tmp32;
q15_t newGain;

q15_t * qPtrSrc;
q15_t * qPtrDst;
	
	reqSamples = 2 * reqFrames;
	
	numSamples = reqSamples;
	
	// If there aren't enough samples in the buffer, adjust the count
	if (mp3[v].wavInPtr > mp3[v].wavOutPtr)
		samplesInBuffer = mp3[v].wavInPtr - mp3[v].wavOutPtr;
	else
		samplesInBuffer = MP3_WAV_BUFFER_SIZE - (mp3[v].wavOutPtr - mp3[v].wavInPtr);
	if (samplesInBuffer < reqSamples)
		numSamples = samplesInBuffer;

	// Check for a stop request
	if (mp3[v].stopReqFlag) {
		if (mp3[v].currGainIdx > 0)
			mp3[v].currGainIdx = 0;
		else
			return true;
	}

	// Service the fader and return if it's complete and marked for stop
	if (mp3ServiceFader(v))
		return true;

	qPtrSrc = &mp3[v].wavBuff[mp3[v].wavOutPtr];
	qPtrDst = &gMP3VoiceBuff[0];
	
	// If this read doesn't wrap the end of the wav buffer
	if ((tmp32 = (MP3_WAV_BUFFER_SIZE - mp3[v].wavOutPtr)) >= numSamples) {
						
		// Copy the requested number of samples to the destination mono
		//  buffers
		arm_copy_q15(qPtrSrc, qPtrDst, numSamples);
		qPtrDst += numSamples;
		mp3[v].wavOutPtr += numSamples;
	}
	
	// Else we have to wrap. Tmp32
	//  contains the number of samples to the end of the buffer
	else {	
		n = tmp32;		
		arm_copy_q15(qPtrSrc, qPtrDst, n);
		qPtrDst += n;
		qPtrSrc = &mp3[v].wavBuff[0];
		n = numSamples - tmp32;
		arm_copy_q15(qPtrSrc, qPtrDst, n);
		qPtrDst += n;
		mp3[v].wavOutPtr = n;
	}

	// If end of file, fill balance with silence
	if (numSamples != reqSamples) {	
		numSamples = (reqSamples - numSamples);
		arm_fill_q15(0, qPtrDst, numSamples);
	}

/*	
	// Calculate a new gain value based on the current voice gain index and the output
	//  gain index.
	newGain = gain_tble[mp3[v].currGainIdx];
	
	// Apply track gain to the data in the source buffer and mix
	//  results to the destination buffer
	if (newGain == mp3[v].currGain) {
		arm_scale_q15(gMP3VoiceBuff, mp3[v].currGain, 0, gMP3VoiceBuff, reqSamples);
		mp3[v].fader.shortFade = 0;
		mp3[v].fader.shortFadeDone = true;
	}
	else {
		
		// If fading over the whole buffer
		if (mp3[v].fader.shortFade == 0) {
			arm_ramp_q15(gMP3VoiceBuff, mp3[v].currGain, newGain, 0, gMP3VoiceBuff, reqSamples);
			mp3[v].currGain = newGain;
		}
		
		// Short fade
		else {
			tmp32 = reqSamples / 3;
			if (mp3[v].fader.shortFade == 2)
				tmp32 = tmp32 << 1;
			arm_ramp_q15(gMP3VoiceBuff, mp3[v].currGain, newGain, 0, gMP3VoiceBuff, tmp32);
			arm_scale_q15(&gMP3VoiceBuff[tmp32], newGain, 0, &gMP3VoiceBuff[tmp32], reqSamples - tmp32);
			mp3[v].currGain = newGain;
			mp3[v].fader.shortFade = 0;
			mp3[v].fader.shortFadeDone = true;
		}			
	}
*/	
	// Add the result to the destination output mix buffer
	arm_add_q15(pDest, gMP3VoiceBuff, pDest, reqSamples);
	
	mp3[v].framesPlayed += reqFrames;

	return false;

}

