#define PACKETHANDLER(func) bool CTitanISC::func( CISCClient* thisclient, CTitanPacket* pak )
#define PACKETRECV(func) func(thisclient, pak)

class CISCClient : public CTitanClient
{	
public:
	CISCClient( ):servername(NULL),address(NULL),port(0){ }
	~CISCClient( ){
		if(servername != NULL)
			free(servername);
		DELARR(address);
	}

	char* servername;
	char* address;
	int port;
	word iscid;
private:
};

class CTitanISC : public CTitanServer
{
public:
	CTitanISC( ) { }
	~CTitanISC( ) { };

	CISCClient* CreateNewClient( ){	return new CISCClient();}
	bool OnServerReady( );
	void OnClientConnect( CTitanClient* client ){
		CISCClient* thisclient = (CISCClient*)client;
	}
	void OnClientDisconnect( CTitanClient* thisclient );
	void EncryptPacket( CTitanClient* baseclient, CTitanPacket* pak ){
		pak->Set<word>(pak->Size(), 0, 0);
	};
	bool DecryptBufferHeader( CTitanClient* baseclient, CTitanPacket* pak ){
		pak->Size(pak->Get<word>(0, 0));
		pak->Command(pak->Get<word>(0, 2));
		return true;
	};
	bool DecryptBufferData( CTitanClient* baseclient, CTitanPacket* pak ){
		pak->Size(pak->Get<word>(0, 0));
		pak->Command(pak->Get<word>(0, 2));
		pak->Pos(4);
		return true;
	};
	void OnReceivePacket( CTitanClient* thisclient, CTitanPacket* pak );

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
};
