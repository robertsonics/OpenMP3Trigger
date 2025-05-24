// ****************************************************************************
//     Filename: TRACK.H
// Date Created: 5/19/2025
//
//     Comments: Track module header for the Robertsonics OpenMP3 Player
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

// Define the max number of tracks for this build. Tracks are numbered
//  starting at 0.  

#define MAX_NUM_TRACKS			4096

// Bit definitions for the track flag byte

#define TRACK_FLAG_EXISTS		0x80
#define TRACK_FLAG_LOOP			0x40
#define TRACK_FLAG_LOCK			0x20
#define TRACK_FLAG_MP3			0x10

// The following structure defines a track

#pragma pack(1)
typedef struct {
	uint8_t flags;				// Flags
	uint8_t dummy;				// 
	FILE_SIZE fileSize;			// File size
} TRACK_STRUCTURE;
#pragma pack()

// Function prototypes for this module

bool trackInit(uint16_t * tnum);

