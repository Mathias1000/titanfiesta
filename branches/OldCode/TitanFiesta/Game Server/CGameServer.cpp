#include "main.h"
#include "CGameServer.hpp"

bool CGameServer::OnServerReady(){
	CTitanIniReader ini("GameServer.ini");
	db = new CTitanSQL(ini.GetString("Server","MySQL","127.0.0.1"), ini.GetString("Username","MySQL","root"), ini.GetString("Password","MySQL",""), ini.GetString("Database","MySQL","titanfiesta"));
	if(!db->Connect()){
		Log(MSG_STATUS, "Failed to connect to MYSQL Server");
	}else{
		Log(MSG_STATUS, "Connected to MYSQL Server");
	}
	
	ServerData.flags = 0;
	ServerData.id = ini.GetInt("Id","Game Server",3);
	ServerData.ip = Config.BindIp;
	ServerData.iscid = 99;
	ServerData.name = ini.GetString("Name","Game Server","Game Server");
	ServerData.owner = ini.GetInt("Owner","Game Server",2);
	ServerData.port = Config.BindPort;
	ServerData.status = 9;
	ServerData.type = 3;

	CISCPacket pakout(TITAN_ISC_IDENTIFY);
	pakout.Add<CServerData*>( &ServerData );
	SendISCPacket( &pakout );

	return true;
}

void CGameServer::OnReceivePacket( CTitanClient* baseclient, CTitanPacket* pak ){
	CGameClient* thisclient = (CGameClient*)baseclient;
	Log(MSG_INFO,"Received packet: Command: %04x Size: %04x", pak->Command(), pak->Size());

	if(thisclient->id == -1){
		if(pak->Command() == 0x1801)
			PACKETRECV(pakUserLogin);
	}else{
		switch(pak->Command()){
			case 0x0817:
				Log(MSG_DEBUG, "0x0817 Hide Players");
			break;
			case 0x1071:
				Log(MSG_DEBUG, "0x1071 Quitting Game...");
			break;
			case 0x1072:
				Log(MSG_DEBUG, "0x1072 Stop Quitting Game! WE WANT MORE PLAY!");
			break;
			case 0x2001:
				PACKETRECV(pakChat);
			break;
			case 0x2012:
				Log(MSG_DEBUG, "0x2012 Stop Moving -> cY: %d cX: %d", pak->Read<dword>(), pak->Read<dword>());
			break;
			case 0x2019:
				Log(MSG_DEBUG, "0x2019 Move Packet -> nY: %d nX: %d cY: %d cX: %d", pak->Read<dword>(), pak->Read<dword>(), pak->Read<dword>(), pak->Read<dword>());
			break;
			case 0x2020:
				Log(MSG_DEBUG, "0x2020 Do Emote -> id: %d", pak->Read<byte>());
			break;
			case 0x2024:
				Log(MSG_DEBUG, "0x2024 Jump!");
			break;
			case 0x2027:
				PACKETRECV(pakRest);
			break;
			case 0x202A:
				PACKETRECV(pakEndRest);
			break;
			case 0x3c02:
			{
				byte btWho = pak->Read<byte>();
				char* who;
				if(btWho == 0)
					who = "ExJam";
				else if(btWho == 1)
					who = "Drakia";
				else if(btWho == 2)
					who = "Brett";

				char buffer[255];
				sprintf_s(buffer, 255, "\"%s\" wants to have sex with %s!", thisclient->charname, who);
				CPacket pakout(0x6402);
				pakout.Add<byte>(11);
				pakout.AddStringLen<byte>(buffer);
				pakout.Add<byte>(0);
				SendPacket(thisclient, &pakout);
			}
			break;
			case 0x4822:
				{
					word skillId = pak->Read<word>();
				Log(MSG_DEBUG, "0x4822 Learn Production Skill -> id: %d", skillId);
				//[Game]IN 06 23 48 AD 71 01 0B 
				//[Game]IN 05 04 48 AD 71 00 
					{
						CPacket pakout(0x4823);
						pakout.Add<word>(skillId);
						pakout.Add<word>(0x0B01);
						SendPacket(thisclient, &pakout);
					}
					{
						CPacket pakout(0x4804);
						pakout.Add<word>(skillId);
						pakout.Add<byte>(0x00);
						SendPacket(thisclient, &pakout);
					}
				}
			break;
			case 0x6001:
				PACKETRECV(pakSetTitle);
			break;
			default:
				pak->Pos(0);
				printf("Unhandled Packet, Command: %04x Size: %04x:\n", pak->Command(), pak->Size());
				for(dword i = 0; i < pak->Size(); i++)
					printf("%02x ", pak->Read<byte>());
				printf("\n");
			break;
		}
	}
}

PACKETHANDLER(pakSetTitle){
	CPacket pakout(0x6002);
	pakout.Add<dword>(pak->Read<dword>());
	pakout.Add<word>(0x0a01);
	SendPacket(thisclient, &pakout);
	return true;
}

PACKETHANDLER(pakEndRest){
	CPacket pakout(0x202B);
	pakout.Add<word>(0x0a81);
	SendPacket(thisclient, &pakout);
	return true;
}

PACKETHANDLER(pakRest){
	CPacket pakout(0x2028);
	pakout.Add<word>(0x0a81);
	SendPacket(thisclient, &pakout);
	return true;
}

PACKETHANDLER(pakChat){
	byte chatLen = pak->Read<byte>();
	if(*(byte*)(pak->Buffer() + pak->Pos()) == '&'){
		char origText[255];
		memcpy(origText, pak->Buffer() + pak->Pos(), chatLen);
		origText[chatLen] = 0;
		char* command = (char*)(pak->Buffer() + pak->Pos() + 1);
		*(command + chatLen - 1) = 0;
		char* context;
		command = strtok_s(command, " ", &context);
		Log(MSG_DEBUG, "Lul wut GM COMMAND %s", command);
		if(_strcmpi(command, "title") == 0){
			char* titleId = strtok_s(NULL, " ", &context);
			char* titleLevel = strtok_s(NULL, " ", &context);
			if(titleId == NULL || titleLevel == NULL){
				Log(MSG_DEBUG, "Not enough arguments for &title");
				return true;
			}
			CPacket pakout(0x6004);
			pakout.Add<byte>(strtoul(titleId, NULL, 0));
			pakout.Add<byte>(0x80 | strtoul(titleLevel, NULL, 0));
			SendPacket(thisclient, &pakout);
		}else if(_strcmpi(command, "mon") == 0){
			char* monId = strtok_s(NULL, " ", &context);
			if(monId == NULL){
				Log(MSG_DEBUG, "Not enough arguments for &mon");
				return true;
			}
			 
			CPacket pakout(0x1C08);
			pakout.Add<word>(0x1336);
			pakout.Add<word>(strtoul(monId, NULL, 0));
			pakout.Add<dword>(0x251e);
			pakout.Add<dword>(0x0cb9);
			pakout.Add<dword>(1337);
			pakout.Fill<byte>(0, 0x21);
			SendPacket(thisclient, &pakout);
		}else if(_strcmpi(command, "delsoulja") == 0){
			char* souljaNumber = strtok_s(NULL, " ", &context);
			if(souljaNumber == NULL){
				Log(MSG_DEBUG, "Not enough arguments for &delsoulja <soulja count>");
				return true;
			}

			for(dword i = 0; i < strtoul(souljaNumber, NULL, 0); i++){
				CPacket pakout(0x1C0E);//deletes client id
				pakout.Add<word>(0x3000 + i);//ClientID
				SendPacket(thisclient, &pakout);
			}
		}else if(_strcmpi(command, "soulja") == 0){
			char* souljaNumber = strtok_s(NULL, " ", &context);
			char* txtRadius = strtok_s(NULL, " ", &context);
			if(souljaNumber == NULL){
				Log(MSG_DEBUG, "Not enough arguments for &soulja <soulja count>");
				return true;
			}

			dword souljaCount = strtoul(souljaNumber, NULL, 0);
			float angleIncrements = ((4.0 * atan( 1.0 )) * 2.0f) / float(souljaCount);
			float rotationIncrements = 180.0f / float(souljaCount);
			float radius = (txtRadius == NULL)?50.0f:float(atoi(txtRadius));
			word clientIdStart = 0x3000;
			dword xStart = 9110;
			dword yStart = 3516;
			char name[0x10];
			CPacket pakout(0x1c07);
			pakout.Add<byte>(souljaCount);
			for(dword i = 0; i < souljaCount; i++){
				pakout.Add<word>(clientIdStart + i);//ClientID
				sprintf_s(name, 0x10, "Soulja %d", i);
				pakout.AddFixLenStr(name, 0x10);
				pakout.Add<dword>(xStart + (cos(angleIncrements * float(i)) * float(radius)));//X
				pakout.Add<dword>(yStart + (sin(angleIncrements * float(i)) * float(radius)));//Y
				pakout.Add<byte>(180 - byte(rotationIncrements * float(i)));//Starting Rotation
				pakout.Add<byte>(0x01);//unk2
				pakout.Add<byte>(0x01);//Is Visible
				pakout.Add<byte>(0x85);//Profession bollocks
				pakout.Add<byte>(0x07);//hair
				pakout.Add<byte>(0x01);//hcolour
				pakout.Add<byte>(0x00);//face
				pakout.Add<word>(0xFFFF);//weapon
				pakout.Add<word>(0x8a45);//body
				pakout.Add<word>(0xFFFF);//shield
				pakout.Add<word>(0xFFFF);//pants
				pakout.Add<word>(0xFFFF);//boots
				pakout.Add<word>(0x7d89);//hat
				pakout.Fill<byte>(0xFF, 0x1a);
				pakout.Add<byte>(0x99);//Refine weapon << 4 | shield
				pakout.Add<word>(0x9999);
				pakout.Add<byte>(0x00);//0x25 nothing special
				pakout.Add<byte>(26);//Current "Emote"
				pakout.Add<word>(0x0000);//0x00 nothing?
				pakout.Add<word>(0xFFFF);//TitleId
				pakout.Add<word>(0x00);//Monster ID For Title
				//0x27 bytes of bit array, oh joys.
				pakout.Fill<byte>(0x00, 0x21);
				pakout.Add<byte>(0x40);
				pakout.Fill<byte>(0x00, 0x05);
				pakout.Add<byte>(0x02);
			}
			SendPacket(thisclient, &pakout);
			{
				CPacket pakout(0x6402);
				pakout.Add<byte>(11);
				pakout.AddStringLen<byte>("CRANK DAT SOULJA BOY");
				pakout.Add<byte>(0);
				SendPacket(thisclient, &pakout);
			}
		}else if(_strcmpi(command, "ask") == 0){
			CPacket pakout(0x3c01);
			pakout.AddFixLenStr("Who would you like to have sex with?", 0x40);
			pakout.Add<byte>(0x03);//Answer Count

			pakout.Add<byte>(0x00);
			pakout.AddFixLenStr("ExJam", 0x20);

			pakout.Add<byte>(0x01);
			pakout.AddFixLenStr("Drakia", 0x20);

			pakout.Add<byte>(0x02);
			pakout.AddFixLenStr("Brett", 0x20);

			SendPacket(thisclient, &pakout);
		}else if(_strcmpi(command, "drop") == 0){
			char* itemId = strtok_s(NULL, " ", &context);
			if(itemId == NULL){
				Log(MSG_DEBUG, "Not enough arguments for &drop");
				return true;
			}

			CPacket pakout(0x1c0A);
			pakout.Add<word>(0x1335);
			pakout.Add<word>(strtoul(itemId, NULL, 0));
			pakout.Add<dword>(0x251e);
			pakout.Add<dword>(0x0cb9);
			pakout.Add<byte>(0x60);
			pakout.Add<byte>(0x35);
			pakout.Add<byte>(0x08);
			SendPacket(thisclient, &pakout);
		}else if(_strcmpi(command, "item") == 0){
			char* itemId = strtok_s(NULL, " ", &context);
			if(itemId == NULL){
				Log(MSG_DEBUG, "Not enough arguments for &item");
				return true;
			}
			{
				CPacket pakout(0x3001);
				pakout.Add<word>(0x9009);
				pakout.Add<word>(0x9009);
				pakout.Add<word>(strtoul(itemId, NULL, 0));
				pakout.Add<dword>(1);//Item Count
				pakout.Add<word>(0);
				pakout.Add<byte>(1);
				SendPacket(thisclient, &pakout);
			}
			{
				CPacket pakout(0x300A);
				pakout.Add<word>(strtoul(itemId, NULL, 0));
				pakout.Add<dword>(1);//Item Count
				pakout.Add<word>(0x0341);
				SendPacket(thisclient, &pakout);
			}
		}else if(_strcmpi(command, "gmsay") == 0){
			char* text = origText + strlen("&gmsay ");
			CPacket pakout(0x6402);
			pakout.Add<byte>(11);
			pakout.AddStringLen<byte>(text);
			pakout.Add<byte>(0);
			SendPacket(thisclient, &pakout);
		}else if(_strcmpi(command, "rest") == 0){
			char* first = strtok_s(NULL, " ", &context);
			char* second = strtok_s(NULL, " ", &context);
			if (first == NULL || second == NULL) return true;
			CPacket pakout(0x202B);
			pakout.Add<byte>(strtoul(first, NULL, 0));
			pakout.Add<byte>(strtoul(second, NULL, 0));
			SendPacket(thisclient, &pakout);
		}else if(_strcmpi(command, "tele") == 0){
			char* teleX = strtok_s(NULL, " ", &context);
			char* teleY = strtok_s(NULL, " ", &context);
			if(teleX == NULL || teleY == NULL){
				Log(MSG_DEBUG, "Not enough arguments for &tele <teleX> <teleY>");
				return true;
			}
			Log(MSG_DEBUG, "Tele nX: %d nY: %d", strtoul(teleX, NULL, 0), strtoul(teleY, NULL, 0));
			CPacket pakout(0x201b);
			pakout.Add<dword>(strtoul(teleY, NULL, 0));
			pakout.Add<dword>(strtoul(teleX, NULL, 0));
			SendPacket(thisclient, &pakout);
		}else if(_strcmpi(command, "equip") == 0){
			char* itemId = strtok_s(NULL, " ", &context);
			char* slotId = strtok_s(NULL, " ", &context);
			char* refineId = strtok_s(NULL, " ", &context);
			if(itemId == NULL || slotId == NULL){
				Log(MSG_DEBUG, "Not enough arguments for &weapon <weapon id> |<slot id> (12)| |<refine id> (9)|");
				return true;
			}
			//34 02 30 0F 90 0C 08 07 03 00 00 FF FF 00 00 00 00 FF FF 00 00 00 00 FF FF 00 00 00 00 FF FF 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 01 
			{
				CPacket pakout(0x3002);
				pakout.Add<byte>(0x0F);
				pakout.Add<byte>(0x90);

				pakout.Add<byte>(strtoul(slotId, NULL, 0));//'Slot'/'Item Type' 0x0C for Weapon, 0x0A for Shield/Bow
				pakout.Add<word>(strtoul(itemId, NULL, 0));//Item
				pakout.Add<byte>((refineId == NULL)?9:strtoul(refineId, NULL, 0));//Refine
				pakout.Add<word>(0x01);

				for(dword i = 0; i < 4; i++){
					pakout.Add<word>(0xFFFF);
					pakout.Add<dword>(0x0);
				}

				pakout.Fill<byte>(0, 0x11);
				pakout.Add<byte>(0x01);
				SendPacket(thisclient, &pakout);
			}
			{
				//0C 80 0F 90 FF FF 
				CPacket pakout(0x3001);
				pakout.Add<byte>(0x0C);
				pakout.Add<byte>(0x80);
				pakout.Add<byte>(0x0F);
				pakout.Add<byte>(0x90);
				pakout.Add<byte>(0xFF);
				pakout.Add<byte>(0xFF);
				SendPacket(thisclient, &pakout);
			}
		}
	}else{
		CPacket pakout(0x2002);
		pakout.Add<word>(thisclient->clientid);
		pakout.Add<byte>(chatLen);
		pakout.Add<byte>(':');
		pakout.AddBytes(pak->Buffer() + pak->Pos(), chatLen);
		SendPacket(thisclient, &pakout);
	}
	return true;
}

const static byte packet1802[236] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0a, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x05, 0x00, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x04, 0x00, 
	0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x12, 0x72, 0x00, 0x04, 0x00, 0x00, 0x00, 0x04, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x2e, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0x00, 
	0x00, 0x00, 0x0b, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1e, 0x25, 0x00, 0x00, 0xb9, 0x0c, 0x00, 0x00 
};

PACKETHANDLER(pakUserLogin){
	char charname[0x13];
	charname[0x12] = 0;
	memcpy(charname, pak->Buffer() + 5, 0x12);

	thisclient->charname = db->MakeSQLSafe(charname);
	thisclient->loginid = pak->Get<word>(0x03, 0);
	thisclient->clientid = 0x3713;//so its 13 37 in packet logs ;)
	if(_strcmpi(thisclient->charname, charname)){
		Log(MSG_DEBUG, "MySql Safe login %s != %s", thisclient->charname, charname);
		return false;
	}

	Log(MSG_DEBUG, "Charname: %s, Unique Id: %d", thisclient->charname, thisclient->loginid);

	MYSQL_RES* result = db->DoSQL("SELECT u.`id`,u.`username`,u.`loginid`,u.`accesslevel`,u.`lastslot`,c.`id`,c.`level`,c.`profession`,c.`ismale`,c.`map`,c.`hair`,c.`haircolor`,c.`face` FROM `users` AS `u`, `characters` AS `c` WHERE c.`charname`='%s' AND u.`username`=c.`owner`", thisclient->charname);
	if(!result || mysql_num_rows(result) != 1){
		Log(MSG_DEBUG, "SELECT returned bollocks");
		goto authFail;
	}
	
	MYSQL_ROW row = mysql_fetch_row(result);
	if(atoi(row[2]) != thisclient->loginid){
		Log(MSG_DEBUG, "Incorrect loginid");
		thisclient->id = -1;
		goto authFail;
	}

	thisclient->id = atoi(row[0]);
	thisclient->username = row[1];
	thisclient->accesslevel = atoi(row[3]);
	thisclient->lastslot = atoi(row[4]);
	thisclient->charid = atoi(row[5]);

	if(thisclient->accesslevel < 1){
		Log(MSG_DEBUG, "thisclient->accesslevel < 1");
		thisclient->id = -1;
		goto authFail;
	}

	{	// Char Info
		CPacket pakout(0x1038);
			pakout.Add<word>(thisclient->charid);
			pakout.Add<word>(0x000f);
			pakout.AddFixLenStr(thisclient->charname, 0x10);
			pakout.Add<byte>(thisclient->lastslot); // Slot
			pakout.Add<byte>(atoi(row[6])); // Level
			pakout.Add<qword>(0x00); // Total Exp
			pakout.Add<dword>(0x00); // Unk - Doesn't appear to change anything
			pakout.Add<word>(0x000f); // HP Stones
			pakout.Add<word>(0x000b); // SP Stones
			pakout.Add<dword>(0x002E); // HP
			pakout.Add<dword>(0x0020); // SP
			pakout.Add<dword>(0x0000); // Fame
			pakout.Add<qword>(0x1d1a); // Money
			pakout.AddFixLenStr(row[9], 0x0C); // Cur map
			pakout.Add<dword>(0x251e); // Pos X 9502
			pakout.Add<dword>(0x0cb9); // Pos Y 3257
			pakout.Add<byte>(0x5a); // Starting Rotation
			pakout.Add<byte>(0x01); // STR+
			pakout.Add<byte>(0x02); // END+
			pakout.Add<byte>(0x03); // DEX+
			pakout.Add<byte>(0x04); // INT+
			pakout.Add<byte>(0x05); // SPR+

			//  If you see anything ingame that matches any of these numbers, update this
			pakout.Add<byte>(0x06);
			pakout.Add<byte>(0x07);
			pakout.Add<dword>(0x00); // Kill Points
			pakout.Add<byte>(0x0c);
			pakout.Add<byte>(0x0d);
			pakout.Add<byte>(0x0e);
			pakout.Add<byte>(0x0f);
			pakout.Add<byte>(0x10);
			pakout.Add<byte>(0x11);
			pakout.Add<byte>(0x12);
		SendPacket(thisclient, &pakout);
	}

	{	// Look info
		CPacket pakout(0x1039);
			pakout.Add<byte>(0x01 | (atoi(row[7]) << 2) | (atoi(row[8]) << 7)); // Class
			pakout.Add<byte>(atoi(row[10])); // Hair
			pakout.Add<byte>(atoi(row[11])); // Hair Color
			pakout.Add<byte>(atoi(row[12])); // Face
		SendPacket(thisclient, &pakout);
	}

	{	// Quests?
		CPacket pakout(0x103a);
			pakout.Add<word>(thisclient->charid);
			pakout.Add<word>(0x0f);
			pakout.Add<byte>(0x01);
			pakout.Add<byte>(0x00); // Count
		SendPacket(thisclient, &pakout);
	}

	{	// Basic Action? <-- hell no.
		CPacket pakout(0x103b);
			pakout.Add<word>(thisclient->charid);
			pakout.Add<word>(0x0f);
			pakout.Add<word>(0x00);
		SendPacket(thisclient, &pakout);
	}

	{	// Active Skill list
		CPacket pakout(0x103d);
		pakout.Add<byte>(0x00); // Unk
		pakout.Add<word>(thisclient->charid); // Char ID
		pakout.Add<word>(0x000f); // Unk
		pakout.Add<word>(0x01); // Num of skills
		{ // For num of skills
			pakout.Add<word>(0x18D8); // Skill ID [Inferno01]
			pakout.Add<word>(0x0000); // unk
			pakout.Add<word>(0x0000); // Unk
			pakout.Add<word>(0x5432); // Empowerment (dmg | (sp << 4) | (time << 8) | (cool << 12)
			pakout.Add<word>(0x000F); // Uses
			pakout.Add<word>(0x0000); // Unk
		}
		SendPacket(thisclient, &pakout);
	}

	{	// Passive skill list
		CPacket pakout(0x103e);
			pakout.Add<word>(0x00);
		SendPacket(thisclient, &pakout);
	}

	{	// Inventory
		CPacket pakout(0x1047);
			pakout.Add<byte>(0x00); // Count
			pakout.Add<byte>(0x09); // Type
		SendPacket(thisclient, &pakout);
	}

	{	// Equips
		CPacket pakout(0x1047);
			pakout.Add<byte>(0x00); // Count
			pakout.Add<byte>(0x08); // Type
		SendPacket(thisclient, &pakout);
	}

	{	// House
		CPacket pakout(0x1047);
			pakout.Add<byte>(0x01); // Count
			pakout.Add<byte>(0x0C); // Type
			pakout.Add<byte>(0x08); // Unk
			pakout.Add<byte>(0x00); // Unk
			pakout.Add<byte>(0xC0); // Unk
			{ // For Count
				pakout.Add<word>(31170); // ItemID [Liberty House]
				pakout.Add<byte>(0xFF); // Quantity
				pakout.Add<byte>(0xEC); // Unk
				pakout.Add<byte>(0xBB); // Slot
				pakout.Add<byte>(0x76); // Sep
			}
		SendPacket(thisclient, &pakout);
	}

	{	// Titles
		CPacket pakout(0x1049);
			pakout.Add<byte>(0x0d);//Current Title
			pakout.Add<byte>(0x03);//Current Title#
			pakout.Add<word>(0x0000);//u wut
			pakout.Add<word>(0x0001); // Count
			pakout.Add<word>(0xC30D); //titleID | level << 8 | 0x8000 | 0x4000 (if not visible)
		SendPacket(thisclient, &pakout);
	}

	{
		CPacket pakout(0x104A);
			pakout.Add<word>(0x00);
		SendPacket(thisclient, &pakout);
	}

	{
		CPacket pakout(0x1048);
			pakout.Add<word>(0xFF);
		SendPacket(thisclient, &pakout);
	}

	{
		CPacket pakout(0x1802);
			pakout.Add<word>(thisclient->clientid);
			pakout.AddBytes((byte*)packet1802, 236);
		SendPacket(thisclient, &pakout);
	}

	db->QFree(result);
	return true;
authFail:
	{
		CPacket pakout(0x0C09);
		pakout.Add<word>(0x44);
		SendPacket(thisclient, &pakout);
	}
	db->QFree(result);
	return true;
}

void CGameServer::ReceivedISCPacket( CISCPacket* pak ){
	Log(MSG_INFO,"Received ISC Packet: Command: %04x Size: %04x", pak->Command(), pak->Size());
	switch(pak->Command()){
		case TITAN_ISC_SETISCID:
		{
			ServerData.iscid = pak->Read<word>();
		}
		break;
		case TITAN_ISC_IDENTIFY:
		case TITAN_ISC_REMOVE:
		case TITAN_ISC_UPDATEUSERCNT:
		break;
	}
}
