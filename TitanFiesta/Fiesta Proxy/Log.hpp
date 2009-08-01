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
	//Outgoing filter
	//if(pak->command == 0x2019 || pak->command == 0x2012) return;

	//Incoming filter
	//if(pak->command == 0x201a || pak->command == 0x2018 || pak->command == 0x1805) return;

	if(pak->command == 0x0826 || pak->command == 0x0827) return;//XTRAP Ping

	boost::mutex::scoped_lock lConsole(mConsole);
	FILE* fh = fopen("Proxy.log", "a");

	SetConsoleColor(MSG_INFO);
	printf("%c%c ", serv, source);
	fprintf(fh, "%c%c ", serv, source);

	SetConsoleColor(CC_WHITE);
	for(dword i = 0; i < pak->size; i++) {
		printf("%02x ", pak->buffer[i]);
		fprintf(fh, "%02x ", pak->buffer[i]);
	}
	
	printf("\n");
	fprintf(fh, "\n");
	
	fclose(fh);
	lConsole.unlock();
}

static void Log( LOG_TYPE flag, char* format, ... ){
	boost::mutex::scoped_lock lConsole(mConsole);
	FILE* fh = fopen("Proxy.log", "a");
	SetConsoleColor(flag);
	switch(flag){
	case MSG_NONE:
		printf("[MSG]: ");
		fprintf(fh, "[MSG]: ");
		break;
	case MSG_LOAD:
		printf("[LOAD]: ");
		fprintf(fh, "[LOAD]: ");
		break;
	case MSG_STATUS:
		printf("[STATUS]: ");
		fprintf(fh, "[STATUS]: ");
		break;
	case MSG_INFO:
		printf("[INFO]: ");
		fprintf(fh, "[INFO]: ");
		break;
	case MSG_WARNING:
		printf("[WARNING]: ");
		fprintf(fh, "[WARNING]: ");
		break;
	case MSG_ERROR:
		printf("[ERROR]: ");
		fprintf(fh, "[ERROR]: ");
		break;
	case MSG_DEBUG:
		printf("[DEBUG]: ");
		fprintf(fh, "[DEBUG]: ");
		break;
	}
	SetConsoleColor(CC_WHITE);

	va_list arglist;
	va_start(arglist, format);
	vprintf(format, arglist);
	vfprintf(fh, format, arglist);
	printf((flag == MSG_LOAD)?"\r":"\n");
	fprintf(fh, "\n");
	va_end(arglist);
	fclose(fh);
	lConsole.unlock();
}
