/* Copyright (C) 2008, 2009 TitanFiesta Dev Team
 * Licensed under GNU GPL v3
 * For license details, see LICENCE in the root folder. */

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

bool ReceivedLoginServerPacket(CPacket* pak, CConnectClient* login);
bool ReceivedWorldServerPacket(CPacket* pak, CConnectClient* world);
bool ReceivedGameServerPacket(CPacket* pak, CConnectClient* game);

bool ReceivedLoginClientPacket(CPacket* pak, CConnectClient* login);
bool ReceivedWorldClientPacket(CPacket* pak, CConnectClient* world);
bool ReceivedGameClientPacket(CPacket* pak, CConnectClient* game);

#include "CPacket.hpp"
#include "Log.hpp"
#include "CFiestaCrypt.hpp"
#include "CFiestaLauncher.hpp"
#include "CListener.hpp"
