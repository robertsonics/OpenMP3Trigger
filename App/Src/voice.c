// ****************************************************************************
//     Filename: VOICE.C
// Date Created: 2/24/2024
//
//     Comments: Voice module for Robertsonics Audio Players
//
// Build Environment: Keil uVision5, MDK-ARM
//                    STM32CubeMX
//
// COPYRIGHT (c) 2024, Robertsonics, All Rights Reserved
//
// Programmers: Jamie Robertson (508)397-8527
//              robertsonics@gmail.com
//
// ****************************************************************************

#include "player.h"


// ****************************************************************************
// External variables

extern MP3_VOICE_STRUCTURE mp3[];	// Our mp3 structure array
extern WAV_VOICE_STRUCTURE wav[];	// Our mp3 structure array

extern Q1_15 gain_tble[];			// Our gain table

extern TRACK_STRUCTURE track[];		// Our track structure array


// ****************************************************************************
// Global variables

uint8_t gNumMP3Voices;
uint8_t gNumWavVoices;
uint8_t gVoiceIndex = 0;					// Our global voice index



//*****************************************************************************
// voicesInit
//*****************************************************************************
void voicesInit(void) {
	
uint8_t v;
	
	// Initialize our MP3 voices
	for (v = 0; v < MAX_NUM_MP3_VOICES; v++) {
		memset((uint8_t *)&mp3[v], 0, sizeof(MP3_VOICE_STRUCTURE));
		mp3[v].state = VOICE_STATE_AVAIL;
		mp3[v].fader.active = false;
	}
	gNumMP3Voices = MAX_NUM_MP3_VOICES;
	mp3DecodeInit();
	
	for (v = 0; v < MAX_NUM_WAV_VOICES; v++) {
		memset((uint8_t *)&wav[v], 0, sizeof(WAV_VOICE_STRUCTURE));
		mp3[v].state = VOICE_STATE_AVAIL;
		mp3[v].fader.active = false;
	}
	gNumWavVoices = MAX_NUM_MP3_VOICES;

}


//*****************************************************************************
// voicesStopAll
//*****************************************************************************
void voicesStopAll(void) {
	
uint8_t v;
	
	for (v = 0; v < gNumMP3Voices; v++) {
		mp3Stop(v);
	}
	
	for (v = 0; v < gNumWavVoices; v++) {
		wavStop(v);
	}

}


//*****************************************************************************
// voicesCheck
//*****************************************************************************
uint8_t voicesCheck(void) {

uint8_t v;
uint8_t cnt;;

	cnt = 0;
	for (v = 0; v < gNumMP3Voices; v++) {
		if (mp3[v].state == VOICE_STATE_PLAYING)
			cnt++;
	}
	for (v = 0; v < gNumWavVoices; v++) {
		if (wav[v].state == VOICE_STATE_PLAYING)
			cnt++;
	}
	return cnt;
}


//*****************************************************************************
// voicesService
//*****************************************************************************
void voicesService(void) {
	
	if (mp3[gVoiceIndex].state == VOICE_STATE_PLAYING) {
		while (mp3CheckWavSpace(gVoiceIndex)) {
			mp3DecodeWavData(gVoiceIndex);
		}
	}
	if (++gVoiceIndex >= MAX_NUM_MP3_VOICES)
		gVoiceIndex = 0;	
}


