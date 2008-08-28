#include "main.hpp"

CListener listener;

void ReceivedLoginServerPacket(CPacket* pak, CConnectClient* login){
	switch(pak->command){
		case 0x0C0C:
			listener.SetWorldIP(pak->Get<char*>(0x04));
			listener.SetWorldPort(pak->Get<word>(0x14));

			pak->Set(FixLenStr("127.0.0.1", 0x10), 0x04);
			pak->Set<word>(9110, 0x14);
		break;
	}
}

void ReceivedWorldServerPacket(CPacket* pak, CConnectClient* world){
	switch(pak->command){
		case 0x1003:
			listener.SetGameIP(pak->Get<char*>(0x03));
			listener.SetGamePort(pak->Get<word>(0x13));

			pak->Set(FixLenStr("127.0.0.1", 0x10), 0x03);
			pak->Set<word>(9210, 0x13);
		break;
	}
}

void ReceivedGameServerPacket(CPacket* pak, CConnectClient* game){
	switch(pak->command){
		case 0x180A:
			listener.SetGameIP(pak->Get<char*>(0x0D));
			listener.SetGamePort(pak->Get<word>(0x1D));

			pak->Set(FixLenStr("127.0.0.1", 0x10), 0x0D);
			pak->Set<word>(9210, 0x1D);
		break;
	}
}

void ReceivedLoginClientPacket(CPacket* pak, CConnectClient* login){
}

void ReceivedWorldClientPacket(CPacket* pak, CConnectClient* world){
}

void ReceivedGameClientPacket(CPacket* pak, CConnectClient* game){
	switch(pak->command){
		case 0x2001:
			if(pak->buffer[pak->pos + 1] == '&'){
				pak->buffer[pak->size] = 0;
				HandleCommand(reinterpret_cast<char*>(pak->buffer + pak->pos + 1), game);
			}
		break;
	}
}

int main(int argc, char** argv){
	char* email;
	char* password;
	if(argc < 3){
		email = "james.benton2@hotmail.com";
		password = "0589cc40d464f3fc1da74caddeaa1fb4";
	}else{
		email = argv[1];
		password = argv[2];
	}
	WORD sockVersion;
	WSADATA wsaData;
	sockVersion = MAKEWORD(1, 1);
	WSAStartup(sockVersion, &wsaData);

	CFiestaLauncher launcher;
	launcher.Launch(email, password);

	if(!listener.Start("64.127.118.7", 9010)){
		Log(MSG_ERROR, "Listener.Start() returned false!");
		return 0;
	}

	WSACleanup();

	return 0;
}