#include "main.h"

CListener listener;

bool ReceivedLoginServerPacket(CPacket* pak, CConnectClient* login){
	switch(pak->command){
		case 0x0C0C:
			listener.SetWorldIP(pak->Get<char*>(0x04));
			listener.SetWorldPort(pak->Get<word>(0x14));

			pak->Set(FixLenStr("127.0.0.1", 0x10), 0x04);
			pak->Set<word>(9110, 0x14);
		break;
}
	return true;
}

bool ReceivedWorldServerPacket(CPacket* pak, CConnectClient* world){
	switch(pak->command){
		case 0x1003:
			listener.SetGameIP(pak->Get<char*>(0x03));
			listener.SetGamePort(pak->Get<word>(0x13));

			pak->Set(FixLenStr("127.0.0.1", 0x10), 0x03);
			pak->Set<word>(9210, 0x13);
		break;
	}
	return true;
}

bool ReceivedGameServerPacket(CPacket* pak, CConnectClient* game){
	switch(pak->command){
		case 0x180A:
			listener.SetGameIP(pak->Get<char*>(0x0D));
			listener.SetGamePort(pak->Get<word>(0x1D));

			pak->Set(FixLenStr("127.0.0.1", 0x10), 0x0D);
			pak->Set<word>(9210, 0x1D);
		break;
	}
	return true;
}

bool ReceivedLoginClientPacket(CPacket* pak, CConnectClient* login){
	return true;
}

bool ReceivedWorldClientPacket(CPacket* pak, CConnectClient* world){
	return true;
}

bool ReceivedGameClientPacket(CPacket* pak, CConnectClient* game){
	return true;
}

int main(int argc, char** argv){
	if(argc < 4){
		Log(MSG_ERROR, "%s <email> <password> <loginip>", argv[0]);
		return 0;
	}

	char* email = argv[1];
	char* password = argv[2];	
	char* loginip = argv[3];

	WORD sockVersion;
	WSADATA wsaData;
	sockVersion = MAKEWORD(1, 1);
	WSAStartup(sockVersion, &wsaData);
	
	CFiestaLauncher launcher;
	launcher.Launch(email, password);
	
	if(!listener.Start(loginip, 9010)){
		Log(MSG_ERROR, "Listener.Start() returned false!");
		return 0;
	}

	WSACleanup();

	return 0;
}
