#include <main.h>

CTitanClient::CTitanClient(CTitanServer* TitanServer, dword myThread) {
	ThreadID = myThread;
	ParentServer = TitanServer;
#ifdef TITAN_USING_STREAMS
	pak = new CTitanPacket((byte*)malloc(TITAN_RECV_STREAM_SIZE),true);
	pak->Size( TITAN_RECV_STREAM_SIZE );
#endif
#ifdef TITAN_USING_PACKETS
	pak = new CTitanPacket();
	pak->Size( PACKET_HEADER_SIZE );
	curRecv = 0;
#endif
}

CTitanClient::~CTitanClient(){
}