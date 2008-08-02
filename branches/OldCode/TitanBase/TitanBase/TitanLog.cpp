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

const void LogPacket( CTitanPacket* pak )
{
	Log(MSG_INFO,"LogPacket(): Command: 0x%08X Size: 0x%08X", pak->Command(), pak->Size());
	dword cPos = pak->Pos();
	pak->Pos(0);
	for(dword i=0;i<pak->Size();i++)
		printf("%02X ", pak->Read<byte>());
	pak->Pos(cPos);
	printf("\n");
}