#include <winsock.h>
#include <curl/curl.h>
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "libcurl.lib")

typedef unsigned __int8 byte;
typedef unsigned __int16 word;
typedef unsigned __int32 dword;
typedef unsigned __int64 qword;

#include "XorTable.h"

#include <iostream>
#include "CPacket.hpp"
void StartCharServer();
void StartWorldServer();
#include "CConnection.hpp"
