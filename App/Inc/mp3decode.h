// ****************************************************************************
//     Filename: MP3DECODE.H
// Date Created: 5/19/2025
//
//     Comments: Decoder interface header for the Robertsonics OpenMP3 Player
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

#define MP3_FRAME_SIZE_IN_FRAMES	576
#define MP3_FRAME_SIZE_IN_SAMPLES 	2 * MP3_FRAME_SIZE_IN_FRAMES

// Function prototypes for this module

void mp3DecodeInit(void);
int16_t mp3DecodeNewData(uint8_t v);

