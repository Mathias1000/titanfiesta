class CConnectClient {
public:
	CConnectClient():isRunning(true),finished(false){}

	void ClientRecvThread(){
		char buffer[0x8000];
		dword recvd = 0;
		dword bufsize = 0;
		dword readlen = 1;
		bool isWordSize = false;
		fd_set fds;
		int activity;
		timeval timeout = {0,0};

		do {
			Sleep(10);

			FD_ZERO(&fds);
			FD_SET(cliSocket, &fds);
			activity = select( 0, &fds, NULL, NULL, &timeout );
			if(activity == 0) continue;
			if(activity <= 0) Log(MSG_DEBUG, "%s Client: Unable to select sockets! (WSK2 ERROR: %i)", GetSrvName(), WSAGetLastError());

			recvd = recv(cliSocket, buffer + bufsize, readlen-bufsize, 0);

			if(recvd == 0 || recvd == SOCKET_ERROR){
				Log(MSG_ERROR, "%s Client: Recv Error recvd=%i (WSK2 ERROR: %i)", GetSrvName(), recvd, WSAGetLastError());
				goto ended;
			}

			bufsize += recvd;
			if(isWordSize){
				readlen = *reinterpret_cast<word*>(buffer + 1) + 3;
				isWordSize = false;
			}
			if(bufsize != readlen) continue;
			if(bufsize == 1){
				readlen = *reinterpret_cast<byte*>(buffer) + 1;
				if(readlen == 1){
					readlen = 3;
					isWordSize = true;
				}
				if(readlen < 1){ bufsize = 0; readlen = 1; continue; }
				if(readlen > 1) continue;
			}

			ReceivedClientPacket(buffer, readlen);

			bufsize = 0;
			readlen = 1;
		}while(isRunning);

ended:
		closesocket(cliSocket);
		if(isRunning){
			isRunning = false;
			thrSrvRecv->join();
			finished = true;
		}
	}

	void ServerRecvThread(){		
		srvSocket = socket(AF_INET, SOCK_STREAM, 0);
		if(srvSocket == INVALID_SOCKET){
			Log(MSG_ERROR, "Could not create valid %s server socket(WSK2 ERROR: %i)", GetSrvName(), WSAGetLastError());
			isRunning = false;
			thrCliRecv->join();
			return;
		}

		sockaddr_in ain;
		ain.sin_family = AF_INET;
		ain.sin_addr.s_addr	= inet_addr(connectIP);
		ain.sin_port = htons(connectPort);
		if(connect(srvSocket, (const sockaddr*)&ain, sizeof(ain)) == SOCKET_ERROR){
			Log(MSG_ERROR, "%s Server: Could not connect to %s:%i (WSK2 ERROR: %i)", GetSrvName(), connectIP, connectPort, WSAGetLastError());
			isRunning = false;
			thrCliRecv->join();
			return;
		}

		char buffer[0x8000];
		dword recvd = 0;
		dword bufsize = 0;
		dword readlen = 1;
		bool isWordSize = false;
		fd_set fds;
		int activity;
		timeval timeout = {0,0};

		do{
			Sleep(10);

			FD_ZERO(&fds);
			FD_SET(srvSocket, &fds);
			activity = select( 0, &fds, NULL, NULL, &timeout );
			if(activity == 0) continue;
			if(activity <= 0) Log(MSG_DEBUG, "%s Server: Unable to select sockets! (WSK2 ERROR: %i)", GetSrvName(), WSAGetLastError());
			recvd = recv( srvSocket, buffer + bufsize, readlen-bufsize, 0 );
			
			if(recvd == 0 || recvd == SOCKET_ERROR){
				Log(MSG_ERROR, "%s Server: Recv Error recvd=%i (WSK2 ERROR: %i)", GetSrvName(), recvd, WSAGetLastError());
				goto ended;
			}

			bufsize += recvd;
			if(isWordSize){
				readlen = *reinterpret_cast<word*>(buffer + 1) + 3;
				isWordSize = false;
			}
			if(bufsize != readlen) continue;
			if(bufsize == 1){
				readlen = *reinterpret_cast<byte*>(buffer) + 1;
				if(readlen == 1){
					readlen = 3;
					isWordSize = true;
				}
				if(readlen < 1){ bufsize = 0; readlen = 1; continue; }
				if(readlen > 1) continue;
			}

			ReceivedServerPacket( buffer, readlen );

			bufsize = 0;
			readlen = 1;
		}while(isRunning);

ended:
		closesocket(srvSocket);
		if(isRunning){
			isRunning = false;
			thrCliRecv->join();
			finished = true;
		}
	}

	void SendServerPacket(CPacket* pak){
		LogPacket(pak, GetSrvName()[0], '>');
		cliOutCrypt.CryptPacket(pak);
		send(srvSocket, reinterpret_cast<char*>(pak->buffer), pak->size, 0);
	}

	void SendClientPacket(CPacket* pak){
		LogPacket(pak, GetSrvName()[0], '<');
		send(cliSocket, reinterpret_cast<char*>(pak->buffer), pak->size, 0);
	}

	const char* GetSrvName(){
		switch(srvType){
			case 0:
				return "Login";
			case 1:
				return "World";
			case 2:
				return "Game";
		}
		return "Unknown";
	}

protected:
	void ReceivedClientPacket(char* buffer, dword length){
		CPacket pak(buffer, length);
		cliInCrypt.CryptPacket(&pak);
		pak.GetFromBuffer();
		if(srvType == 0){
			ReceivedLoginClientPacket(&pak, this);
		}else if(srvType == 1){
			ReceivedWorldClientPacket(&pak, this);
		}else if(srvType == 2){
			ReceivedGameClientPacket(&pak, this);
		}
		SendServerPacket(&pak);
	}

	void ReceivedServerPacket(char* buffer, dword length){
		CPacket pak(buffer, length);
		if(pak.command == 0x0807){
			word cryptLoc = pak.Read<word>();
			cliInCrypt.SetCryptLocation(cryptLoc);
			cliOutCrypt.SetCryptLocation(cryptLoc);
		}
		if(srvType == 0){
			ReceivedLoginServerPacket(&pak, this);
		}else if(srvType == 1){
			ReceivedWorldServerPacket(&pak, this);
		}else if(srvType == 2){
			ReceivedGameServerPacket(&pak, this);
		}
		SendClientPacket(&pak);
	}

protected:
	boost::thread* thrCliRecv;
	boost::thread* thrSrvRecv;

	SOCKET cliSocket;
	SOCKET srvSocket;

	CFiestaCrypt cliInCrypt;
	CFiestaCrypt cliOutCrypt;

	char* connectIP;
	int connectPort;

	int srvType;

	volatile bool isRunning;
	volatile bool finished;

	friend class CListener;
};

class CListener {
public:
	CListener(){}
	~CListener(){}

	bool Start(char* loginIP, int loginPort){
		sockaddr_in ain;
		u_long argp = 1L;

		strcpy_s(this->loginIP, 0x10, loginIP);
		this->loginPort = loginPort;

		sckListenLogin = socket(AF_INET, SOCK_STREAM, 0);
		if(sckListenLogin == INVALID_SOCKET){
			Log(MSG_ERROR, "Could not create valid Login listen socket (WSK2 ERROR: %i)", WSAGetLastError());
			return false;
		}

		if(ioctlsocket(sckListenLogin, FIONBIO, (u_long FAR *)&argp) == SOCKET_ERROR){
			Log(MSG_ERROR, "Could not set Login socket to non-blocking (WSK2 ERROR: %i)", WSAGetLastError());
			return false;
		}

		sckListenWorld = socket(AF_INET, SOCK_STREAM, 0);
		if(sckListenWorld == INVALID_SOCKET){
			Log(MSG_ERROR, "Could not create valid World listen socket (WSK2 ERROR: %i)", WSAGetLastError());
			return false;
		}

		if(ioctlsocket(sckListenWorld, FIONBIO, (u_long FAR *)&argp) == SOCKET_ERROR){
			Log(MSG_ERROR, "Could not set World socket to non-blocking (WSK2 ERROR: %i)", WSAGetLastError());
			return false;
		}

		sckListenGame = socket(AF_INET, SOCK_STREAM, 0);
		if(sckListenGame == INVALID_SOCKET){
			Log(MSG_ERROR, "Could not create valid Game listen socket (WSK2 ERROR: %i)", WSAGetLastError());
			return false;
		}

		if(ioctlsocket(sckListenGame, FIONBIO, (u_long FAR *)&argp) == SOCKET_ERROR){
			Log(MSG_ERROR, "Could not set Game socket to non-blocking (WSK2 ERROR: %i)", WSAGetLastError());
			return false;
		}

		ain.sin_family = AF_INET;
		ain.sin_addr.s_addr = inet_addr("127.0.0.1");
		ain.sin_port = htons(9010);

		if(bind(sckListenLogin, (const sockaddr*)&ain, sizeof(ain)) == SOCKET_ERROR){
			Log(MSG_ERROR, "Could not bind Login listen socket (WSK2 ERROR: %i)", WSAGetLastError());
			return false;
		}

		if(listen(sckListenLogin, SOMAXCONN) == -1 ) {
			Log(MSG_ERROR, "Could not listen on Login socket (WSK2 ERROR: %i)", WSAGetLastError());
			return false;
		}

		ain.sin_port = htons(9110);
		if(bind(sckListenWorld, (const sockaddr*)&ain, sizeof(ain)) == SOCKET_ERROR){
			Log(MSG_ERROR, "Could not bind World listen socket (WSK2 ERROR: %i)", WSAGetLastError());
			return false;
		}

		if(listen(sckListenWorld, SOMAXCONN) == -1 ) {
			Log(MSG_ERROR, "Could not listen on World socket (WSK2 ERROR: %i)", WSAGetLastError());
			return false;
		}

		ain.sin_port = htons(9210);
		if(bind(sckListenGame, (const sockaddr*)&ain, sizeof(ain)) == SOCKET_ERROR){
			Log(MSG_ERROR, "Could not bind Game listen socket (WSK2 ERROR: %i)", WSAGetLastError());
			return false;
		}

		if(listen(sckListenGame, SOMAXCONN) == -1 ) {
			Log(MSG_ERROR, "Could not listen on Game socket (WSK2 ERROR: %i)", WSAGetLastError());
			return false;
		}

		thrListen = new boost::thread(boost::bind(&CListener::ListenThread, this));
		
		thrListen->join();

		return true;
	}

	void ListenThread(){
		sockaddr client_info;
		int client_info_len = sizeof( sockaddr_in );
		SOCKET NewSocket = INVALID_SOCKET;
		CConnectClient* NewClient;
		int srvType = 0;

		Log(MSG_STATUS, "Proxy Started, waiting for connections");
		while(true){
			while(NewSocket == INVALID_SOCKET){
				Sleep(25);
				srvType = 0;NewSocket = accept(sckListenLogin, (sockaddr*)&client_info, &client_info_len);if(NewSocket != INVALID_SOCKET) break;
				Sleep(25);
				srvType = 1;NewSocket = accept(sckListenWorld, (sockaddr*)&client_info, &client_info_len);if(NewSocket != INVALID_SOCKET) break;
				Sleep(25);
				srvType = 2;NewSocket = accept(sckListenGame, (sockaddr*)&client_info, &client_info_len);if(NewSocket != INVALID_SOCKET) break;
				Sleep(25);
				CheckClientList();
			}
			
			NewClient = new CConnectClient();
			NewClient->srvType = srvType;

			switch(srvType){
				case 0:
					NewClient->connectIP = loginIP;
					NewClient->connectPort = loginPort;
				break;
				case 1:
					NewClient->connectIP = worldIP;
					NewClient->connectPort = worldPort;
				break;
				case 2:
					NewClient->connectIP = gameIP;
					NewClient->connectPort = gamePort;
				break;
			}

			Log(MSG_DEBUG, "New %s client connected!", NewClient->GetSrvName());

			NewClient->cliSocket = NewSocket;

			NewClient->thrCliRecv = new boost::thread(boost::BOOST_BIND(&CConnectClient::ClientRecvThread, NewClient));
			NewClient->thrSrvRecv = new boost::thread(boost::BOOST_BIND(&CConnectClient::ServerRecvThread, NewClient));
			
			clientList.push_back(NewClient);

			NewSocket = INVALID_SOCKET;
		}
	}

	void CheckClientList(){
		for(std::vector<CConnectClient*>::const_iterator itr = clientList.begin(); itr != clientList.end(); ++itr){
			if((*itr)->finished != true) continue;

			Log(MSG_DEBUG, "Removed a %s connection, active count: %i", (*itr)->GetSrvName(), clientList.size() - 1);

			delete (*itr)->thrCliRecv;
			delete (*itr)->thrSrvRecv;
			delete (*itr);

			clientList.erase(itr);
			break;
		}
	}

	inline void SetWorldIP(char* newIP){
		strcpy_s(worldIP, 0x10, newIP);
	}

	inline void SetWorldPort(int newPort){
		worldPort = newPort;
	}

	inline void SetGameIP(char* newIP){
		strcpy_s(gameIP, 0x10, newIP);
	}

	inline void SetGamePort(int newPort){
		gamePort = newPort;
	}
private:
	std::vector<CConnectClient*> clientList;

	SOCKET sckListenLogin;
	SOCKET sckListenWorld;
	SOCKET sckListenGame;

	boost::thread* thrListen;

	char loginIP[0x10];
	char worldIP[0x10];
	char gameIP[0x10];

	int loginPort;
	int worldPort;
	int gamePort;
};