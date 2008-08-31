#include "main.hpp"

CListener listener;
byte outBuffer[256];
int curX, curY;

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
		case 0x1C0A:
			{
				word dropid = pak->Read<word>();
				word itemid = pak->Read<word>();

				int dropY = pak->Read<dword>();
				int dropX = pak->Read<dword>();

				float xDiff = float(curX - dropX);
				float yDiff = float(curY - dropY);

				float distance = sqrt((xDiff*xDiff) + (yDiff*yDiff));
				Log(MSG_INFO, "Drop Distance: %4.2f", distance);
				if(distance < 200.0f){
					CPacket pakout(0x3009, outBuffer);
					pakout.Add<word>(dropid);
					pakout.SetBuffer();
					game->SendServerPacket(&pakout);
				}
			}
		break;
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
				HandleCommand(pak, reinterpret_cast<char*>(pak->buffer + pak->pos + 1), game);
			}
		break;
		case 0x2019:
			pak->Read<qword>();
			curY = pak->Read<dword>();
			curX = pak->Read<dword>();
		break;
		
		//2401 = attack word <client id>

		//Pickup Drop 04 09 30 51 24 word <client id>

		//word <client id> word <item id>
		//Tough Meat 11 0a 1c bc 20 b9 0b a8 45 00 00 c9 3f 00 00 a3 4d 08
		//Tough Meat 11 0a 1c 75 24 b9 0b b3 44 00 00 3d 3f 00 00 a3 49 08
		//SBL LQ     11 0a 1c 37 22 ee 0b cc 44 00 00 3d 3f 00 00 a3 49 08
	}
}

int main(int argc, char** argv){
	if(argc < 3){
		Log(MSG_ERROR, "%s <email> <password>", argv[0]);
		return 0;
	}

	char* email = argv[1];
	char* password = argv[2];	

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