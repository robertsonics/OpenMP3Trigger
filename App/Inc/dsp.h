// ****************************************************************************
//     Filename: DSP.H
// Date Created: 5/19/2025
//
//     Comments: DSP module header for the Robertsonics OpenMP3 Player
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

#define MIN_GAIN_DB				-80
#define MAX_GAIN_DB				0
#define MUTE_GAIN_DB			-100

#define MAX_GAIN_TABLE_ENTRY	(((MAX_GAIN_DB - MIN_GAIN_DB) * 2) - 1)

#define MAKEQ1_15(x) ((int)dspClip16(((x) * 32768.0f) + 0.5f))
#define MAKEQ2_14(x) ((int)dspClip16(((x) * 16384.0f) + 0.5f)) 
#define MAKEQ3_13(x) ((int)dspClip16(((x) * 8192.0f) + 0.5f)) 
#define MAKEQ4_12(x) ((int)dspClip16(((x) * 4096.0f) + 0.5f)) 
#define MAKEQ5_11(x) ((int)dspClip16(((x) * 2048.0f) + 0.5f))
	
#define MAKEQ1_31(x) ((int)dspClip32(((x) * 2147483648.0f) + 0.5f))

// Function prototypes for this module

void initDsp(void);
uint32_t dspGetDither(void);
uint8_t dBtoIndex(int16_t dbGain);
void arm_ramp_q15(
	q15_t * pSrc,
	q15_t scaleFractStart,
	q15_t scaleFractEnd,
	int8_t shift,
	q15_t * pDst,
	uint32_t blockSize);
void arm_copy_split_q15(
	q15_t * pSrc,
	q15_t * pDstL,
	q15_t * pDstR,
	uint32_t blockSize);
void copy_q15(
	q15_t * pSrc,
	q15_t * pDst,
	uint32_t blockSize);
float dspClip16(float);
float dspClip32(float);

