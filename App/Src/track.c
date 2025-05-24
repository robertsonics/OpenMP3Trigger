// ****************************************************************************
//     Filename: TRACK.C
// Date Created: 5/19/2025
//
//     Comments: Track module for the Robertsonics OpenMP3 Player
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

extern FATFS fatFs;
extern TRACK_STRUCTURE track[];			// Our track structure array


// ****************************************************************************
// Global variables


//*****************************************************************************
// initTracks
//*****************************************************************************
bool trackInit(uint16_t *tNum) {
	
int i;
FRESULT fRslt;
uint16_t numMp3 = 0;

DIR dir;
FILINFO fInfo;

	for (i = 0; i < MAX_NUM_TRACKS; i++) {
		track[i].flags = 0;
		track[i].fileSize.lSize = 0;
	}

	fRslt = f_mount(&fatFs, "", 1);
	if (fRslt != FR_OK) return false;

	fRslt = f_findfirst(&dir, &fInfo, "SOUNDS", "*.mp3");
	if (fRslt != FR_OK) return false;

	while ((fRslt == FR_OK) && (fInfo.fname[0] != 0)) {
		numMp3++;
		fRslt = f_findnext(&dir, &fInfo);
	}
	*tNum = numMp3;
	return true;
	
}
