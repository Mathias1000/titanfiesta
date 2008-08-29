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

typedef enum
{
	MSG_NONE = CC_WHITE,
	MSG_LOAD = CC_LIGHTMAGENTA,
	MSG_STATUS = CC_LIGHTGREEN,
	MSG_INFO = CC_GREEN,
	MSG_WARNING = CC_YELLOW,
	MSG_ERROR = CC_LIGHTRED,
	MSG_DEBUG = CC_LIGHTBLUE
} LOG_TYPE;

static boost::mutex mConsole;

static void SetConsoleColor( dword textcolor, dword backcolor = 0 ){
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), textcolor + (backcolor << 4));
}

static void LogPacket(CPacket* pak, char serv, char source){
	boost::mutex::scoped_lock lConsole(mConsole);

	SetConsoleColor(MSG_INFO);
	printf("%c%c ", serv, source);

	SetConsoleColor(CC_WHITE);
	for(dword i = 0; i < pak->size; i++)
		printf("%02x ", pak->buffer[i]);
	
	printf("\n");
	lConsole.unlock();
}

static void Log( LOG_TYPE flag, char* format, ... ){
	boost::mutex::scoped_lock lConsole(mConsole);

	SetConsoleColor(flag);
	switch(flag){
	case MSG_NONE:
		printf("[MSG]: ");
		break;
	case MSG_LOAD:
		printf("[LOAD]: ");
		break;
	case MSG_STATUS:
		printf("[STATUS]: ");
		break;
	case MSG_INFO:
		printf("[INFO]: ");
		break;
	case MSG_WARNING:
		printf("[WARNING]: ");
		break;
	case MSG_ERROR:
		printf("[ERROR]: ");
		break;
	case MSG_DEBUG:
		printf("[DEBUG]: ");
		break;
	}
	SetConsoleColor(CC_WHITE);

	va_list arglist;
	va_start(arglist, format);
	vprintf(format, arglist);
	printf((flag == MSG_LOAD)?"\r":"\n");
	va_end(arglist);

	lConsole.unlock();
}