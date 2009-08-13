/* Copyright (C) 2009 TitanFiesta Dev Team
 * Licensed under GNU GPL v3
 * For license details, see LICENCE in the root folder. */
#include "CGameClient.h"
#include "CMap.h"

CMap::CMap(CShnRow* MapInfo, CTitanServer* Server) {
	this->Server = Server;
	this->MapInfo = MapInfo;

	Id = MapInfo->cells[0]->wData;
	MapName = (const char*)MapInfo->cells[1]->strData;
	Name = (const char*)MapInfo->cells[2]->strData;
	StartX = MapInfo->cells[3]->dwData;
	StartY = MapInfo->cells[4]->dwData;
	KingdomMap = MapInfo->cells[5]->btData;
	Inside = MapInfo->cells[7]->btData;


	Tick = 0;
	Running = true;
	NextEID = 100;
}

CMap::~CMap() {

}

bool CMap::AddClient(CGameClient *Client) {
	Client->Map = this;
	rwmClientList.acquireWriteLock();
	ClientList.push_back(Client);
	// Send this client all Entity information.
	// Setup this clients ident packet
	CPacket pakident(0x1c06);
		pakident.Add<word>(Client->Character.ClientId);
		pakident.AddFixLenStr(Client->Character.Name, 0x10);
		pakident.Add<dword>(Client->Character.Pos.X);
		pakident.Add<dword>(Client->Character.Pos.Y);
		pakident.Add<byte>(0);
		pakident.Add<byte>(0x01); // State
		pakident.Add<byte>(Client->Character.Job);
		pakident.Add<byte>(Client->Character.Profession);
		pakident.Add<byte>(Client->Character.Hair.Style);
		pakident.Add<byte>(Client->Character.Hair.Color);
		pakident.Add<byte>(Client->Character.Face.Style);
		pakident.Add<word>(Client->Equipment->GetItemId(1)); // Helmet
		pakident.Add<word>(Client->Equipment->GetItemId(12)); // Weapon
		pakident.Add<word>(Client->Equipment->GetItemId(7)); // Armor
		pakident.Add<word>(Client->Equipment->GetItemId(10)); // Shield
		pakident.Add<word>(Client->Equipment->GetItemId(19)); // Pants
		pakident.Add<word>(Client->Equipment->GetItemId(21)); // Boots
		pakident.Fill<byte>(0xff, 0x1A);
		pakident.Add<word>(Client->Equipment->GetItemId(28)); // Pet
		pakident.Add<byte>(Client->Equipment->GetRefine(12) << 4 | 
						   Client->Equipment->GetRefine(10)); // Refine
		pakident.Add<word>(0x00);
		pakident.Add<byte>(0x00);
		pakident.Add<word>(0xffff);
		pakident.Add<byte>(Client->Character.Emote);
		pakident.Add<word>(0xffff);
		pakident.Add<byte>(0);
		pakident.Add<byte>(0);
		pakident.Add<word>(0);
		pakident.Fill<byte>(0x00, 0x28);
		pakident.Add<dword>(0x00);
		pakident.Add<word>(0x0002);

	// Send Player List
	for (std::vector<CGameClient*>::iterator i = ClientList.begin(); i != ClientList.end(); i++) {
		CGameClient* c = *i;
		if (c == NULL) continue;
		CPacket pakout(0x1c06);
		pakout.Add<word>(c->Character.ClientId);
		pakout.AddFixLenStr(c->Character.Name, 0x10);
		pakout.Add<dword>(c->Character.Pos.X);
		pakout.Add<dword>(c->Character.Pos.Y);
		pakout.Add<byte>(0);
		pakout.Add<byte>(0x01); // State
		pakout.Add<byte>(c->Character.Job);
		pakout.Add<byte>(c->Character.Profession);
		pakout.Add<byte>(c->Character.Hair.Style);
		pakout.Add<byte>(c->Character.Hair.Color);
		pakout.Add<byte>(c->Character.Face.Style);
		pakout.Add<word>(c->Equipment->GetItemId(1)); // Helmet
		pakout.Add<word>(c->Equipment->GetItemId(12)); // Weapon
		pakout.Add<word>(c->Equipment->GetItemId(7)); // Armor
		pakout.Add<word>(c->Equipment->GetItemId(10)); // Shield
		pakout.Add<word>(c->Equipment->GetItemId(19)); // Pants
		pakout.Add<word>(c->Equipment->GetItemId(21)); // Boots
		pakout.Fill<byte>(0xff, 0x1A);
		pakout.Add<word>(c->Equipment->GetItemId(28)); // Pet
		pakout.Add<byte>(c->Equipment->GetRefine(12) << 4 | 
						 c->Equipment->GetRefine(10)); // Refine
		pakout.Add<byte>(0x00); // Refine
		pakout.Add<word>(0x00);
		pakout.Add<byte>(0x00);
		pakout.Add<word>(0xffff);
		pakout.Add<byte>(c->Character.Emote);
		pakout.Add<word>(0xffff);
		pakout.Add<byte>(0);
		pakout.Add<byte>(0);
		pakout.Add<word>(0);
		pakout.Fill<byte>(0x00, 0x28);
		pakout.Add<dword>(0x00);
		pakout.Add<word>(0x0002);
		//ClientCount++;

		Server->SendPacket(Client, &pakout); // Send client to thisclient
		Server->SendPacket(c, &pakident); // Send this client to other client
	}
	//pakout.Set<byte>(ClientCount, 0);
	//Server->SendPacket(Client, &pakout);
	// Send NPC List

	// Send Mob List

	// Send Drop List

	rwmClientList.releaseWriteLock();
	return true;
}

bool CMap::RemoveClient(CGameClient *Client) {
	rwmClientList.acquireWriteLock();
	CPacket pakout(0x1c0e);
	pakout.Add<word>(Client->Character.ClientId);

	// Store the client to erase
	std::vector<CGameClient*>::iterator Erase;
	for (std::vector<CGameClient*>::iterator i = ClientList.begin(); i != ClientList.end(); i++) {
		CGameClient* c = *i;
		if (c == Client) {
			Erase = i;
			continue;
		}
		// Remove the client from others screen.
		Server->SendPacket(c, &pakout);
	}
	ClientList.erase(Erase);
	rwmClientList.releaseWriteLock();

	Client->Map = NULL;
	return false;
}

void CMap::Thread() {
	// Thread "process" function
	while (Running) {
		if (Tick == 0) {
			rwmClientList.acquireReadLock();
			if (ClientList.size() != 0)
				printf("%s -> %d\n", MapInfo->cells[6]->strData, ClientList.size());
			rwmClientList.releaseReadLock();
		}
		++Tick = Tick % 1000;

		// Sleep so as to not take all CPU power.
		boost::this_thread::sleep(boost::posix_time::milliseconds(10));
	}
}

void CMap::SendToAll(CPacket* pak) {
	rwmClientList.acquireReadLock();
	for (std::vector<CGameClient*>::iterator i = ClientList.begin(); i != ClientList.end(); i++) {
		Server->SendPacket(*i, pak);
	}
	rwmClientList.releaseReadLock();
}

word CMap::GetNewEID(){
	word NewEID;
	if (EIDRecycle.empty()){
		NewEID = NextEID;
		if (NextEID != MAX_EID)
			NextEID++;
	}
	else{
		NewEID = EIDRecycle.top();
		EIDRecycle.pop();
	}
	return NewEID;
}

void CMap::FreeEID(word EID){
	EIDRecycle.push(EID);
}