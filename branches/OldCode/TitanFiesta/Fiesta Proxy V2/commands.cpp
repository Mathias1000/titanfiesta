#include "main.hpp"

void HandleCommand(CPacket* pak, char* command, CConnectClient* game){
	char* args = strchr(command, ' ');
	if(args != NULL){ *args = 0; args++; }

	Log(MSG_DEBUG, "Command: %s", command);

	if(_strcmpi(command, "&tele") == 0){
		//0 = Roumen
		//1 = Elderine
		//2 = Sand Hill
		//3 = Forest of Mist
		byte mapid = strtoul(args, NULL, 0) & 0xFF;

		pak->ResetPacket(0x181A);
		pak->Add<byte>(mapid);
		pak->SetBuffer();
	}
}

/*
To Burning Hill from Sand Hill
04 0a 20 e5 08//Get teleport
03 02 3c 00//Yes

To Sand Hill from Burning Hill
04 0a 20 98 08 //Get Teleport
03 02 3c 00//Yes
*/
