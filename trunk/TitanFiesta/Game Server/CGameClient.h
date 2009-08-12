/* Copyright (C) 2009 TitanFiesta Dev Team
 * Licensed under GNU GPL v3
 * For license details, see LICENCE in the root folder. */
#pragma once

#include "main.h"
#include "CCharacter.h"
#include "../Common/CItems.hpp"

class CMap; // Forward decleration

class CGameClient : public CTitanClient
{	
public:
	CGameClient( ) {
		Username[0x11] = 0;
		Id = -1;
		AccessLevel = -1;
		Inventory = NULL;
		Equipment = NULL;

	}
	~CGameClient( ){
		if(Inventory != NULL) delete Inventory;
		if(Equipment != NULL) delete Equipment;
	}

	dword xorTableLoc;
	char Username[0x12];
	int Id;
	int AccessLevel;
	byte LastSlot;
	int LoginId;

	CItemManager* Inventory;
	CItemManager* Equipment;
	CMap* Map;

	CCharacter Character;

	// Mutex used so CMap doesn't do anything while GameServer is.
	ReadWriteMutex rwmAccess;
private:
};