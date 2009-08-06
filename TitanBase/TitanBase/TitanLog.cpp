/* Copyright (C) 2008, 2009 TitanFiesta Dev Team
 * Licensed under GNU GPL v3
 * For license details, see LICENCE in the root folder. */

#include <main.h>

boost::mutex mConsole;

const void SetConsoleColor( dword textcolor, dword backcolor )
{
	#if TITAN_PLATFORM == TITAN_PLATFORM_WIN32
	SetConsoleTextAttribute( GetStdHandle(STD_OUTPUT_HANDLE), textcolor + ( backcolor << 4 ) );
	#else
	printf("\033[%02x;%02xm", (textcolor & 0xFF00)>>8, textcolor & 0xFF);
	#endif
}

const void Log( LOG_TYPE flag, string format, ... ){
	boost::mutex::scoped_lock lConsole( mConsole );

	SetConsoleColor( flag );
	switch(flag){
		case MSG_NONE:
			printf( "[MSG]: " );
			break;
		case MSG_LOAD:
			printf( "[LOAD]: " );
			break;
		case MSG_LOADED:
			SetConsoleColor( MSG_STATUS );
			printf( "[LOADED]: " );
			break;
		case MSG_STATUS:
			printf( "[STATUS]: " );
			break;
		case MSG_INFO:
			printf( "[INFO]: " );
			break;
		case MSG_WARNING:
			printf( "[WARNING]: " );
			break;
		case MSG_ERROR:
			printf( "[ERROR]: " );
			break;
#ifdef TITAN_NO_DEBUG_MSG
		case MSG_DEBUG:
			lConsole.unlock( );
			return;
		break;
#else
		case MSG_DEBUG:
			printf( "[DEBUG]: " );
		break;
#endif
	}
	SetConsoleColor( CC_WHITE );

	va_list arglist;
	va_start( arglist, format );
	vprintf( format, arglist );
	printf( ( flag == MSG_LOAD ) ? "\r" : "\n" );

	va_end( arglist );
	lConsole.unlock( );
}

const void LogPacket( byte* pak, word size )
{
	boost::mutex::scoped_lock lConsole( mConsole );
	for(dword i = 0; i < size; i++)
		printf("%02X ", pak[i]);
	printf("\n");
	lConsole.unlock( );
}