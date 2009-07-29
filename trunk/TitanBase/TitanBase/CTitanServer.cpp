#include <main.h>

CTitanServer::CTitanServer(){
	Config.BindPort = 0;
	Config.BindIp = NULL;
#ifdef TITAN_USING_ISC
	Config.ISCIp = TITAN_DEFAULT_ISC_IP;
	Config.ISCPort = TITAN_DEFAULT_ISC_PORT;
#endif
	WSADATA wsa;
	WSAStartup( MAKEWORD(2,2), &wsa );

	#ifdef TITAN_USING_MYSQL
	db = new CTitanSQL();
	#endif
}

CTitanServer::~CTitanServer(){
	WSACleanup();
}

bool CTitanServer::Start(){
	sockaddr_in ain;

	if( isActive ){
		Log(MSG_ERROR,"Server already started!");
		return false;
	}

	mySocket = socket( AF_INET, SOCK_STREAM, 0 );
	if(mySocket == INVALID_SOCKET){
		Log(MSG_ERROR, "Could not create valid listen socket (WSK2 ERROR: %i)", WSAGetLastError() );
		return false;
	}

#ifdef TITAN_USING_ISC
	sckISC = socket( AF_INET, SOCK_STREAM, 0 );
	if (sckISC == INVALID_SOCKET) {
		Log(MSG_ERROR, "Could not create valid ISC socket (WSK2 ERROR: %i)", WSAGetLastError( ) );
		return false;
	}
#endif

	if(Config.BindPort == 0){
		Log(MSG_ERROR, "Binding Port was not set.");
		return false;
	}

	if(Config.BindIp == NULL){
		Log(MSG_ERROR, "Binding IP was not set.");
		return false;
	}
	
	ain.sin_family = AF_INET;
	if(Config.BindIp == NULL){
		ain.sin_addr.s_addr	= INADDR_ANY;
	}else{
		ain.sin_addr.s_addr = inet_addr(Config.BindIp);
	}
	ain.sin_port = htons(Config.BindPort);
	if(bind(mySocket, (const sockaddr*)&ain, sizeof(ain)) == SOCKET_ERROR){
		Log(MSG_ERROR, "Could not bind listen socket (WSK2 ERROR: %i)", WSAGetLastError());
		return false;
	}

#ifdef TITAN_USING_ISC
	ain.sin_family		= AF_INET;
	ain.sin_addr.s_addr	= inet_addr(Config.ISCIp);
	ain.sin_port		= htons(Config.ISCPort);
	if(connect(sckISC, (const sockaddr*)&ain, sizeof(ain)) == SOCKET_ERROR) {
		Log(MSG_ERROR, "Could not connect to ISC (WSK2 ERROR: %i)", WSAGetLastError());
		return false;
	}
#endif

	if(listen(mySocket, SOMAXCONN) == -1 ) {
		Log(MSG_ERROR, "Could not listen on socket (WSK2 ERROR: %i)", WSAGetLastError());
		return false;
	}

	isActive = true;

	if(!OnServerReady()){
		Log(MSG_ERROR, "OnServerReady() Returned False" );
		return false;
	}

#ifdef TITAN_USING_ISC
	ISCThread = new boost::thread( boost::bind( &CTitanServer::ISCClient, this ) );
#endif

	ListenThread = new boost::thread( boost::bind( &CTitanServer::ServerLoop, this ) );

	Wait();
	return true;
}

void CTitanServer::Wait(){
#ifdef TITAN_USING_CONSOLE_COMMANDS
	char line[255];
	while(std::cin.getline(line, 255)){
		if(strlen(line) > 0){
			ProcessCommand( line );
		}
	}
#elif defined(TITAN_USING_SHOW_STATUS)
	dword time = 0;
	while(true){
		Sleep(1000);
		time	+= 1;
		if(((float)time/TITAN_SHOW_STATUS_DELAY) == (float)((int)time/TITAN_SHOW_STATUS_DELAY)){
			Log(MSG_STATUS, "Up Time: %ds, Threads: %d, Clients: %d", time, ClientThreads.size(), ClientList.size() );
		}
	}
#elif defined(TITAN_USING_PROCESS_THREAD)
	ProcessingThread();
#else
	while(true) Sleep(1000);
#endif
}

void CTitanServer::ServerLoop(){
	sockaddr client_info;
	int client_info_len = sizeof( sockaddr_in );
	SOCKET NewSocket = INVALID_SOCKET;
	CTitanClient* NewClient;

	Log(MSG_STATUS, "Server Started");
	while(true){
		while( NewSocket == INVALID_SOCKET ){
			NewSocket = accept( mySocket, (sockaddr*)&client_info, &client_info_len );
		}
		
		CTitanClientThread* ClientThread = NULL;

		rwmThreadList.acquireReadLock( );
		for(dword i = 0; i < ClientThreads.size(); i++){
			if(ClientThreads[i]->ClientCount < TITAN_CLIENTS_PER_THREAD){
				ClientThread = ClientThreads[i];
				break;
			}
		}
		rwmThreadList.releaseReadLock( );

		bool newThread = false;
		if(ClientThread == NULL){
			//Create a thread!
			ClientThread = new CTitanClientThread( this );

			ClientThread->Id = ClientThreads.size();
			newThread = true;
		}
		ClientThread->ClientCount++;

		NewClient = CreateNewClient();
		NewClient->ParentServer = this;
		NewClient->ThreadID = ClientThread->Id;
		NewClient->isActive = true;
		NewClient->mySocket = NewSocket;
		memcpy(&NewClient->ip,&client_info.sa_data[2],4);

		rwmClientList.acquireWriteLock( );
		ClientList.push_back( NewClient );
		rwmClientList.releaseWriteLock( );

		OnClientConnect( NewClient );
		Log(MSG_DEBUG, "%d.%d.%d.%d added to thread! (TID: %i, CC: %i / %i)", NewClient->ip[0], NewClient->ip[1], NewClient->ip[2], NewClient->ip[3], ClientThread->Id, ClientThread->ClientCount, TITAN_CLIENTS_PER_THREAD );	

		if(newThread){
			rwmThreadList.acquireWriteLock( );
			ClientThread->MyThread = new boost::thread(boost::bind(&CTitanClientThread::Start, ClientThread ));
			ClientThreads.push_back(ClientThread);
			rwmThreadList.releaseWriteLock( );
		}

		NewSocket = INVALID_SOCKET;
		NewClient = NULL;
	}
}

void CTitanServer::RemoveThread( CTitanClientThread* thisthread )
{
	rwmThreadList.acquireWriteLock( );	
	for(std::vector<CTitanClientThread*>::iterator itvdata = ClientThreads.begin(); itvdata != ClientThreads.end(); itvdata++) {
		if( thisthread == *itvdata ) { ClientThreads.erase( itvdata ); break; }
	}
	for(dword i = 0; i < ClientThreads.size(); i++){
		for(dword j = 0; j < ClientList.size(); j++){
			if(ClientList[j]->ThreadID == ClientThreads[i]->Id){
				ClientList[j]->ThreadID = i;
			}
		}
		ClientThreads[i]->Id = i;
	}
	rwmThreadList.releaseWriteLock( );
	Log(MSG_DEBUG, "Removed Thread [TID: %d]", thisthread->Id);

	delete thisthread->MyThread;
	delete thisthread;
}

void CTitanServer::DisconnectClient( CTitanClient* thisclient )
{
	rwmClientList.acquireWriteLock( );
	for(std::vector<CTitanClient*>::iterator itvdata = ClientList.begin(); itvdata != ClientList.end(); itvdata++) {
		if( thisclient == *itvdata ) { ClientList.erase( itvdata ); break; }
	}
	rwmClientList.releaseWriteLock( );

	if(thisclient->ThreadID != ClientThreads.size() - 1){
		CTitanClient* otherclient = NULL;
		rwmClientList.acquireReadLock( );
		for(dword i = 0; i < ClientList.size(); i++){
			otherclient = ClientList[i];
			if(otherclient->ThreadID == (ClientThreads.size() - 1)){
				break;
			}
		}
		rwmClientList.releaseReadLock( );

		rwmThreadList.acquireWriteLock( );
		if(otherclient != NULL){
			ClientThreads[otherclient->ThreadID]->ClientCount--;
			Log(MSG_DEBUG,"Client moved from thread %d to %d",otherclient->ThreadID,thisclient->ThreadID);

			otherclient->ThreadID = thisclient->ThreadID;
		}else{
			ClientThreads[thisclient->ThreadID]->ClientCount--;
		}
		rwmThreadList.releaseWriteLock( );
	}else{
		rwmThreadList.acquireWriteLock( );
		ClientThreads[thisclient->ThreadID]->ClientCount--;
		rwmThreadList.releaseWriteLock( );
	}

	OnClientDisconnect( thisclient );
}

CTitanClient* CTitanServer::CreateNewClient( )
{
	return new CTitanClient( );
}

void CTitanServer::SendPacket( CTitanClient* thisclient, CTitanPacket* pak )
{
	EncryptPacket( thisclient, pak );
	send( thisclient->mySocket, (string)pak->Buffer(), pak->Size(), 0 );
}

#ifdef TITAN_USING_ISC
void CTitanServer::SendISCPacket( CISCPacket* pak ){
	pak->Set<word>(pak->Size(), 0, 0);
	pak->Set<word>(pak->Command(), 2, 0);
	send( sckISC, (string)pak->Buffer(), pak->Size(), 0 );
}

void CTitanServer::ISCClient()
{
	dword recvd = 0;
	dword bufsize = 0;
	dword readlen = 4;
	Log(MSG_DEBUG, "ISC Communication Active");
	CISCPacket thispak;

	do {
		Sleep( 1 );

		recvd = recv( sckISC, reinterpret_cast<char*>(thispak.Buffer() + bufsize), readlen-bufsize, 0 );

		if( recvd == 0 || recvd == SOCKET_ERROR ) {				
			Log(MSG_ERROR, "Disconnected from ISC");
			exit(-1);
			break;
		}

		bufsize += recvd;
		if( bufsize != readlen ) continue;
		if( bufsize == 4 ) {
			readlen = thispak.Get<word>(0,0);
			if( readlen < 4 ) Log(MSG_DEBUG, "Invalid Packet Header");
			if( readlen > 4 ) continue;
		}
		thispak.Size(thispak.Get<word>(0,0));
		thispak.Command(thispak.Get<word>(2,0));
		thispak.Pos(4);
		ReceivedISCPacket( &thispak );

		bufsize = 0;
		readlen = 4;
	} while ( 1 );

	exit(-1);
}
#endif