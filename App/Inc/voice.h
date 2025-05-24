// ****************************************************************************
//     Filename: VOICE.H
// Date Created: 2/24/2024
//
//     Comments: Header file for the Robertsonics Voice module
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

#define VOICE_ERR_NOERROR		0
#define VOICE_ERR_BADINDEX		1
#define VOICE_ERR_BADOPEN		2

#define VOICE_STATE_AVAIL		0
#define VOICE_STATE_PLAYING		1
#define VOICE_STATE_PAUSED		2
#define VOICE_STATE_STOPPED		3


// Function prototypes for this module

void voicesInit(void);
void voicesStopAll(void);
void voicesService(void);
uint8_t voicesCheck(void);

