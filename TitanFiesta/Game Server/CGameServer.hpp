#define PACKETHANDLER(func) bool CGameServer::func( CGameClient* thisclient, CTitanPacket* pak )
#define PACKETRECV(func) func(thisclient, pak)

#include "..\Common\CShn.hpp"
#include "..\Common\CItems.hpp"

class CGameClient : public CTitanClient
{	
public:
	CGameClient( ) {
		username = NULL;
		password = NULL;
		id = -1;
		accesslevel = -1;
		Inventory = NULL;
		Equipment = NULL;
		
		level = 0;
		profession = 0;
		gender = 0;
		facestyle = 0;
		hairstyle = 0;
		haircolor = 0;
		emote = -1;

	}
	~CGameClient( ){
		if(Inventory != NULL) delete Inventory;
		if(Equipment != NULL) delete Equipment;
		if(username != NULL)
			free(username);
		DELARR(password);
	}

	dword xorTableLoc;
	char* username;
	char* password;
	char* charname;
	int id;
	dword charid;
	int accesslevel;
	byte lastslot;
	int loginid;
	word clientid;

	byte level;
	byte profession;
	byte gender;
	byte facestyle;
	byte hairstyle;
	byte haircolor;
	byte emote;

	CItemManager* Inventory;
	CItemManager* Equipment;

	dword newX;
	dword newY;
	dword curX;
	dword curY;
private:
};

class CGameServer : public CTitanServer
{
public:
	CGameServer( ) { }
	~CGameServer( ) { };

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
};