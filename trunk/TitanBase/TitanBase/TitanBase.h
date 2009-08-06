/* Copyright (C) 2008, 2009 TitanFiesta Dev Team
 * Licensed under GNU GPL v3
 * For license details, see LICENCE in the root folder. */

#ifdef TITAN_USING_ISC
class CISCPacket : public CTitanPacket
{
private:
public:
	CISCPacket(dword command = 0){
		_Command = command;
		_Size = 4;
		_CurPos = 4;
		//_Buffer = (byte*)malloc(DEFAULT_PACKET_SIZE);
		Set<word>(_Size,0,0);
		Set<word>(_Command,0,2);
	}
	/*CISCPacket(byte* myBuffer,bool iAmHereToShowThatMyBufferIsAPointerNotAdWord){
		_Buffer = myBuffer;
		_CurPos = 0;
		_Size = Read<word>();
		_Command = Read<word>();
	}*/
};
#endif

class CTitanSocket
{
private:

public:
	CTitanSocket( SOCKET thisSocket = INVALID_SOCKET, bool active = false ) : mySocket(thisSocket), isActive(active) {}
	~CTitanSocket( ) {}

	SOCKET mySocket;
	bool isActive;
};

class CTitanClient : public CTitanSocket
{
private:
	
public:
	class CTitanServer* ParentServer;
	CTitanClient( CTitanServer* TitanServer = NULL, dword myThread = 0 );
	~CTitanClient( );

	dword ThreadID;
#ifdef TITAN_USING_PACKETS
	dword curRecv;
#endif
	CTitanPacket* pak;
	byte ip[4];
};

class CTitanClientThread
{
private:

public:
	CTitanClientThread( CTitanServer* TitanServer = NULL, boost::thread* BoostThread  = NULL, dword TId = 0 );
	void Start( );

	class CTitanServer* ParentServer;
	dword Id;
	dword ClientCount;
	boost::thread* MyThread;
};

class CTitanServer : public CTitanSocket
{
private:
	boost::thread* ListenThread;
	
	ReadWriteMutex rwmThreadList;
	std::vector<CTitanClientThread*> ClientThreads;
	
#ifdef TITAN_USING_ISC
	boost::thread* ISCThread;
	SOCKET sckISC;
	void ISCClient();
#endif

	void ServerLoop();
public:
	CTitanServer( );
	~CTitanServer( );
	
	void SendPacket( CTitanClient* thisclient, CTitanPacket* pak );
	void DisconnectClient( CTitanClient* thisclient );

	void RemoveThread( CTitanClientThread* thisthread );

	bool Start();
	void Wait();

	virtual CTitanClient* CreateNewClient( );
	virtual bool OnServerReady() = 0;
	virtual void OnClientConnect( CTitanClient* baseclient ) = 0;
	virtual void OnClientDisconnect( CTitanClient* baseclient ) = 0;
	virtual void OnReceivePacket( CTitanClient* baseclient, CTitanPacket* pak ) = 0;
	virtual void EncryptPacket( CTitanClient* baseclient, CTitanPacket* pak ) = 0;
#ifdef TITAN_USING_STREAMS
	virtual bool DecryptBufferStream( CTitanClient* thisclient, CTitanPacket* pak, dword recvd ) = 0;
#endif
#ifdef TITAN_USING_PACKETS
	virtual bool DecryptBufferHeader( CTitanClient* baseclient, CTitanPacket* pak ) = 0;
	virtual bool DecryptBufferData( CTitanClient* baseclient, CTitanPacket* pak ) = 0;
#endif
#ifdef TITAN_USING_ISC
	virtual void ReceivedISCPacket( CISCPacket* pak ) = 0;
	void SendISCPacket( CISCPacket* pak );
#endif
#ifdef TITAN_USING_CONSOLE_COMMANDS
	virtual void ProcessCommand( string command ) = 0;
#endif
#ifdef TITAN_USING_PROCESS_THREAD
	virtual void ProcessingThread() = 0;
#endif

#ifdef TITAN_USING_MYSQL
	CTitanSQL* db;
#endif

	CTitanConfig Config;
	ReadWriteMutex rwmClientList;
	std::vector<CTitanClient*> ClientList;
};