#include "main.h"
#include "CTitanISC.hpp"

bool CTitanISC::OnServerReady(){
	ServerData.flags = 0;
	ServerData.id = 0;
	ServerData.ip = Config.BindIp;
	ServerData.name = "TitanISC";
	ServerData.owner = 0;
	ServerData.port = Config.BindPort;
	ServerData.status = 0;
	ServerData.type = 0;

	return true;
}

void CTitanISC::OnReceivePacket( CTitanClient* baseclient, CTitanPacket* pak ){
	CISCClient* thisclient = (CISCClient*)baseclient;

	Log(MSG_DEBUG, "OnReceivePacket CMD: %04d", pak->Command());

	switch(pak->Command()){
		case TITAN_ISC_IDENTIFY:
		{
			CServerData* tempData = pak->Read<CServerData*>();

			rwmServerList.acquireReadLock();
			if (ServerList.size() != 0) {
				CServerData* lastServer = *(ServerList.end() - 1);
				tempData->iscid = lastServer->iscid + 1;
			} else {
				tempData->iscid = 0;
			}
			rwmServerList.releaseReadLock();
			thisclient->iscid = tempData->iscid;
			pak->Set<word>(tempData->iscid, 1);
			Log(MSG_DEBUG, "Server '%s' [%s:%d][%d] Identified", tempData->name, tempData->ip, tempData->port, tempData->iscid);

			rwmServerList.acquireWriteLock();
			ServerList.push_back( tempData );
			rwmServerList.releaseWriteLock();

			CPacket pakout(TITAN_ISC_SETISCID);
			pakout.Add<word>(tempData->iscid);
			SendPacket(thisclient, &pakout);
		}
		break;
		case TITAN_ISC_SETISCID: break;
		case TITAN_ISC_REMOVE: break;
		case TITAN_ISC_UPDATEUSERCNT:
		{
			rwmServerList.acquireWriteLock();
			word iscid = pak->Read<word>();
			for (std::vector<CServerData*>::iterator i = ServerList.begin(); i != ServerList.end(); i++) {
				CServerData* s = *i;
				if (s->iscid == iscid) {
					s->currentusers = pak->Read<dword>();
					break;
				}
			}
			rwmServerList.releaseWriteLock();
		}
		break;
		case TITAN_ISC_SERVERLIST:
		{
			CPacket pakout(TITAN_ISC_SERVERLIST);
			rwmServerList.acquireReadLock();
			pakout.Add<byte>(ServerList.size());

			for (std::vector<CServerData*>::iterator i = ServerList.begin(); i != ServerList.end(); i++) {
				CServerData* s = *i;
				pakout.Add<CServerData*>(s);
			}

			rwmServerList.releaseReadLock();
			SendPacket(thisclient, &pakout);
			return; // We don't want to forward this packet.
		}
		break;
		default:
			Log(MSG_DEBUG, "Invalid ISC Command");
		break;
	}

	// Forward packets.
	rwmClientList.acquireReadLock();
	for (unsigned int i = 0; i < ClientList.size(); i++) {
		CTitanClient *c = ClientList.at(i);
		if (c == baseclient) continue;
		SendPacket(c, pak);
	}
	rwmClientList.releaseReadLock();
}

void CTitanISC::OnClientDisconnect( CTitanClient* baseclient ){
	CISCClient* thisclient = (CISCClient*)baseclient;

	CPacket pakout(TITAN_ISC_REMOVE);
	pakout.Add<word>(thisclient->iscid);

	// Remove Server from list
	rwmServerList.acquireWriteLock();
	std::vector<CServerData*>::iterator i;
	for (i = ServerList.begin(); i != ServerList.end(); i++) {
		CServerData* s = *i;
		if (s->iscid == thisclient->iscid) {
			ServerList.erase(i);
			delete s;
			break;
		}
	}
	rwmServerList.releaseWriteLock();

	// Send packet to remaining servers.
	rwmClientList.acquireReadLock();
	for (dword i = 0; i < ClientList.size(); i++) {
		CISCClient* c = (CISCClient*)ClientList.at(i);
		if (c == baseclient) continue;
		SendPacket(c, &pakout);
	}
	rwmClientList.releaseReadLock();
}