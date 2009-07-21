#include "main.h"
#include "CLoginServer.hpp"
#include "../Common/md5.hpp"

CRandomMersenne rg((int32)time(0));

bool CLoginServer::OnServerReady(){
	CTitanIniReader ini("LoginServer.ini");
	db = new CTitanSQL(ini.GetString("Server","MySQL","127.0.0.1"), ini.GetString("Username","MySQL","root"), ini.GetString("Password","MySQL",""), ini.GetString("Database","MySQL","titanfiesta"));
	if(!db->Connect()){
		 Log(MSG_ERROR, "Failed to connect to MYSQL Server");
		 return false;
	}else{
		Log(MSG_STATUS, "Connected to MYSQL Server");
	}

	ServerData.flags = 0;
	ServerData.id = ini.GetInt("Id","Login Server",1);
	ServerData.ip = Config.BindIp;
	ServerData.iscid = 99;
	ServerData.name = ini.GetString("Name","Login Server","Login Server");
	ServerData.owner = ini.GetInt("Owner","Login Server",0);
	ServerData.port = Config.BindPort;
	ServerData.status = 0;
	ServerData.type = 1;

	CISCPacket pakout(TITAN_ISC_IDENTIFY);
	pakout.Add<CServerData*>( &ServerData );
	SendISCPacket( &pakout );

	return true;
}

void CLoginServer::OnReceivePacket( CTitanClient* baseclient, CTitanPacket* pak ){
	CLoginClient* thisclient = (CLoginClient*)baseclient;

	switch(pak->Command()){
		case 0xC01:
		{
			CPacket pakout(0xC03);
			pakout.Add<word>(0x01);
			SendPacket(thisclient, &pakout);
		}
		break;
		case 0x0C04: // pakServerAck
			if (thisclient->id != -1)
				printf("Ack: %s\n", pak->Buffer() + 4);
		break;
		case 0x0C06: // pakUserLogin
			if(thisclient->id == -1)
				PACKETRECV(pakUserLogin);
		break;
		case 0xC0B:
			if(thisclient->id != -1)
				PACKETRECV(pakJoinServer);
		break;
		case 0x0C18: // User disconnecting from server list.
			if (thisclient->id != -1)
				printf("User %s disconnected.  Status: %d\n", thisclient->username, pak->Read<byte>());
		break;
		case 0xC1B:
			PACKETRECV(pakPing);
		break;
		case 0x0C20:
			if(thisclient->id == -1)
				PACKETRECV(pakTokenLogin);
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

PACKETHANDLER(pakPing){
	/*CPacket pakout(0xC1C);
	pakout.AddFixLenStr("TitanFiesta", 0x12);
	rwmServerList.acquireReadLock();
	pakout.Add<byte>(ServerList.size());
	for(dword i = 0; i < ServerList.size(); i++){
		pakout.Add<byte>(ServerList[i]->id);
		pakout.AddFixLenStr(ServerList[i]->name, 0x10);
		pakout.Add<byte>(ServerList[i]->status);
	}
	rwmServerList.releaseReadLock();
	SendPacket(thisclient, &pakout);*/
	Log(MSG_DEBUG, "Client wants updates server status!");
	return true;
}

PACKETHANDLER(pakJoinServer){
	byte serverId = pak->Read<byte>();
	CServerData* srv = GetServerByID(serverId);
	if(srv == NULL){
		Log(MSG_ERROR, "Invalid server id for joining");
		return false;
	}
	// New client requires 64-byte unique ID
	for (int i = 0; i < 16; i++) {
		*((dword*)thisclient->loginid + i) = rg.BRandom();
	}
	char* buf = db->EncodeBinary(thisclient->loginid, 0x40);
	Log(MSG_DEBUG, "LoginID: %s", buf);
	db->ExecSQL("UPDATE `users` SET `loginid`='%s' WHERE `id`='%d'", buf, thisclient->id);
	delete[] buf;

	CPacket pakout(0xC0C);
	pakout.Add<byte>(srv->status); // Server Status
	pakout.AddFixLenStr(srv->ip, 0x10); // IP
	pakout.Add<word>(srv->port); // Port
	pakout.AddBytes(thisclient->loginid, 0x40);//Special Login ID
	SendPacket(thisclient, &pakout);
	return true;
}

// USA Client Login
PACKETHANDLER(pakTokenLogin){
	if (SERVERTYPE != USASERVER) {
		Log(MSG_DEBUG, "USA client tried connecting");
		return false;
	}

	char md5hash[33];
	char username[0x12];
	md5hash[32] = 0;
	memcpy(md5hash, pak->Buffer() + 3, 32);
	strcpy_s(username, 0x12, (const char*)pak->Buffer() + 3 + 32);

	thisclient->username = db->MakeSQLSafe(username);
	thisclient->password = _strdup(md5hash);
	if(_strcmpi(thisclient->username, username)){
		Log(MSG_DEBUG, "MySql Safe login %s != %s", thisclient->username, username);
		return false;
	}
	MYSQL_RES* result = db->DoSQL("SELECT `id`,`password`,`accesslevel` FROM `users` WHERE `username`='%s'", thisclient->username);
	if(!result || mysql_num_rows(result) != 1){
		Log(MSG_DEBUG, "SELECT returned bollocks");
		goto authFail;
	}
	
	MYSQL_ROW row = mysql_fetch_row(result);
	if(strcmp(row[1], (const char*)thisclient->password) != 0){
		Log(MSG_DEBUG, "Incorrect password");
		goto authFail;
	}

	thisclient->id = atoi(row[0]);
	thisclient->accesslevel = atoi(row[2]);

	if(thisclient->accesslevel < 1){
		Log(MSG_DEBUG, "thisclient->accesslevel < 1");
		thisclient->id = -1;
		goto authFail;
	}

	Log(MSG_DEBUG, "User %s authenticated", username);

	{
		CPacket pakout(0xC0A);
		rwmServerList.acquireReadLock();
		pakout.Add<byte>(ServerList.size());
		dword serverCount = 0;
		for(dword i = 0; i < ServerList.size(); i++){
			if (ServerList[i]->type != 2) continue;
			pakout.Add<byte>(ServerList[i]->id);
			pakout.AddFixLenStr(ServerList[i]->name, 0x10);
			pakout.Add<byte>(ServerList[i]->status);
			serverCount++;
		}
		rwmServerList.releaseReadLock();
		pakout.Set<byte>(serverCount, 0x00);
		SendPacket(thisclient, &pakout);
	}

	/*
	STATUS
	0 = offline
	1 = maitanence
	2 = off
	3 = off
	4 = off
	5 = high
	6 = low
	7 = low
	8 = low
	9 = low
	10 = medium
	*/
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

// Europe Client Login
PACKETHANDLER(pakUserLogin){
	if (SERVERTYPE != EURSERVER) {
		Log(MSG_DEBUG, "Europe client tried connecting");
		return false;
	}

	char username[0x12];
	char password[0x10];
	char md5hash[0x21];
	md5hash[0x20] = '\0';

	memcpy(username, pak->Buffer() + 3, 0x12);
	memcpy(password, pak->Buffer() + 3 + 0x12, 0x10);
	// Generate MD5 sum from password. This makes this function compatible with pakTokenLogin.
	{
		struct cvs_MD5Context context;
		unsigned char checksum[16];
		cvs_MD5Init(&context);
		cvs_MD5Update(&context, (unsigned const char*)password, strlen(password));
		cvs_MD5Final(checksum, &context);
		for (int i = 0; i < 16; i++)
			sprintf_s(md5hash + (i*2), 3, "%02x", checksum[i]);
	}

	thisclient->username = db->MakeSQLSafe(username);
	thisclient->password = _strdup(md5hash);
	if(_strcmpi(thisclient->username, username)){
		Log(MSG_DEBUG, "MySql Safe login %s != %s", thisclient->username, username);
		return false;
	}
	MYSQL_RES* result = db->DoSQL("SELECT `id`,`password`,`accesslevel` FROM `users` WHERE `username`='%s'", thisclient->username);
	if(!result || mysql_num_rows(result) != 1){
		Log(MSG_DEBUG, "SELECT returned bollocks");
		goto authFail;
	}
	
	MYSQL_ROW row = mysql_fetch_row(result);
	if(strcmp(row[1], (const char*)thisclient->password) != 0){
		Log(MSG_DEBUG, "Incorrect password");
		goto authFail;
	}

	thisclient->id = atoi(row[0]);
	thisclient->accesslevel = atoi(row[2]);

	if(thisclient->accesslevel < 1){
		Log(MSG_DEBUG, "thisclient->accesslevel < 1");
		thisclient->id = -1;
		goto authFail;
	}

	Log(MSG_DEBUG, "User %s authenticated", username);

	{
		CPacket pakout(0xC0A);
		rwmServerList.acquireReadLock();
		pakout.Add<byte>(ServerList.size());
		dword serverCount = 0;
		for(dword i = 0; i < ServerList.size(); i++){
			if (ServerList[i]->type != 2) continue;
			pakout.Add<byte>(ServerList[i]->id);
			pakout.AddFixLenStr(ServerList[i]->name, 0x10);
			pakout.Add<byte>(ServerList[i]->status);
			serverCount++;
		}
		rwmServerList.releaseReadLock();
		pakout.Set<byte>(serverCount, 0x00);
		SendPacket(thisclient, &pakout);
	}

	/*
	STATUS
	0 = offline
	1 = maitanence
	2 = off
	3 = off
	4 = off
	5 = high
	6 = low
	7 = low
	8 = low
	9 = low
	10 = medium
	*/
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


void CLoginServer::ReceivedISCPacket( CISCPacket* pak ){
	Log(MSG_INFO,"Received ISC Packet: Command: %04x Size: %04x", pak->Command(), pak->Size());
	switch(pak->Command()){
		case TITAN_ISC_IDENTIFY:
		{
			CServerData* tempData = pak->Read<CServerData*>();
			rwmServerList.acquireWriteLock();
			ServerList.push_back( tempData );
			rwmServerList.releaseWriteLock();
			Log(MSG_INFO,"Server %s Connected with ISC ID %d", tempData->name, tempData->iscid);
		}
		break;
		case TITAN_ISC_SETISCID:
		{
			ServerData.iscid = pak->Read<word>();
			Log(MSG_DEBUG, "Set ISCID to %d", ServerData.iscid);
		}
		break;
		case TITAN_ISC_REMOVE:
		{
			word iscid = pak->Read<word>();
			rwmServerList.acquireWriteLock( );
			for(std::vector<CServerData*>::iterator itvdata = ServerList.begin(); itvdata != ServerList.end(); itvdata++) {
				CServerData* dat = ((CServerData*)*itvdata); 
				if(dat->iscid == iscid){ 
					Log(MSG_INFO, "Server %s with ISC ID %d has disconnected", dat->name, dat->iscid);
					ServerList.erase(itvdata); 
					delete dat; 
					break; 
				}
			}
			rwmServerList.releaseWriteLock( );
		}
		break;
		case TITAN_ISC_UPDATEUSERCNT:
		{
			word iscid = pak->Read<word>();
			rwmServerList.acquireWriteLock();
			for(dword i=0; i<ServerList.size(); i++) {
				CServerData* dat = ServerList.at(i);
				if(!dat) continue;
				if(dat->iscid == iscid){ 
					dat->currentusers = pak->Read<dword>();
					break; 
				}
			}
			rwmServerList.releaseWriteLock();			
		}
		break;
	}
}
