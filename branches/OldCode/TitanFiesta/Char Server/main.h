#pragma once

//Required Defines
#define DEFAULT_PACKET_SIZE 0x100
#define TITAN_ADDSTRING_NULLTERMINATED

//Optional Extra Includes
#define TITAN_USING_ISC
#define TITAN_USING_INIREAD
#define TITAN_USING_MYSQL

//Packet Setting
#define TITAN_USING_PACKETS
#define PACKET_HEADER_SIZE 3

//Other Settings
#define TITAN_CLIENTS_PER_THREAD 2

#include "..\..\TitanBase\TitanBase\TitanGlobals.h"
#include "..\Common\XorTable.h"
#include "..\Common\Common.h"

class CPacket : public CTitanPacket
{
private:
public:
	CPacket(dword command = 0){
		_Command = command;
		_Size = 0;
		_CurPos = 0;
		Add<byte>(2);
		Add<word>(_Command);
	}
};
