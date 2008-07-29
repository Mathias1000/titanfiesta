class CConnection {
public:
	CConnection(char* name):serverName(name),disconnect(false),xorTableLoc(0){}
	~CConnection(){}

	char* serverName;


	bool WaitConnection(char* ip, dword port){
		sockaddr_in ain;
		cliSck = socket(AF_INET, SOCK_STREAM, 0);
		if(cliSck == INVALID_SOCKET) return false;
		ain.sin_family = AF_INET;
		ain.sin_addr.s_addr = inet_addr(ip);
		ain.sin_port = htons(port);
		if(bind(cliSck, (const sockaddr*)&ain, sizeof(ain)) == SOCKET_ERROR) return false;

		if(listen(cliSck, SOMAXCONN) == -1 ) return false;

		sockaddr client_info;
		int client_info_len = sizeof( sockaddr_in );
		NewSocket = INVALID_SOCKET;

		while( NewSocket == INVALID_SOCKET ){
			NewSocket = accept( cliSck, (sockaddr*)&client_info, &client_info_len );
		}
		return true;
	}

	bool Connect(char* ip, dword port, dword nId){
		id = nId;
		sockaddr_in ain;
		srvSck = socket(AF_INET, SOCK_STREAM, 0);
		if(srvSck == INVALID_SOCKET) return false;
		ain.sin_family = AF_INET;
		ain.sin_addr.s_addr	= inet_addr(ip);
		ain.sin_port = htons(port);
		if(connect(srvSck, (const sockaddr*)&ain, sizeof(ain)) == SOCKET_ERROR) return false;

		return true;
	}

	void ReceivedClientPacket(char* buffer, dword length){
		CPacket pak;
		pak.buffer = buffer;
		pak.size = length;
		SendServerPacket(&pak);
	}

	void ReceivedServerPacket(char* buffer, dword length){
		CPacket pak;
		pak.buffer = buffer;
		pak.size = length;
		if(length >= 3){
			pak.command = *reinterpret_cast<word*>(pak.buffer + 1);
			if(pak.command == 0x0C0C){
				strcpy_s(charServerIP, 32, pak.buffer + 4);
				memset(pak.buffer + 4, '\0', 0x10);
				memcpy(pak.buffer + 4, "127.0.0.1", strlen("127.0.0.1"));
				StartCharServer();
			}
			if(pak.command == 0x1003){
				strcpy_s(worldServerIP, 32, pak.buffer + 3);
				memset(pak.buffer + 3, '\0', 0x10);
				memcpy(pak.buffer + 3, "127.0.0.1", strlen("127.0.0.1"));
				worldServerPort = *(word*)(pak.buffer + 0x13);
				StartWorldServer();
			}
			if(pak.command == 0x0807){
				xorTableLoc = *reinterpret_cast<word*>(pak.buffer + 3);
			}
		}
		SendClientPacket(&pak);
	}

	void SendServerPacket(CPacket* pak){
		printf("%sOUT %02x ", serverName, (unsigned char)pak->buffer[0]);
		for(dword i = 1; i < pak->size; i++){
			printf("%02x ", (unsigned char)(pak->buffer[i] ^ xorTable[xorTableLoc]));
			xorTableLoc++;
			if(xorTableLoc == 0x1F3) xorTableLoc = 0;
		}
		printf("\n");
		send( srvSck, pak->buffer, pak->size, 0 );
	}

	void SendClientPacket(CPacket* pak){
		//if(pak->command != 0x201a){
		printf("%sIN ", serverName);
		for(dword i = 0; i < pak->size; i++)
			printf("%02x ", (unsigned char)pak->buffer[i]);
		printf("\n");
		//}
		send( NewSocket, pak->buffer, pak->size, 0 );
	}

	void ServerThread(){
		char buffer[0x8000];
		dword recvd = 0;
		dword bufsize = 0;
		dword readlen = 1;
		bool isWordSize = false;


		do {
			Sleep( 10 );

			recvd = recv( srvSck, buffer + bufsize, readlen-bufsize, 0 );

			if( recvd == 0 || recvd == SOCKET_ERROR ){
				printf("[SRV] Recvd: %i Error: %ld\n", recvd, WSAGetLastError());
				break;
			}
			
			bufsize += recvd;
			if(isWordSize){
				readlen = *reinterpret_cast<word*>(buffer + 1) + 3;
				isWordSize = false;
			}
			if( bufsize != readlen ) continue;
			if( bufsize == 1 ) {
				readlen = *reinterpret_cast<byte*>(buffer) + 1;
				if(readlen == 1){
					readlen = 3;
					isWordSize = true;
				}
				if( readlen < 1 ){ bufsize = 0; readlen = 1; continue; }
				if( readlen > 1 ) continue;
			}

			ReceivedServerPacket( buffer, readlen );

			bufsize = 0;
			readlen = 1;
		}while( !disconnect );

		closesocket(srvSck);

		disconnect = true;
	}

	void ClientThread(){
		char buffer[0x8000];
		dword recvd = 0;
		dword bufsize = 0;
		dword readlen = 1;
		bool isWordSize = false;

		do {
			Sleep( 10 );

			recvd = recv( NewSocket, buffer + bufsize, readlen-bufsize, 0 );

			if( recvd == 0 || recvd == SOCKET_ERROR ){
				printf("[CLI] Recvd: %i Error: %ld\n", recvd, WSAGetLastError());
				break;
			}

			bufsize += recvd;
			if(isWordSize){
				readlen = *reinterpret_cast<word*>(buffer + 1) + 3;
				isWordSize = false;
			}
			if( bufsize != readlen ) continue;
			if( bufsize == 1 ) {
				readlen = *reinterpret_cast<byte*>(buffer) + 1;
				if(readlen == 1){
					readlen = 3;
					isWordSize = true;
				}
				if( readlen < 1 ){ bufsize = 0; readlen = 1; continue; }
				if( readlen > 1 ) continue;
			}

			ReceivedClientPacket( buffer, readlen );

			bufsize = 0;
			readlen = 1;
		}while( !disconnect );

		closesocket(cliSck);

		disconnect = true;
	}
private:
	SOCKET cliSck;
	SOCKET srvSck;
	SOCKET NewSocket;
	dword id;

	bool disconnect;

	word xorTableLoc;
};
