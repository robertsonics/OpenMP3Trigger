// ****************************************************************************
//     Filename: MEMORY.C
// Date Created: 5/19/2025
//
//     Comments: Global memory allocations for the OpenMP3Player
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

uint8_t gSdBuff[8192] __attribute__((aligned (32)));

MP3_VOICE_STRUCTURE mp3[MAX_NUM_MP3_VOICES] __attribute__((aligned (32)));		// Our voice structure array

q15_t gVoiceSdBuff[SAMPLES_PER_BLOCK] __attribute__((aligned (32)));


