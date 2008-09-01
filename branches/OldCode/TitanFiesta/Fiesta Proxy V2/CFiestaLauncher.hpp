#pragma once
#include <curl/curl.h>
#pragma comment(lib, "libcurl.lib")

struct MemoryStruct {
	char *memory;
	size_t size;
};

static void *myrealloc(void *ptr, size_t size){
	if(ptr)
		return realloc(ptr, size);
	else
		return malloc(size);
}

static size_t WriteMemoryCallback(void *ptr, size_t size, size_t nmemb, void *data){
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

class CFiestaLauncher {
public:
	CFiestaLauncher(){}
	~CFiestaLauncher(){}

	bool Launch(char* username, char* md5hash){
		curl_global_init(CURL_GLOBAL_ALL);
		curl = curl_easy_init();

		MemoryStruct chunk;
		chunk.memory = NULL;
		chunk.size = 0;

		char url[255];
		sprintf_s(url, 255, "http://rest.outspark.net/user/v1/login?realm=fiesta&user=%s&password=%s&version=98&output=text", username, md5hash);
		curl_easy_reset(curl);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
		curl_easy_setopt(curl, CURLOPT_URL, url);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
		curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");
		curl_easy_perform(curl);
		
		if(strstr(chunk.memory, "token") == NULL){
			Log(MSG_ERROR, "Could not acquire token from outspark!");
			curl_easy_cleanup(curl);
			return false;
		}

		char* pch = chunk.memory + strlen("{\"token\":\"");
		char* pch2 = strchr(pch, '"'); *pch2 = 0;
		char token[64];
		strcpy_s(token, 64, pch);
		LaunchExe(token);

		if(chunk.memory) free(chunk.memory);

		curl_easy_cleanup(curl);
		return true;
	}

	void LaunchExe(char* token){
		STARTUPINFO si = { sizeof(si) };
		PROCESS_INFORMATION pi;
		char commandLine[256];
		sprintf_s(commandLine, 256, "Fiesta.bin -t %s -i 127.0.0.1 -u http://store.outspark.com/game/fiesta", token);
		CreateProcessA(0, commandLine, NULL, NULL, FALSE, NULL, NULL, NULL, &si, &pi);
	}
private:
	CURL *curl;
};