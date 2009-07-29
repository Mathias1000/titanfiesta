/*-----------------------------------*\
| The one, the only, TitanGlobals!!   |
|  Include this in your program for   |
|  teh leet titan base code!!!!11one  |
\*-----------------------------------*/

#pragma once
#pragma warning(disable:4267) //'size_t' conversions
#pragma warning(disable:4244) //'double' conversions
#pragma comment(lib,"ws2_32.lib")

//Global typedefs
typedef unsigned __int8			byte;
typedef unsigned __int16		word;
typedef unsigned __int32		dword;
typedef unsigned __int64		qword;
typedef char*					string;

//Global macros
#define CLEAN_ARRAY( arr, size ) { for( dword arrpos = 0; arrpos<size; arrpos++ ) arr[arrpos] = 0; }
#define DEL( val ) { if( val != NULL ) { delete val; val = NULL; } }
#define DELARR( arr ) { if( arr != NULL ) { delete [] arr; arr = NULL; } }
#define DEL2DARR( arr, size ) { if( arr != NULL ) { for(dword i = 0; i < size; i++){ delete arr[i]; } delete [] arr; arr = NULL; } }
#define DELVEC( vector ) { for(dword vector_pos=0;vector_pos<vector.size();vector_pos++){ DEL( vector.at(vector_pos) ) } vector.clear(); }
#define DELVECARR( vector ) { for(dword vector_pos=0;vector_pos<vector.size();vector_pos++){ DELARR( vector.at(vector_pos) ) } vector.clear(); }

#define strtobyte( str ) byte(strtoul(str, NULL, 0))
#define strtoword( str ) word(strtoul(str, NULL, 0))
#define strtodword( str ) dword(strtoul(str, NULL, 0))
#define strtofloat( str ) float(atof(str))

//Global defines
#ifndef TITAN_CLIENTS_PER_THREAD
#	define TITAN_CLIENTS_PER_THREAD 1
#endif
#ifndef CLIENT_THREAD_SLEEP
#	define CLIENT_THREAD_SLEEP 10
#endif

#define TITAN_PLATFORM_WIN32 1
#define TITAN_PLATFORM_LINUX 2
#define TITAN_COMPILER_MSVC 1
#define TITAN_COMPILER_GNUC 2
#define TITAN_COMPILER_BORL 3
#define TITAN_ARCHITECTURE_32 1
#define TITAN_ARCHITECTURE_64 2

#if defined( _MSC_VER )
#	define TITAN_COMPILER TITAN_COMPILER_MSVC
#	define TITAN_COMP_VER _MSC_VER
#elif defined( __GNUC__ )
#	define TITAN_COMPILER TITAN_COMPILER_GNUC
#	define TITAN_COMP_VER (((__GNUC__)*100) + \ (__GNUC_MINOR__*10) + \ __GNUC_PATCHLEVEL__)
#elif defined( __BORLANDC__ )
#	define TITAN_COMPILER TITAN_COMPILER_BORL
#	define TITAN_COMP_VER __BCPLUSPLUS__
#else
#	pragma error "Compiler Unknown, you = gay."
#endif

#if TITAN_COMPILER == TITAN_COMPILER_MSVC
#   if TITAN_COMP_VER >= 1200
#       define FORCEINLINE __forceinline
#   endif
#elif defined(__MINGW32__)
#   if !defined(FORCEINLINE)
#       define FORCEINLINE __inline
#   endif
#else
#   define FORCEINLINE __inline
#endif

#if defined( __WIN32__ ) || defined( _WIN32 ) || defined( WIN32 )
#	define TITAN_PLATFORM TITAN_PLATFORM_WIN32
#else
#	define TITAN_PLATFORM TITAN_PLATFORM_LINUX
#endif

#if defined(__x86_64__) || defined(_M_X64)
#   define TITAN_ARCH_TYPE TITAN_ARCHITECTURE_64
typedef qword ADDR_PTR;
#else
#   define TITAN_ARCH_TYPE TITAN_ARCHITECTURE_32
typedef qword ADDR_PTR;
#endif

#ifndef DEFAULT_PACKET_SIZE
#	define DEFAULT_PACKET_SIZE 0x200
# pragma message("Warning: Default Packet Size not specified, using 0x200. To change, use DEFAULT_PACKET_SIZE")
#endif

#ifndef PACKET_HEADER_SIZE
#	ifdef TITAN_USING_STREAMS
#		define PACKET_HEADER_SIZE 0
#	else
#		error Packet Header Size not set, use PACKET_HEADER_SIZE
#	endif
#endif

#ifndef __cplusplus
#	error THIS BASE IS FOR C++ ONLY!!!
#endif

#ifdef TITAN_USING_STREAMS
#	ifdef TITAN_USING_PACKETS
#		error You cannot use Streams and Packets, please change TITAN_USING_STREAMS or TITAN_USING_PACKETS
#	endif
#endif

#ifndef TITAN_USING_STREAMS
#	ifndef TITAN_USING_PACKETS
#		error You must use Streams or Packets, please set TITAN_USING_STREAMS or TITAN_USING_PACKETS
#	endif
#endif

#ifdef TITAN_USING_SHOW_STATUS
# ifndef TITAN_SHOW_STATUS_DELAY
#		pragma message("You are using TITAN_USING_SHOW_STATUS but have not set TITAN_SHOW_STATUS_DELAY seconds, using default 30 seconds")
#		define TITAN_SHOW_STATUS_DELAY 30
# endif
#	ifdef TITAN_USING_CONSOLE_COMMANDS
#		error You cannot use TITAN_USING_SHOW_STATUS and TITAN_USING_CONSOLE_COMMANDS
#	endif
#endif

#ifndef TITAN_ADDSTRING_NULLTERMINATED
#	ifndef TITAN_ADDSTRING_STRLEN_TYPE
#		error You must define a string type either TITAN_ADDSTRING_NULLTERMINATED or TITAN_ADDSTRING_STRLEN_TYPE typename
#	endif
#endif
#ifndef TITAN_ADDSTRING_STRLEN_TYPE
# define TITAN_ADDSTRING_STRLEN_TYPE dword
#endif

//Global includes
#include <winsock2.h>
#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <iostream>
#include <time.h>
#include <math.h>
#include <vector>
#if TITAN_PLATFORM == TITAN_PLATFORM_WIN32
#	include <windows.h>
#endif

#if defined(TITAN_USING_ISC) || defined(TITAN_IS_ISC_SERVER)
#	define TITAN_DEFAULT_ISC_IP "127.0.0.1"
#	define TITAN_DEFAULT_ISC_PORT 1337
	typedef enum
	{
		TITAN_ISC_IDENTIFY,
		TITAN_ISC_SETISCID,
		TITAN_ISC_REMOVE,
		TITAN_ISC_UPDATEUSERCNT,
		TITAN_ISC_SERVERLIST,
	} TITAN_ISC_PACKETS;

	struct CServerData
	{
		CServerData( )
		{
			type = 0;
			iscid = 0;
			id = 0;
			owner = 0;
			status = 0;
			name = NULL;
			ip = NULL;
			port = 0;
			flags = 0;
			currentusers = 0;
			maxusers = 0;
		}
		~CServerData( )
		{
			DELARR( name );
			DELARR( ip );
		}
		byte type;
		word iscid;
		word id;
		word owner;
		word status;
		string name;
		string ip;
		dword port;
		qword flags;
		dword currentusers;
		dword maxusers;
	};
#endif

struct CTitanConfig
{
	string BindIp;
	word BindPort;

#ifdef TITAN_USING_ISC
	string ISCIp;
	word ISCPort;
#endif
};

#ifdef TITAN_USING_CVECTOR2F
struct CVector2F
{
	CVector2F() : x(0), y(0) {}
	CVector2F(float nX, float nY) : x(nX), y(nY) {}
	float x, y;
	
	bool operator == ( const CVector2F& otherpoint )
	{
		return (fabs(this->x-otherpoint.x)<0.001f && fabs(this->y-otherpoint.y)<0.001f);
	}

	float distance( const CVector2F& v )
	{
		float dX = x - v.x;
		dX *= dX;
		float dY = y - v.y;
		dY *= dY;
		return sqrt(dX + dY);
	}
};
#endif

#ifdef TITAN_USING_CVECTOR2D
struct CVector2D
{
	CVector2D() : x(0), y(0) {}
	CVector2D(dword nX, dword nY) : x(nX), y(nY) {}
	dword x, y;
	
	bool operator == ( const CVector2D& otherpoint ){
		return (x == otherpoint.x) && (y == otherpoint.y);
	}

	bool operator != ( const CVector2D& otherpoint ){
		return (x != otherpoint.x) || (y != otherpoint.y);
	}
};
#endif

struct FixLenStr {
	FixLenStr(char* str, dword len):_str(str),_len(len){}
	char* _str;
	int _len;
};

#include "TitanLog.h"
#include "CTitanPacket.hpp"
#include "CReadWriteMutex.hpp"
#ifdef TITAN_USING_BINARY
#	include "TitanBinary.h"
#endif
#ifdef TITAN_USING_ARRAY
#	include "CTitanArray.hpp"
#endif
#ifdef TITAN_USING_FILE
#	include "CTitanFile.hpp"
#endif
#ifdef TITAN_USING_BUFFERED_FILE
#	include "CTitanBufferedFile.hpp"
#endif
#ifdef TITAN_USING_MYSQL
#	include "CTitanSQL.hpp"
#endif
#ifdef TITAN_USING_INIREAD
#	include "CTitanIniReader.hpp"
#endif
#ifdef TITAN_USING_CRC32
#	include "CCrc32Dynamic.hpp"
#endif
#ifdef TITAN_USING_ASM_MATH
#	include "TitanASMMath.h"
#endif
#ifdef TITAN_USING_FPOINT
# include "TitanFPoint.hpp"
#endif
#ifdef TITAN_USING_RANDOM
# include "randomc.h"
#endif
#include "TitanBase.h"