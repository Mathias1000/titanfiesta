/* Copyright (C) 2008, 2009 TitanFiesta Dev Team
 * Licensed under GNU GPL v3
 * For license details, see LICENCE in the root folder. */

#include "main.h"
#include "CCharServer.hpp"
#include "..\Common\CItems.hpp"

bool CCharServer::OnServerReady(){
	CTitanIniReader ini("CharServer.ini");
	db = new CTitanSQL(ini.GetString("Server","MySQL","127.0.0.1"), ini.GetString("Username","MySQL","root"), ini.GetString("Password","MySQL",""), ini.GetString("Database","MySQL","titanfiesta"));
	if(!db->Connect()){
		 Log(MSG_ERROR, "Failed to connect to MYSQL Server");
		 return false;
	}else{
		 Log(MSG_STATUS, "Connected to MYSQL Server");
	}
	
	itemInfo = new CShn();
	if(!itemInfo->Open("ItemInfo.shn", 0)){
		Log(MSG_ERROR, "Could not open 'ItemInfo.shn'");
		return false;
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
	ServerData.maxusers = ini.GetInt("MaxUsers", "Char Server", 5);

	// Identify with ISC
	{
	CISCPacket pakout(TITAN_ISC_IDENTIFY);
	pakout.Add<CServerData*>( &ServerData );
	SendISCPacket( &pakout );
	}
	// Fetch serverlist from ISC
	{
		CISCPacket pakout(TITAN_ISC_SERVERLIST);
		SendISCPacket( &pakout );
	}

	return true;
}

void CCharServer::OnReceivePacket( CTitanClient* baseclient, CTitanPacket* pak ){
	CCharClient* thisclient = (CCharClient*)baseclient;
	Log(MSG_INFO,"Received packet: Command: %04x Size: %04x", pak->Command(), pak->Size());

	switch(pak->Command()){
		case 0x200c:
			if (thisclient->id != -1)
				PACKETRECV(pakWhisper);
		break;
		 case 0x7002:
			  if(thisclient->id != -1)
				   PACKETRECV(pak7002);
		 break;
		 case 0x7004:
			  if(thisclient->id != -1)
				   PACKETRECV(pak7004);
		 break;
		 case 0x700c:
			  if(thisclient->id != -1)
				   PACKETRECV(pak700c);
		 break;
		 case 0x700e:
			  if(thisclient->id != -1)
				   PACKETRECV(pak700e);
		 break;
		 case 0x700a:
			  if(thisclient->id != -1)
				   PACKETRECV(pak700a);
		 break;
		 case 0x7c06:
			  if(thisclient->id != -1)
				   PACKETRECV(pak7c06);
		 break;
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
		 case 0x1401:
			  if(thisclient->id != -1)
				   PACKETRECV(pakCreateChar);
		 break;
		 case 0x1407:
			  if(thisclient->id != -1)
				   PACKETRECV(pakDeleteChar);
		 break;
		 case 0x1001:
			  if(thisclient->id != -1)
				   PACKETRECV(pakSelectChar);
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

PACKETHANDLER(pak7002) {
	CPacket pakout(0x7003);
	pakout.AddFile("7003.pak");
	SendPacket(thisclient, &pakout);
	return true;
}

PACKETHANDLER(pak7004) {
	CPacket pakout(0x7005);
	pakout.AddFile("7005.pak");
	SendPacket(thisclient, &pakout);
	return true;
}

PACKETHANDLER(pak700c) {
	CPacket pakout(0x700d);
	pakout.AddFile("700d.pak");
	SendPacket(thisclient, &pakout);
	return true;
}

PACKETHANDLER(pak700e) {
	CPacket pakout(0x700f);
	pakout.AddFile("700f.pak");
	SendPacket(thisclient, &pakout);
	return true;
}

PACKETHANDLER(pak700a) {
	CPacket pakout(0x700b);
	pakout.AddFile("700b.pak");
	SendPacket(thisclient, &pakout);
	return true;
}

PACKETHANDLER(pak7c06) {
	CPacket pakout(0x7c07);
	pakout.Add<dword>(0x00000db1);
	SendPacket(thisclient, &pakout);
	return true;
}

PACKETHANDLER(pakWhisper) {
	char* target = (char*)pak->ReadBytes(0x10);
	byte len = pak->Read<byte>();
	char* message = (char*)pak->ReadBytes(len, true);
	CCharClient* targetclient = GetClientByCharname(target);
	delete[] target;

	if (targetclient == NULL) {
		delete[] message;
		Log(MSG_DEBUG, "Invalid whisper target");
		return false;
	}
	pak->Set<word>(0x200f, 1, 0);
	SendPacket(thisclient, pak);

	CPacket pakout(0x200d);
		pakout.AddFixLenStr(thisclient->charname, 0x10);
		pakout.Add<byte>(0x00); // Purple
		pakout.Add<byte>(len);
		pakout.AddBytes((byte*)message, len);
	SendPacket(targetclient, &pakout);
	delete[] message;
	return true;
}

PACKETHANDLER(pakSelectChar){
	byte slot = pak->Get<byte>(0x03, 0);

	db->ExecSQL("UPDATE `users` SET `lastslot`=%d WHERE username='%s'", slot, thisclient->username);

	MYSQL_RES* result = db->DoSQL("SELECT `charname`, `equipment` FROM `characters` WHERE `owner`='%s' AND `slot`=%d", thisclient->username, slot);
	if(!result){
		Log(MSG_DEBUG, "SELECT returned bollocks");
		return false;
	}
	MYSQL_ROW row = mysql_fetch_row(result);
	strncpy_s(thisclient->charname, 0x10, row[0], 0x10);
	thisclient->charname[0x10] = '\0'; // Null terminate, just incase.
	thisclient->lastslot = slot;
	Log(MSG_DEBUG, "User selected character %s", row[0]);
	db->QFree(result);

	if(ServerList.size() == 0){
		Log(MSG_ERROR, "No Game Server found");
		return false;
	}
	if(ServerList.size() > 1)
		Log(MSG_ERROR, "More than one Game Server found, using first one");

	// Set the equipment for this character. Not actually sure if this is needed anywhere.
	thisclient->Equipment->LoadItemDump((byte*)row[1]);

	CPacket pakout(0x1003);
	pakout.AddFixLenStr(ServerList[0]->ip, 0x10);
	pakout.Add<word>(ServerList[0]->port);
	SendPacket(thisclient, &pakout);
	return true;
}

PACKETHANDLER(pakDeleteChar){
	byte slot = pak->Get<byte>(0x03, 0);
	Log(MSG_DEBUG, "Delete character in slot %d", slot);

	// Should we do any checks on this?
	dword del = db->ExecSQL("DELETE FROM `characters` WHERE owner='%s' AND slot=%d", thisclient->username, slot);

	if (del < 1)
		 Log(MSG_DEBUG, "Didn't delete any characters");
	else if (del > 1)
		 Log(MSG_DEBUG, "Deleted more than 1 character");

	CPacket pakout(0x140c);
		 pakout.Add<byte>(slot);
	SendPacket(thisclient, &pakout);
	return true;
}

PACKETHANDLER(pakCreateChar){
	// Ok, so, some odd behaviour. If you have 1 slot free, and try to create a char in it, client doesn't seem to get the packet.
	char name[0x10];
	byte slot = pak->Get<byte>(0x03, 0);
	memcpy(name, pak->Buffer() + 4, 0x10);
	byte isMale = (pak->Get<byte>(0x14, 0) >> 7) & 0x01;
	byte profession = (pak->Get<byte>(0x14, 0) >> 2) & 0x1F;
	byte unk = pak->Get<byte>(0x14, 0) & 0x03;
	byte hairStyle = pak->Get<byte>(0x15, 0);
	byte hairColour = pak->Get<byte>(0x16, 0);
	byte faceStyle = pak->Get<byte>(0x17, 0);
	Log(MSG_DEBUG, "Slot: %d, IsMale: %d, Prof: %d, Hair: %d, Colour: %d, Face: %d", slot, isMale, profession, hairStyle, hairColour, faceStyle);


	MYSQL_RES* result = db->DoSQL("SELECT `slot` FROM `characters` WHERE `owner`='%s' ORDER BY `slot` ASC", thisclient->username);

	// Check 4 character limit
	if (mysql_num_rows(result) >= 4) {
		// Free memory
		db->QFree(result);

		 Log(MSG_DEBUG, "Character count is already >= 4, bad creation - %s", thisclient->username);
		 return false;
	}
	db->QFree(result);
	// Check SQLsafe name?
	char* sqlSafeName = db->MakeSQLSafe(name);
	if(_strcmpi(sqlSafeName, name) != 0){
		 Log(MSG_DEBUG, "MySql Safe char create %s != %s", sqlSafeName, name);
		 free(sqlSafeName);
		 return false;
	}
	// Check if the character already exists
	result = db->DoSQL("SELECT `charname` FROM `characters` WHERE `charname`='%s'", sqlSafeName);
	if(mysql_num_rows(result) > 0){
		// Free memory
		free(sqlSafeName);
		db->QFree(result);

		 Log(MSG_DEBUG, "Character already exists");

		 CPacket pakout(0x1404);
		 pakout.Add<byte>(0x81);
		 pakout.Add<byte>(0x01);
		 SendPacket(thisclient, &pakout);

		 return false;
	}
	db->QFree(result);

	// Insert character into DB, send response packet.
	db->ExecSQL("INSERT INTO `characters` (`owner`,`charname`,`slot`,`profession`,`ismale`,`hair`,`haircolor`,`face`,`equipment`,`inventory`) values \
			    ('%s', '%s', %u, %u, %u, %u, %u, %u, 0x0008, 0x0009)", thisclient->username, sqlSafeName, slot, profession, isMale, hairStyle, hairColour, faceStyle);

	free(sqlSafeName);
	Log(MSG_DEBUG, "Created character with CharId: %d", db->LastInsertId());
	CPacket pakout(0x1406);
	pakout.Add<byte>(0x01); // # of chars
	pakout.Add<dword>(db->LastInsertId()); // Char ID
	pakout.AddFixLenStr(name, 0x10); // Name
	pakout.Add<word>(0x01); // Level
	pakout.Add<byte>(slot); // Slot
	pakout.AddFixLenStr("Rou", 0x0D); // Map name
	pakout.Add<dword>(0x00); // Unk
	pakout.Add<byte>(0x01 | profession << 2 | (isMale << 7)); // Job/Gender/Unk
	pakout.Add<byte>(hairStyle); // Hair style
	pakout.Add<byte>(hairColour); // Hair color
	pakout.Add<byte>(faceStyle); // Face
	pakout.Add<word>(0xffff); // Helmet
	pakout.Add<word>(0xffff); // Weapon
	pakout.Add<word>(0xffff); // Armor
	pakout.Add<word>(0xffff); // Shield
	pakout.Add<word>(0xffff); // Leg Armor
	pakout.Add<word>(0xffff); // Boot
	pakout.Fill<byte>(0xFF, 0x1A); // Equips (None)
	pakout.Add<word>(0xffff); // Pet
	pakout.Add<word>(0x00); // ??
	pakout.Add<byte>(0xf0); // ??
	pakout.Add<dword>(0xffffffff); // ??
	pakout.Fill<byte>(0x00, 0x0C); // ??
	pakout.Add<dword>(0x00); // PosX
	pakout.Add<dword>(0x00); // PosY
	pakout.Add<word>(0x00); // ??
	pakout.Add<word>(0x00); // ??

	SendPacket(thisclient, &pakout);

	return true;
}

PACKETHANDLER(pakUserLogin){
	MYSQL_RES* result = NULL;
	memset(thisclient->username, 0, 0x13);
	memcpy(thisclient->loginid, pak->Data() + 0x12, 0x40);

	// Check to make sure that loginid isn't all 0's, otherwise could be used to exploit.
	{
		byte loginid[0x40] = {0};
		if (!memcmp(loginid, thisclient->loginid, 0x40)) {
			Log(MSG_DEBUG, "Error, player with loginid of NULL connected.");
			goto authFail;
		}
	}

	char* buf = db->MakeSQLSafe((string)thisclient->loginid, 0x40);

	result = db->DoSQL("SELECT `id`,`username`,`accesslevel` FROM `users` WHERE `loginid`='%s'", buf);
	if(!result || mysql_num_rows(result) != 1){
		 Log(MSG_DEBUG, "SELECT returned bollocks");
		free( buf );
		 goto authFail;
	}
	free( buf );
	
	MYSQL_ROW row = mysql_fetch_row(result);

	thisclient->id = atoi(row[0]);
	memcpy(thisclient->username, row[1], strlen(row[1]));
	thisclient->accesslevel = atoi(row[2]);
	printf("User %s with ID %d al %d has logged in\n", thisclient->username, thisclient->id, thisclient->accesslevel);

	if(thisclient->accesslevel < 1){
		 Log(MSG_DEBUG, "thisclient->accesslevel < 1");
		 thisclient->id = -1;
		 goto authFail;
	}
	db->QFree(result);
	
	thisclient->Equipment = new CItemManager(itemInfo, MAXEQSLOT);
	if (thisclient->Equipment == NULL) {
		Log(MSG_ERROR, "Error creating CItemManager Line: %d", __LINE__);
		return false;
	}
	{
		// Update current user count
		ServerData.currentusers++;
		CISCPacket iscp(TITAN_ISC_UPDATEUSERCNT);
		iscp.Add<word>(ServerData.iscid);
		iscp.Add<dword>(ServerData.currentusers);
		SendISCPacket(&iscp);
	}
	SendCharList(thisclient);
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

void CCharServer::SendCharList(CCharClient* thisclient) {
	// Select characters
	MYSQL_RES* result = db->DoSQL("SELECT `id`,`charname`,`level`,`slot`,`map`,`profession`,`ismale`,`hair`,`haircolor`,`face`,`equipment` FROM `characters` WHERE `owner`='%s'", thisclient->username);
	if(!result){
		 Log(MSG_DEBUG, "SELECT returned bollocks");
		 return;
	}
	
	MYSQL_ROW row;
	CPacket pakout(0xC14);
	pakout.Add<word>(*(word*)thisclient->loginid); // Unique ID
	pakout.Add<byte>(mysql_num_rows(result)); // Num of chars
	while (row = mysql_fetch_row(result)) {

		// Load inventory into a local variable for each character.
		CItemManager Equipment(itemInfo, MAXEQSLOT);
		Equipment.LoadItemDump((byte*)row[10]);

		pakout.Add<dword>(atoi(row[0])); // Character ID
		 pakout.AddFixLenStr(row[1], 0x10); // Name
		 pakout.Add<word>(atoi(row[2])); // Level
		 pakout.Add<byte>(atoi(row[3])); // Char slot
		 pakout.AddFixLenStr(row[4], 0x0D); // Current town (map folder name)
		 pakout.Add<dword>(0x00); // Unk [Changes every login]
		 pakout.Add<byte>(0x01 | (atoi(row[5]) << 2) | (atoi(row[6]) << 7));//Prof | Gender
		 pakout.Add<byte>(atoi(row[7]));//Hair Style
		 pakout.Add<byte>(atoi(row[8]));//Hair Colour
		 pakout.Add<byte>(atoi(row[9]));//Face Style
		pakout.Add<word>(Equipment.GetItemId(1)); // Helmet (Hidden)
		pakout.Add<word>(Equipment.GetItemId(12)); // Weapon (Hidden)
		pakout.Add<word>(Equipment.GetItemId(7)); // Armor
		pakout.Add<word>(Equipment.GetItemId(10)); // Shield (Hidden)
		pakout.Add<word>(Equipment.GetItemId(19)); // Pants
		pakout.Add<word>(Equipment.GetItemId(21)); // Boots
		 pakout.Fill<byte>(0xFF, 0x1A);
		pakout.Add<word>(Equipment.GetItemId(28)); // Pet
		pakout.Add<byte>(0x00); // Weapon Refine << 4 | Shield Refine
		pakout.Add<byte>(0x00); // Unknown
		pakout.Add<byte>(0xf0); // Unknown
		 pakout.Add<dword>(0xffffffff);
		pakout.Fill<byte>(0x00, 0x0c);
		pakout.Add<dword>(0x0000); // Pos?
		pakout.Add<dword>(0x0000); // Pos?
		 pakout.Add<word>(0xdb78); // ?
		 pakout.Add<word>(0xc315); // ?
	}
	db->QFree(result);
	SendPacket(thisclient, &pakout);
}

#ifdef TITAN_USING_CONSOLE_COMMANDS
void CCharServer::ProcessCommand( string command ) {
	printf("Command: %s\n", command);
}
#endif

CCharClient* CCharServer::GetClientByCharname(string charname) {
	for (std::vector<CTitanClient*>::iterator i = ClientList.begin(); i != ClientList.end(); i++) {
		CCharClient* c = (CCharClient*)*i;
		if (c->lastslot == -1) continue;
		if (strncmp(c->charname, charname, 0x10)) continue;
		return c;
	}
	return NULL;
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
				   if(dat->iscid == iscid){ 
					   ServerList.erase(itvdata); 
					   delete dat; 
					   break; 
				   }
			  }
			  rwmServerList.releaseWriteLock( );
		 }
		 break;
		 case TITAN_ISC_UPDATEUSERCNT: break;
		case TITAN_ISC_SERVERLIST:
		{
			byte ServerCount = pak->Read<byte>();
			rwmServerList.acquireWriteLock();

			// Clear serverlist.
			for(std::vector<CServerData*>::iterator i = ServerList.begin(); i != ServerList.end(); i++) {
				CServerData* s = *i;
				ServerList.erase( i ); 
				delete s; 
			}
			// Populate. Only accept GameServer that is owned by us.
			Log(MSG_DEBUG, "Acquiring server list");
			for (byte i = 0; i < ServerCount; i++) {
				CServerData* s = pak->Read<CServerData*>();
				if((s->type != 3) || (s->owner != ServerData.id)) {
					delete s;
					continue;
				}
				ServerList.push_back( s );
				Log(MSG_DEBUG, "\t%s %s:%d [%d/%d]", s->name, s->ip, s->port, s->currentusers, s->maxusers);
			}

			rwmServerList.releaseWriteLock();
		}
	}
}
