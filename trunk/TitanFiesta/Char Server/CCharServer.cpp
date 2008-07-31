#include "main.h"
#include "CCharServer.hpp"

bool CCharServer::OnServerReady(){
	CTitanIniReader ini("CharServer.ini");
	db = new CTitanSQL(ini.GetString("Server","MySQL","127.0.0.1"), ini.GetString("Username","MySQL","root"), ini.GetString("Password","MySQL",""), ini.GetString("Database","MySQL","titanfiesta"));
	if(!db->Connect()){
		Log(MSG_STATUS, "Failed to connect to MYSQL Server");
	}else{
		Log(MSG_STATUS, "Connected to MYSQL Server");
	}
	
	ServerData.flags = 0;
	ServerData.id = ini.GetInt("Id","Char Server",2);
	ServerData.ip = Config.BindIp;
	ServerData.iscid = 99;
	ServerData.name = ini.GetString("Name","Char Server","Char Server");
	ServerData.owner = ini.GetInt("Owner","Char Server",1);
	ServerData.port = Config.BindPort;
	ServerData.status = 9;
	ServerData.type = 2;

	CISCPacket pakout(TITAN_ISC_IDENTIFY);
	pakout.Add<CServerData*>( &ServerData );
	SendISCPacket( &pakout );

	return true;
}

void CCharServer::OnReceivePacket( CTitanClient* baseclient, CTitanPacket* pak ){
	CCharClient* thisclient = (CCharClient*)baseclient;
	Log(MSG_INFO,"Received packet: Command: %04x Size: %04x", pak->Command(), pak->Size());

	switch(pak->Command()){
		case 0xC0F:
			if(thisclient->id == -1)
				PACKETRECV(pakUserLogin);
		break;
		case 0x80D:
		{
			CPacket pakout(0x80E);
			pakout.Add<byte>(0x11);
			pakout.Add<word>(0x3B2E);
			SendPacket(thisclient, &pakout);
		}
		break;
		case 0x827:
			if(thisclient->id != -1)
				PACKETRECV(pakCharList);
		break;
		case 0x1401:
			if(thisclient->id != -1)
				PACKETRECV(pakCreateChar);
		break;
		case 0x1407:
			if(thisclient->id != -1)
				PACKETRECV(pakDeleteChar);
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

PACKETHANDLER(pakDeleteChar){
	byte slot = pak->Get<byte>(0x03, 0);
	Log(MSG_DEBUG, "Delete character in slot %d", slot);

	// Should we do any checks on this?
	db->DoSQL("DELETE FROM `characters` WHERE owner='%s' AND slot=%d", thisclient->username, slot);

	CPacket pakout(0x140c);
		pakout.Add<byte>(slot);
	SendPacket(thisclient, &pakout);
	return true;
}

PACKETHANDLER(pakCreateChar){
	// Bah, James, you idiot, the slot # is sent with this packet :P Way to ignore it
	// You also got haircolor and facestyle from 0x16 :P
	// Ok, so, some odd behaviour. If you have 1 slot free, and try to create a char in it, client doesn't seem to get the packet.
	char name[0x10];
	memcpy(name, pak->Buffer() + 4, 0x10);
	byte slot = pak->Get<byte>(0x03, 0);
	byte isMale = ((pak->Get<byte>(0x14, 0) & 0x80) != 0);
	//byte profession = (pak->Get<byte>(0x14, 0) >> 2) & 0x1F;
	byte profession = pak->Get<byte>(0x14, 0) & 0x7F;
	byte unk = pak->Get<byte>(0x14, 0) & 0x03;
	byte hairStyle = pak->Get<byte>(0x15, 0);
	byte hairColour = pak->Get<byte>(0x16, 0);
	byte faceStyle = pak->Get<byte>(0x17, 0);
	Log(MSG_DEBUG, "Slot: %d, IsMale: %d, Prof: %d, Hair: %d, Colour: %d, Face: %d", slot, isMale, profession, hairStyle, hairColour, faceStyle);


	MYSQL_RES* result = db->DoSQL("SELECT `slot` FROM `characters` WHERE `owner`='%s' ORDER BY `slot` ASC", thisclient->username);

	// Check 4 character limit
	if (mysql_num_rows(result) >= 4) {
		Log(MSG_DEBUG, "Character count is already >= 4, bad creation - %s", thisclient->username);
		return false;
	}

	char* sqlSafeName = db->MakeSQLSafe(name);
	if(_strcmpi(sqlSafeName, name) != 0){
		Log(MSG_DEBUG, "MySql Safe char create %s != %s", sqlSafeName, name);
		return false;
	}
	// Check if the character already exists
	result = db->DoSQL("SELECT `charname` FROM `characters` WHERE `charname`='%s'", name);
	if(mysql_num_rows(result) > 0){
		Log(MSG_DEBUG, "Character already exists");

		CPacket pakout(0x1404);
		pakout.Add<byte>(0x81);
		pakout.Add<byte>(0x01);
		SendPacket(thisclient, &pakout);

		return false;
	}

	db->DoSQL("INSERT INTO `characters` (`owner`,`charname`,`slot`,`profession`,`ismale`,`hair`,`haircolor`,`face`) values \
			  ('%s', '%s', %u, %u, %u, %u, %u, %u)", thisclient->username, sqlSafeName, slot, profession, isMale, hairStyle, hairColour, faceStyle);

	free(sqlSafeName);

	CPacket pakout(0x1406);
	pakout.Add<byte>(0x01); // # of chars
	pakout.Add<word>(0x0000); // Char ID
	pakout.Add<word>(0x000f); // Str len
	pakout.AddFixLenStr(name, 0x10); // Name
	pakout.Add<word>(0x01); // Level
	pakout.Add<byte>(slot); // Slot
	pakout.AddFixLenStr("Rou", 0x0D); // Map name
	pakout.Add<dword>(0x00); // Unk
	pakout.Add<byte>(profession | (isMale << 7)); // Job/Gender/Unk
	pakout.Add<byte>(hairStyle); // Hair style
	pakout.Add<byte>(hairColour); // Hair color
	pakout.Add<byte>(faceStyle); // Face
	pakout.Fill<byte>(0xFF, 0x26); // Equips (None)
	pakout.Add<word>(0x00); // ??
	pakout.Add<byte>(0xf0); // ??
	pakout.Add<dword>(0xff); // ??
	pakout.Fill<byte>(0x00, 0x0C); // ??
	pakout.Add<dword>(0x00); // Pos?
	pakout.Add<dword>(0x00); // Pos?
	pakout.Add<word>(0x00); // ??
	pakout.Add<word>(0x00); // ??

	SendPacket(thisclient, &pakout);

	return true;
}

PACKETHANDLER(pakCharList){
	// Select characters
	MYSQL_RES* result = db->DoSQL("SELECT `id`,`charname`,`level`,`slot`,`map`,`profession`,`ismale`,`hair`,`haircolor`,`face` FROM `characters` WHERE `owner`='%s'", thisclient->username);
	if(!result){
		Log(MSG_DEBUG, "SELECT returned bollocks");
		return false;
	}
	
	MYSQL_ROW row;
	CPacket pakout(0xC14);

	pakout.Add<word>(0x0000); // Unk [Changes every login]
	pakout.Add<byte>(mysql_num_rows(result)); // Num of chars
	while (row = mysql_fetch_row(result)) {
		pakout.Add<word>(atoi(row[0])); // Character ID
		pakout.Add<word>(0x000f); // Name length (Always 16?)
		pakout.AddFixLenStr(row[1], 0x10); // Name
		pakout.Add<word>(atoi(row[2])); // Level
		pakout.Add<byte>(atoi(row[3])); // Char slot
		pakout.AddFixLenStr(row[4], 0x0D); // Current town (map folder name)
		pakout.Add<dword>(0x00); // Unk [Changes every login]
		pakout.Add<byte>(atoi(row[5]) | (atoi(row[6]) << 7));//Prof | Gender
		pakout.Add<byte>(atoi(row[7]));//Hair Style
		pakout.Add<byte>(atoi(row[8]));//Hair Colour
		pakout.Add<byte>(atoi(row[9]));//Face Style
		// Wiping it to default armor for testing
		pakout.Add<word>(0xFFFF); //Weapon [Not Shown]
		pakout.Add<word>(0xFFFF);//Body Armour
		pakout.Add<word>(0xFFFF); //Shield [Not Shown]
		pakout.Add<word>(0xFFFF); //Leg Armour
		pakout.Add<word>(0xFFFF); //Boot Armour
		pakout.Fill<byte>(0xFF, 0x1C);
		pakout.Add<word>(0x0); // 0x0060 on my main char
		pakout.Add<byte>(0xf0);
		pakout.Add<dword>(0xffffffff);
		pakout.Fill<byte>(0x00, 12);
		pakout.Add<dword>(0x0cdc); // Pos?
		pakout.Add<dword>(0x1bc9); // Pos?
		pakout.Add<word>(0xdb78); // ?
		pakout.Add<word>(0xc315); // ?
	}

	SendPacket(thisclient, &pakout);
	return true;
}

const static byte packet0826[130] = {
		0x80, 0x00, 0xF1, 0xFF, 0x00, 0x00, 0xFF, 0x1F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x63, 0x63, 
		0x20, 0xD5, 
};

PACKETHANDLER(pakUserLogin){
	char username[0x13];
	username[0x12] = 0;
	memcpy(username, pak->Buffer() + 3, 0x12);

	thisclient->username = db->MakeSQLSafe(username);
	thisclient->loginid = pak->Get<word>(0x12);
	if(_strcmpi(thisclient->username, username)){
		Log(MSG_DEBUG, "MySql Safe login %s != %s", thisclient->username, username);
		return false;
	}

	Log(MSG_DEBUG, "Username: %s, Unique Id: %d", thisclient->username, thisclient->loginid);

	MYSQL_RES* result = db->DoSQL("SELECT `id`,`loginid`,`accesslevel` FROM `users` WHERE `username`='%s'", thisclient->username);
	if(!result || mysql_num_rows(result) != 1){
		Log(MSG_DEBUG, "SELECT returned bollocks");
		goto authFail;
	}
	
	MYSQL_ROW row = mysql_fetch_row(result);
	if(atoi(row[1]) != thisclient->loginid){
		Log(MSG_DEBUG, "Incorrect loginid");
		thisclient->id = -1;
		goto authFail;
	}

	thisclient->id = atoi(row[0]);
	thisclient->accesslevel = atoi(row[1]);

	if(thisclient->accesslevel < 1){
		Log(MSG_DEBUG, "thisclient->accesslevel < 1");
		thisclient->id = -1;
		goto authFail;
	}

	{
		CPacket pakout(0x0826);
		pakout.AddBytes((byte*)packet0826, 130);
		SendPacket(thisclient, &pakout);
	}

	return true;
authFail:
	{
		CPacket pakout(0x0C09);
		pakout.Add<word>(0x44);
		SendPacket(thisclient, &pakout);
	}
	return true;
}

void CCharServer::ReceivedISCPacket( CISCPacket* pak ){
	Log(MSG_INFO,"Received ISC Packet: Command: %04x Size: %04x", pak->Command(), pak->Size());
	switch(pak->Command()){
		case TITAN_ISC_SETISCID:
		{
			ServerData.iscid = pak->Read<word>();
		}
		break;
		case TITAN_ISC_IDENTIFY:
		{			
			CServerData* tempData = pak->Read<CServerData*>();
			if((tempData->type != 3) || (tempData->owner != ServerData.id)){
				delete tempData;
				return;
			}

			rwmServerList.acquireWriteLock();
			ServerList.push_back( tempData );
			rwmServerList.releaseWriteLock();
			Log(MSG_INFO,"Channel %s Connected with ISC ID %d", tempData->name, tempData->iscid);
		}
		break;
		case TITAN_ISC_REMOVE:
		{			
			word iscid = pak->Read<word>();
			rwmServerList.acquireWriteLock( );
			for(std::vector<CServerData*>::iterator itvdata = ServerList.begin(); itvdata != ServerList.end(); itvdata++) {
				CServerData* dat = ((CServerData*)*itvdata); 
				if(dat->iscid == iscid){ ServerList.erase(itvdata); delete dat; break; }
			}
			rwmServerList.releaseWriteLock( );
		}
		break;
		case TITAN_ISC_UPDATEUSERCNT:
		break;
	}
}
