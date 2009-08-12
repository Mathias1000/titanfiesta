/* Copyright (C) 2008, 2009 TitanFiesta Dev Team
 * Licensed under GNU GPL v3
 * For license details, see LICENCE in the root folder. */
#pragma once

#define PACKETHANDLER(func) bool CGameServer::func( CGameClient* thisclient, CTitanPacket* pak )
#define PACKETRECV(func) func(thisclient, pak)

#include "..\Common\CShn.hpp"
#include "CGameClient.h"

class CMap; // Forward decleration

class CGameServer : public CTitanServer
{
public:
	CGameServer( ) {
		itemInfo = NULL;
		mapInfo = NULL;
	}
	~CGameServer( );

	CGameClient* CreateNewClient( ){	return new CGameClient();}
	bool OnServerReady( );
	void OnClientConnect( CTitanClient* client ){
		CGameClient* thisclient = (CGameClient*)client;
		thisclient->xorTableLoc = 7;
		CPacket pakout(0x807);
		pakout.Add<word>(thisclient->xorTableLoc);
		SendPacket(client, &pakout);
	}
	void OnClientDisconnect( CTitanClient* baseclient );

	void EncryptPacket( CTitanClient* baseclient, CTitanPacket* pak ){
		if(pak->Size() > 0xFF){
			pak->Set<byte>(0, 0, 0);
			memcpy(pak->Buffer() + 3, pak->Buffer() + 1, pak->Size() - 1);
			pak->Size(pak->Size() + 2);
			pak->Set<word>(pak->Size() -  3, 1, 0);
		}else{
			pak->Set<byte>(pak->Size() - 1,0,0);
		}
	}
	bool DecryptBufferHeader( CTitanClient* baseclient, CTitanPacket* pak ){
		if(pak->Get<byte>(0,0) == 0){
			pak->Size(pak->Get<word>(1,0) + 3);
			pak->HeaderSize(PACKET_HEADER_SIZE + 2);
		}else{
			pak->Size(pak->Get<byte>(0,0) + 1);
		}
		return true;
	}
	bool DecryptBufferData( CTitanClient* baseclient, CTitanPacket* pak ){
		CGameClient* thisclient = (CGameClient*)baseclient;
		dword decStart = 1;
		if(pak->Get<byte>(0,0) == 0){
			pak->Size(pak->Get<word>(1,0) + 3);
			pak->HeaderSize(PACKET_HEADER_SIZE + 2);
			decStart = 3;
		}else{
			pak->Size(pak->Get<byte>(0,0) + 1);
		}
		byte* buffer = pak->Buffer();
		for(dword i = decStart; i < pak->Size(); i++){
			buffer[i] ^= xorTable[thisclient->xorTableLoc];
			thisclient->xorTableLoc++;
			if(thisclient->xorTableLoc == 0x1f3) thisclient->xorTableLoc = 0;
		}
		pak->Command(pak->Get<word>(decStart, 0));
		pak->Pos(decStart + 2);
		return true;
	}

	void ReceivedISCPacket( CISCPacket* pak );
	void OnReceivePacket( CTitanClient* thisclient, CTitanPacket* pak );

	PACKETHANDLER(pakUserLogin);
	PACKETHANDLER(pakClientReady);
	PACKETHANDLER(pakChat);
	PACKETHANDLER(pakShout);
	PACKETHANDLER(pakRest);
	PACKETHANDLER(pakMove);
	PACKETHANDLER(pakEndRest);
	PACKETHANDLER(pakMoveInvItem);
	PACKETHANDLER(pakSetTitle);
	PACKETHANDLER(pakBasicAction);
	PACKETHANDLER(pakEquipInvItem);
	PACKETHANDLER(pakEquipInvItemSlot);
	PACKETHANDLER(pakUnequipInvItem);

private:
	CServerData ServerData;

	CTitanSQL* db;
	CShn* itemInfo;
	CShn* mapInfo;

	// Map threads
	std::map<word, CMap*> Maps; // Links mapid -> CMap instance.
	std::vector<CMap*> MapList; // Used for quick-access to all maps.
	boost::thread_group thrGrpMaps;
};
