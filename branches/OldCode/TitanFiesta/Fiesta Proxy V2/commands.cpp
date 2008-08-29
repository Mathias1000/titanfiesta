#include "main.hpp"

byte outBuffer[256];

void HandleCommand(CPacket* pak, char* command, CConnectClient* game){
	char* args = strchr(command, ' ');
	if(args != NULL){ *args = 0; args++; }

	Log(MSG_DEBUG, "Command: %s", command);

	if(_strcmpi(command, "&tele") == 0){
		byte mapid = strtoul(args, NULL, 0) & 0xFF;

		pak->ResetPacket(0x181A);
		pak->Add<byte>(mapid);
		pak->SetBuffer();
	}
}