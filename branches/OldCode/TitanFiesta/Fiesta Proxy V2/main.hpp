#include <iostream>
#include <winsock.h>
#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>

#pragma comment(lib, "ws2_32.lib")

typedef unsigned __int8 byte;
typedef unsigned __int16 word;
typedef unsigned __int32 dword;
typedef unsigned __int64 qword;

class CConnectClient;
class CPacket;

void ReceivedLoginServerPacket(CPacket* pak, CConnectClient* login);
void ReceivedWorldServerPacket(CPacket* pak, CConnectClient* world);
void ReceivedGameServerPacket(CPacket* pak, CConnectClient* game);

void ReceivedLoginClientPacket(CPacket* pak, CConnectClient* login);
void ReceivedWorldClientPacket(CPacket* pak, CConnectClient* world);
void ReceivedGameClientPacket(CPacket* pak, CConnectClient* game);

void HandleCommand(CPacket* pak, char* command, CConnectClient* game);

extern byte outBuffer[256];

#include "CPacket.hpp"
#include "Log.hpp"
#include "CFiestaCrypt.hpp"
#include "CFiestaLauncher.hpp"
#include "CListener.hpp"