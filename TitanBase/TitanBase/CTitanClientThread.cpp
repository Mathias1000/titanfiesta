/* Copyright (C) 2008, 2009 TitanFiesta Dev Team
 * Licensed under GNU GPL v3
 * For license details, see LICENCE in the root folder. */

#include <main.h>

CTitanClientThread::CTitanClientThread( CTitanServer* TitanServer, boost::thread* BoostThread, dword TId )
																				: ParentServer(TitanServer), MyThread(BoostThread), Id(TId)
{
	ClientCount = 0;
}

void CTitanClientThread::Start( )
{
	fd_set fds;
	int activity;
	timeval timeout = {0,0};
	dword recvd = 0;
	
	Log(MSG_DEBUG, "Thread Started [TID: %d]", Id);
	bool isActive = true;
	while(isActive){
		Sleep(CLIENT_THREAD_SLEEP);

		FD_ZERO( &fds );

		ParentServer->rwmClientList.acquireReadLock();
		for(dword i = 0; i < ParentServer->ClientList.size(); i++){
			CTitanClient* thisclient = ParentServer->ClientList[i];
			if( thisclient->ThreadID == Id && thisclient->isActive ) {
				FD_SET( thisclient->mySocket, &fds );
			}
		}
		ParentServer->rwmClientList.releaseReadLock();

		if( fds.fd_count <= 0 ){ isActive = false;	continue; }
		activity = select( 0, &fds, NULL, NULL, &timeout );
		if( activity == 0 ) continue;
		if( activity <= 0 ) Log(MSG_DEBUG, "Unable to select sockets! (TID: %i)", Id);

		ParentServer->rwmClientList.acquireReadLock( );
		for( dword i=0; i < ParentServer->ClientList.size( ); i++ ){
			CTitanClient* thisclient = ParentServer->ClientList[i];
			if( FD_ISSET( thisclient->mySocket, &fds ) ){
#ifdef TITAN_USING_STREAMS
				recvd = recv( thisclient->mySocket, (string)thisclient->pak->Buffer(), thisclient->pak->Size(), 0 );
#endif
#ifdef TITAN_USING_PACKETS
				recvd = recv( thisclient->mySocket, (string)&thisclient->pak->Buffer()[thisclient->curRecv], thisclient->pak->Size() - thisclient->curRecv, 0 );
#endif
				if( recvd == 0 || recvd == SOCKET_ERROR ) {				
					thisclient->isActive = false;
					ParentServer->rwmClientList.releaseReadLock( );
					ParentServer->DisconnectClient( thisclient );
					ParentServer->rwmClientList.acquireReadLock( );
					Log(MSG_DEBUG, "User disconnected! (TID: %i) Reason: recvd=%i", Id, recvd);

					continue;
				}
#ifdef TITAN_USING_STREAMS
				if(!ParentServer->DecryptBufferStream( thisclient, thisclient->pak, recvd )){
					Log(MSG_DEBUG, "Invalid Packet Data");
				}else{
					ParentServer->OnReceivePacket( thisclient, thisclient->pak );
				}
#endif
#ifdef TITAN_USING_PACKETS
				thisclient->curRecv += recvd;

				//Finished reading data?
				if( thisclient->curRecv != thisclient->pak->Size() )	continue;

				if( thisclient->pak->Size() == PACKET_HEADER_SIZE ){
					if(!ParentServer->DecryptBufferHeader( thisclient, thisclient->pak )){
						Log( MSG_ERROR, "Invalid Packet Header" );
						thisclient->pak->Size( PACKET_HEADER_SIZE );
						thisclient->pak->Pos(0);
						continue;
					}

					if(thisclient->pak->Size() > PACKET_HEADER_SIZE) continue;
				}

				if(!ParentServer->DecryptBufferData(thisclient, thisclient->pak)){
					Log(MSG_ERROR, "Invalid Packet Data");
				}else{
					ParentServer->OnReceivePacket( thisclient, thisclient->pak );
				}
#endif
#ifdef TITAN_USING_STREAMS
				thisclient->pak->Size( TITAN_RECV_STREAM_SIZE );
				thisclient->pak->Pos(0);
				thisclient->pak->Command( 0 );
				memset(thisclient->pak->Buffer(), 0, TITAN_RECV_STREAM_SIZE);
#endif
#ifdef TITAN_USING_PACKETS
				thisclient->pak->Size( PACKET_HEADER_SIZE );
				thisclient->pak->Pos(0);
				thisclient->curRecv = 0;
#endif
			}
		}
		ParentServer->rwmClientList.releaseReadLock( );
	}

	ParentServer->RemoveThread( this );
}
