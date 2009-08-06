/* Copyright (C) 2008, 2009 TitanFiesta Dev Team
 * Licensed under GNU GPL v3
 * For license details, see LICENCE in the root folder. */

#define TITAN_BUFFER_SIZE 8192

class CTitanBufferedFile {
public:
	CTitanBufferedFile():bOpen(false){}
	CTitanBufferedFile(char* path, char* mode):bOpen(false){ Open(path, mode); }
	~CTitanBufferedFile(){}

	void Close(){
		if(bOpen) fclose(pFh);
		bOpen = false;
		currentBufferPos = 0;
		currentReadPos = 0;
	}

	bool Open(char* path, char* mode){
		if(bOpen) return false;

		currentBufferPos = 0;
		currentReadPos = 0;
		fopen_s(&pFh, path, mode);
		bOpen = (pFh != NULL);
		CheckBuffer(-1);
		return bOpen;
	}

	template <class T> T Get(){
		if(!CheckBuffer(sizeof(T))) throw((int)101);
		T value;
		memcpy(&value, &dataBuffer[currentReadPos - currentBufferPos], sizeof(T));
		currentReadPos += sizeof(T);
		return value;
	}

	char* GetString(int length){
		if(!bOpen) return NULL;
		if(length <= 0) return NULL;
		if(length >= TITAN_BUFFER_SIZE) return NULL;
		if(!CheckBuffer(length)) throw((int)101);

		char* value = new char[length+1];
		memcpy(value, &dataBuffer[currentReadPos - currentBufferPos], length);
		value[length] = 0;
		currentReadPos += length;
		return value;
	}

	char* GetWcsToMbs(int length){
		if(!bOpen) return NULL;
		if(length <= 0) return NULL;
		if(length >= TITAN_BUFFER_SIZE) return NULL;
		if(!CheckBuffer(length)) throw((int)101);

		dword bufLen = length/2;
		char* value = new char[bufLen+1];
		dword mbsPos = 0;
		for(int i = 0; i < length; i+=2){
			value[mbsPos] = dataBuffer[(currentReadPos - currentBufferPos) + i];
			mbsPos++;
		}
		value[bufLen] = 0;
		currentReadPos += length;
		return value;
	}

	byte* ReadBytes(int length){
		if(!bOpen) return NULL;
		if(length <= 0) return NULL;
		if(length >= TITAN_BUFFER_SIZE) return NULL;
		if(!CheckBuffer(length)) throw((int)101);

		byte* value = new byte[length];
		memcpy(value, &dataBuffer[currentReadPos - currentBufferPos], length);
		currentReadPos += length;
		return value;
	}

	void GetBuffer(byte* buffer, int length){
		buffer[0] = 0;
		if(!bOpen) return;
		if(length <= 0) return;
		if(length >= TITAN_BUFFER_SIZE) return;
		if(!CheckBuffer(length)) throw((int)101);

		memcpy(buffer, &dataBuffer[currentReadPos - currentBufferPos], length);
		currentReadPos += length;
	}

	inline void Skip(int SkipBytes){
		if(SkipBytes < 0) return;
		CheckBuffer(SkipBytes);
		currentReadPos += SkipBytes;
	}

	void Seek(long newPos){
		if(!bOpen) return;
		currentReadPos = newPos;
		fseek(pFh, currentReadPos, SEEK_SET);
		fread(dataBuffer, 1, TITAN_BUFFER_SIZE, pFh);
		currentBufferPos = currentReadPos;
	}
	inline long Pos(){ if(!bOpen) return -1; return currentReadPos; }
	inline bool IsOpen(){ return bOpen; }
	
private:
	bool CheckBuffer(int checkSize){
		if(!bOpen) return false;
		if(checkSize == -1 || (currentBufferPos + TITAN_BUFFER_SIZE) < (currentReadPos + checkSize)){
			fseek(pFh, currentReadPos, SEEK_SET);
			fread(dataBuffer, 1, TITAN_BUFFER_SIZE, pFh);
			currentBufferPos = currentReadPos;
		}
		return true;
	}

	FILE* pFh;
	bool bOpen;

	byte dataBuffer[TITAN_BUFFER_SIZE];

	long currentBufferPos;
	long currentReadPos;
};

#undef TITAN_BUFFER_SIZE