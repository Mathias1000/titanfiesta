#include "main.h"
#include "CLoginServer.hpp"

bool CLoginServer::OnServerReady(){
	CTitanIniReader ini("LoginServer.ini");
	db = new CTitanSQL(ini.GetString("Server","MySQL","127.0.0.1"), ini.GetString("Username","MySQL","root"), ini.GetString("Password","MySQL",""), ini.GetString("Database","MySQL","rose"));
	if(!db->Connect()){
		Log(MSG_STATUS, "Failed to connect to MYSQL Server");
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
		case 0xC20:
			PACKETRECV(pakTokenLogin);
		break;
		case 0xC0B:
			PACKETRECV(pakJoinServer);
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

PACKETHANDLER(pakJoinServer){
	byte serverId = pak->Read<byte>();
	CServerData* srv = GetServerByID(serverId);
	if(srv == NULL){
		Log(MSG_ERROR, "Invalid server id for joining");
		return false;
	}
	CPacket pakout(0xC0C);
	pakout.Add<byte>(strlen(srv->ip));
	pakout.AddBytes((byte*)srv->ip, strlen(srv->ip));
	pakout.Add<byte>(0);
	pakout.Add<dword>(0);
	pakout.Add<word>(0);
	pakout.Add<word>(srv->port);
	pakout.Add<word>(0x02C2);//Special Login ID
	SendPacket(thisclient, &pakout);
	return true;
}

PACKETHANDLER(pakTokenLogin){
	char token[64];
	char username[0x12];
	char servername[0x12];
	strcpy_s(token, 64, reinterpret_cast<char*>(pak->Buffer() + 3));

	//04 09 0c 44 00 fail login.
	Log(MSG_DEBUG, "Login token: %s", token);

	memset(username, 0, 0x12);
	memcpy(username, "ExJam", strlen("ExJam"));

	CPacket pakout(0xC0A);
	pakout.AddBytes(reinterpret_cast<byte*>(username), 0x12);
	rwmServerList.acquireReadLock();
	pakout.Add<byte>(ServerList.size());
	for(dword i = 0; i < ServerList.size(); i++){
		pakout.Add<byte>(ServerList[i]->id);
		memset(servername, 0, 0x10);
		memcpy(servername, ServerList[i]->name, strlen(ServerList[i]->name));
		pakout.AddBytes(reinterpret_cast<byte*>(servername), 0x10);
		pakout.Add<byte>(ServerList[i]->status);
	}
	rwmServerList.releaseReadLock();
	SendPacket(thisclient, &pakout);

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
