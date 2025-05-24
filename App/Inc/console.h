// ****************************************************************************
//     Filename: CONSOLE.H
// Date Created: 5/19/2025
//
//     Comments: ASCII serial module header for the Robertsonics OpenMP3 Player
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

#define MAX_CONSOLE_CMD_LEN		64
#define MAX_CONSOLE_PARAMS		8

// Function prototypes for this module

void consoleInit(void);
void consoleService(void);
void consoleDoCommand(void);
bool consoleParseLine(void);
int consoleGetCommand(char * cmdString);
int consoleGetValue(int32_t * val);

void consoleSyntaxErr(void);
void consoleSignOn(void);

bool consoleNewLine(int nl);
bool consoleSendString(char * pMsg);
bool consoleSendBytes(char * pMsg, uint8_t len);
bool consoleSendInt32(uint32_t n);

