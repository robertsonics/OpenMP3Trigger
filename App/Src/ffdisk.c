// ****************************************************************************
//     Filename: FFDISK.C
// Date Created: 5/19/2025
//
//     Comments: FatFs interface module for the Robertsonics OpenMP3 Player
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
#include "diskio.h"

// ****************************************************************************
// External variables



// ****************************************************************************
// Global variables



DSTATUS disk_initialize (BYTE pdrv) {

UNUSED(pdrv);
	
	return RES_OK;
}

DSTATUS disk_status (BYTE pdrv) {

UNUSED(pdrv);

	return RES_OK;
}

DRESULT disk_read (BYTE pdrv, BYTE* buff, LBA_t sector, UINT count) {

UNUSED(pdrv);

    if (biosSdReadSectors(buff, sector, count))
		return RES_OK;
	return RES_ERROR;
}

DRESULT disk_write (BYTE pdrv, const BYTE* buff, LBA_t sector, UINT count) {

UNUSED(pdrv);
	
    if (biosSdWriteSectors((BYTE *)buff, sector, count))
		return RES_OK;
	return RES_ERROR;
}

DRESULT disk_ioctl (BYTE pdrv, BYTE cmd, void* buff) {

UNUSED(pdrv);
UNUSED(cmd);
UNUSED(buff);

	return RES_OK;
}

DWORD get_fattime (void) {
	return 0;
}


