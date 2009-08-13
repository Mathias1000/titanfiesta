/* Copyright (C) 2009 TitanFiesta Dev Team
 * Licensed under GNU GPL v3
 * For license details, see LICENCE in the root folder. */
#pragma once

#include <vector>
#include <stack>
#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>

#include "main.h" // Includes all titan crap.
#include "../../TitanBase/TitanBase/CReadWriteMutex.hpp"
#include "../Common/CShn.hpp"

const word MAX_EID = 0xffff; //maximum value that can be allocated for entity id + 1

class CGameClient; // Forward decleration.

class CMap {
public:
	CMap(CShnRow* MapInfo, CTitanServer* Server);
	~CMap();

	void Thread();
	void EndThread() {Running = false;};
	bool RemoveClient(CGameClient* Client);
	bool AddClient(CGameClient* Client);

	void SendToAll(CPacket* pak);

	dword GetStartY() {
		return StartY;
	}
	dword GetStartX() {
		return StartX;
	}
	const char* GetMapName() {
		return MapName;
	}
	const char* GetName() {
		return Name;
	}
	byte GetKingdomMap() {
		return KingdomMap;
	}
	byte GetInside() {
		return Inside;
	}

	word GetNewEID(); //returns unused entity id or MAX_EID if all are allocated
	void FreeEID(word EID); //this function should be called when an entity is deleted
							//to free its entity id

private:
	CTitanServer* Server;

	std::vector<CGameClient*> ClientList;

	CShnRow* MapInfo;
	word Id;
	const char* MapName;
	const char* Name;
	dword StartX;
	dword StartY;
	byte KingdomMap;
	byte Inside;

	ReadWriteMutex rwmClientList;

	dword Tick;
	bool Running;

	word NextEID; //highest entity id unallocated since server start
	std::stack<word> EIDRecycle; //EIDs that were used but can be reallocated now
};