#define PACKETHANDLER(func) bool CLoginServer::func( CLoginClient* thisclient, CTitanPacket* pak )
#define PACKETRECV(func) func(thisclient, pak)

class CLoginClient : public CTitanClient
{	
public:
	CLoginClient( ):username(NULL),password(NULL),id(-1),accesslevel(-1){ }
	~CLoginClient( ){
		if(username != NULL)
			free(username);
		DELARR(password);
	}

	dword xorTableLoc;
	char* username;
	char* password;
	int id;
	int accesslevel;
	byte loginid[0x40];
private:
};

class CLoginServer : public CTitanServer
{
public:
	CLoginServer( ) { }
	~CLoginServer( ) { };

	CLoginClient* CreateNewClient( ){	return new CLoginClient();}
	bool OnServerReady( );
	void OnClientConnect( CTitanClient* client ){
		CLoginClient* thisclient = (CLoginClient*)client;
		thisclient->xorTableLoc = 7;
		CPacket pakout(0x807);
		pakout.Add<word>(thisclient->xorTableLoc);
		SendPacket(client, &pakout);
	}
	void OnClientDisconnect( CTitanClient* thisclient ) {}

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
		CLoginClient* thisclient = (CLoginClient*)baseclient;
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

	PACKETHANDLER(pakTokenLogin);
	PACKETHANDLER(pakUserLogin);
	PACKETHANDLER(pakJoinServer);
	PACKETHANDLER(pakPing);

	CServerData* GetServerByID(dword id){		
		CServerData* retServ = NULL;
		rwmServerList.acquireReadLock();
		for(dword i = 0; i < ServerList.size(); i++){
			if(ServerList[i]->id != id) continue;
			retServ = ServerList[i];
			break;
		}
		rwmServerList.releaseReadLock();

		return retServ;
	}

private:
	std::vector<CServerData*> ServerList;
	ReadWriteMutex rwmServerList;
	CServerData ServerData;

	CTitanSQL* db;
};
