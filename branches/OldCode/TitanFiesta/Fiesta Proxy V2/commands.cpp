#include "main.hpp"

byte outBuffer[256];

void HandleCommand(char* command, CConnectClient* game){
	char* args = strchr(command, ' ');
	if(args != NULL){ *args = 0; args++; }

	Log(MSG_DEBUG, "Command: %s", command);

	if(_strcmpi(command, "&emote") == 0){
		CPacket pakout(0x2020, outBuffer);
		byte emote = strtoul(args, NULL, 0);
		pakout.Add<byte>(emote);
		game->SendServerPacket(&pakout);
	}
}