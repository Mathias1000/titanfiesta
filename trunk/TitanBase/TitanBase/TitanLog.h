/* Copyright (C) 2008, 2009 TitanFiesta Dev Team
 * Licensed under GNU GPL v3
 * For license details, see LICENCE in the root folder. */

#if TITAN_PLATFORM == TITAN_PLATFORM_WIN32
typedef enum
{
		CC_BLACK,
		CC_BLUE,
		CC_GREEN,
		CC_CYAN,
		CC_RED,
		CC_MAGENTA,
		CC_BROWN,
		CC_LIGHTGRAY,
		CC_DARKGRAY,
		CC_LIGHTBLUE,
		CC_LIGHTGREEN,
		CC_LIGHTCYAN,
		CC_LIGHTRED,
		CC_LIGHTMAGENTA,
		CC_YELLOW,
		CC_WHITE
} CONSOLE_COLORS;
#else
typedef enum
{
		CC_DARKGRAY = 0x0130,
		CC_LIGHTRED = 0x0131,
		CC_LIGHTGREEN = 0x0132,
		CC_YELLOW = 0x0133,
		CC_LIGHTBLUE = 0x0134,
		CC_LIGHTMAGENTA = 0x0135,
		CC_LIGHTCYAN = 0x0136,
		CC_WHITE = 0x0137,
		CC_BLACK = 0x2230,
		CC_RED = 0x2231,
		CC_GREEN = 0x2232,
		CC_BROWN = 0x2233,
		CC_BLUE = 0x2234,
		CC_MAGENTA = 0x2235,
		CC_CYAN = 0x2236,
		CC_LIGHTGRAY = 0x2237
} CONSOLE_COLORS;
#endif

typedef enum
{
	MSG_NONE = CC_WHITE,
	MSG_LOAD = CC_LIGHTMAGENTA,
	MSG_LOADED = 1337,
	MSG_STATUS = CC_LIGHTGREEN,
	MSG_INFO = CC_GREEN,
	MSG_WARNING = CC_YELLOW,
	MSG_ERROR = CC_LIGHTRED,
	MSG_DEBUG = CC_LIGHTBLUE
} LOG_TYPE;


const void SetConsoleColor( dword textcolor, dword backcolor = 0 );
const void Log( LOG_TYPE flag, string format, ... );
const void LogPacket( byte* pak, word size );
