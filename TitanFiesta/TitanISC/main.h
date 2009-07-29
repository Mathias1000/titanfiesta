#pragma once

//Required Defines
#define DEFAULT_PACKET_SIZE 0x100
#define TITAN_ADDSTRING_NULLTERMINATED

//Optional Extra Includes
//#define TITAN_USING_ISC
#define TITAN_IS_ISC_SERVER
#define TITAN_USING_RANDOM

//Packet Setting
#define TITAN_USING_PACKETS
#define PACKET_HEADER_SIZE 4

//Other Settings
#define TITAN_CLIENTS_PER_THREAD 10

#include "..\..\TitanBase\TitanBase\TitanGlobals.h"

class CPacket : public CTitanPacket
{
private:
public:
	CPacket(dword command = 0){
		_Command = command;
		_Size = 0;
		_CurPos = 0;
		Add<word>(2);
		Add<word>(_Command);
	}
};
