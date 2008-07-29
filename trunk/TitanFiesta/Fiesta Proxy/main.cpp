#include "main.h"

CConnection* loginServer;
CConnection* charServer;
CConnection* worldServer;
CURL *curl;

char charServerIP[32];
char worldServerIP[32];

DWORD WINAPI loginServerThread(LPVOID lpParam){
	loginServer->ServerThread();
	return 0;
}

DWORD WINAPI charServerThread(LPVOID lpParam){
	charServer->ServerThread();
	return 0;
}

DWORD WINAPI worldServerThread(LPVOID lpParam){
	worldServer->ServerThread();
	return 0;
}

DWORD WINAPI charServerWaitThread(LPVOID lpParam){
	charServer = new CConnection("[Char]");
	if(!charServer->WaitConnection("127.0.0.1", 9110)){
		printf("Couldnt wait for char server conections! :(\n");
	}
	return 0;
}

DWORD WINAPI worldServerWaitThread(LPVOID lpParam){
	worldServer = new CConnection("[Game]");
	if(!worldServer->WaitConnection("127.0.0.1", 9120)){
		printf("Couldnt wait for game server conections! :(\n");
		return 0;
	}

	if(!worldServer->Connect(worldServerIP, 9120, 0)){
		printf("Couldnt connect to game server! :(\n");
		return 0;
	}

	CreateThread( NULL, 0, worldServerThread, NULL, 0, NULL);
	worldServer->ClientThread();
	return 0;
}

void StartCharServer(){
	CreateThread( NULL, 0, charServerWaitThread, NULL, 0, NULL);
}

void StartWorldServer(){
	CreateThread( NULL, 0, worldServerWaitThread, NULL, 0, NULL);
}

struct MemoryStruct {
  char *memory;
  size_t size;
};

void *myrealloc(void *ptr, size_t size){
  if(ptr)
    return realloc(ptr, size);
  else
    return malloc(size);
}

size_t WriteMemoryCallback(void *ptr, size_t size, size_t nmemb, void *data){
  size_t realsize = size * nmemb;
  struct MemoryStruct *mem = (struct MemoryStruct *)data;

  mem->memory = (char *)myrealloc(mem->memory, mem->size + realsize + 1);
  if(mem->memory) {
    memcpy(&(mem->memory[mem->size]), ptr, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;
  }
  return realsize;
}

int main(int argc, char** argv){
	if(argc < 3){
		printf("%s <username> <md5 password>\n", argv[0]);
		system("PAUSE");
		return 0;
	}

	SetConsoleTitleA("Fiesta Online Proxy - By ExJam");

	WORD sockVersion;
	WSADATA wsaData;
	sockVersion = MAKEWORD(1, 1);
	WSAStartup(sockVersion, &wsaData);
	
	charServer = NULL;
	
	curl_global_init(CURL_GLOBAL_ALL);
  curl = curl_easy_init();
	
	struct MemoryStruct chunk;
	chunk.memory = NULL;
	chunk.size = 0;

	char url[256];
	sprintf_s(url, 256, "http://rest.outspark.net/user/v1/login?realm=fiesta&user=%s&password=%s&version=98&output=text", argv[1], argv[2]);
	curl_easy_reset(curl);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
	curl_easy_setopt(curl, CURLOPT_URL, url);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
	curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");
	curl_easy_perform(curl);

	if(strstr(chunk.memory, "token") == NULL){
		printf("Invalid token login!\n");
		printf("%s", chunk.memory);
		system("PAUSE");
		goto finished;
	}

	printf("Starting game...\n");

	{
		char* pch = chunk.memory + strlen("{\"token\":\"");
		char* pch2 = strchr(pch, '"'); *pch2 = 0;
		char token[64];
		strcpy_s(token, 64, pch);
		STARTUPINFO si = { sizeof( si ) };
		PROCESS_INFORMATION pi;
		char commandLine[256];
		sprintf_s(commandLine, 256, "Fiesta.exe -t %s -i 127.0.0.1 -u http://store.outspark.com/game/fiesta", token);
		CreateProcessA( 0, commandLine, NULL, NULL, FALSE, NULL, NULL, NULL, &si, &pi );
	}

	if(chunk.memory) free(chunk.memory);

	printf("Waiting for connection...\n");
	
	loginServer = new CConnection("[Login]");
	if(!loginServer->WaitConnection("127.0.0.1", 9010)){
		printf("Couldnt wait for login conections! :(\n");
		goto finished;
	}

	if(!loginServer->Connect("64.127.118.7", 9010, 0)){
		printf("Couldnt connect to login server! :(\n");
		goto finished;
	}

	CreateThread( NULL, 0, loginServerThread, NULL, 0, NULL);
	loginServer->ClientThread();

	if(charServer == NULL) goto finished;

	if(!charServer->Connect(charServerIP, 9110, 0)){
		printf("Couldnt connect to char server! :(\n");
		goto finished;
	}

	CreateThread( NULL, 0, charServerThread, NULL, 0, NULL);
	charServer->ClientThread();

	Sleep(1000);
finished:
	WSACleanup();
  curl_easy_cleanup(curl);
	return 0;
}