/*
    NitroHax -- Cheat tool for the Nintendo DS
    Copyright (C) 2008  Michael "Chishm" Chisholm

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <nds.h>

#include <stdio.h>
#include <string.h>
#include <initializer_list>

#include "common/tonccpy.h"
#include "nds_card.h"
#include "launch_engine.h"
#include "crc.h"

struct ndsHeader {
sNDSHeader header;
char padding[0x200 - sizeof(sNDSHeader)];
};

bool consoleInited = false;
bool scfgUnlock = false;
int TWLMODE = 0;
bool TWLCLK = false;	// false == NTR, true == TWL
int TWLVRAM = 0;
bool TWLTOUCH = false;
bool soundFreq = false;
bool sleepMode = true;
bool runCardEngine = false;
bool EnableSD = false;
bool ignoreBlacklists = false;
int language = -1;

u8 cheatData[0x8000] = {0};

int main() {
	fifoSendValue32(FIFO_PM, PM_REQ_SLEEP_DISABLE);
	
	// reuse ds header parsed by the ds firmware and then altered by flashme
	auto& ndsHeader = *((struct ndsHeader*)__NDSHeader);

	memcpy(((char*)&ndsHeader) + 0x80, ((char*)&ndsHeader) + 0x94, 8);
	memset(((char*)&ndsHeader) + 0x94, 0, 8);
	
	//arm9executeAddress and cardControl13 are altered by flashme and become unrecoverable
	//use some heuristics to try to guess them back and check for the header crc to match
	ndsHeader.header.arm9executeAddress = ((char*)ndsHeader.header.arm9destination) + 0x800;
	bool crc_matched = false;
	for(auto control : {0x00416657, 0x00586000, 0x00416017}) {
		ndsHeader.header.cardControl13 = control;
		if(crc_matched = ndsHeader.header.headerCRC16 == swiCRC16(0xFFFF, (void*)&ndsHeader, 0x15E); crc_matched) {
			break;
		}
	}
	if(!crc_matched){
		if (!consoleInited) {
			consoleDemoInit();
			consoleInited = true;
		} else {
			consoleClear();
		}
		iprintf ("crc doens't match\n");
		while (1);
	}

	while (1) {
		// if (runCardEngine) {
			// cheatData[3] = 0xCF;
			// off_t wideCheatSize = getFileSize("sd:/_nds/nds-bootstrap/wideCheatData.bin");
			// if (wideCheatSize > 0) {
				// FILE* wideCheatFile = fopen("sd:/_nds/nds-bootstrap/wideCheatData.bin", "rb");
				// fread(cheatData, 1, wideCheatSize, wideCheatFile);
				// fclose(wideCheatFile);
				// cheatData[wideCheatSize+3] = 0xCF;
			// }
			// tonccpy((void*)0x023F0000, cheatData, 0x8000);
		// }
		runLaunchEngine();
	}
	return 0;
}

