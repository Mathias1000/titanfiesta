/* Copyright (C) 2008, 2009 TitanFiesta Dev Team
 * Licensed under GNU GPL v3
 * For license details, see LICENCE in the root folder. */

struct FixLenStr {
	FixLenStr(char* str, int len):_str(str),_len(len){}
	char* _str;
	int _len;
};

class CPacket
{
public:
	CPacket(char* nBuf, dword nLen){
		buffer = reinterpret_cast<byte*>(nBuf);
		size = nLen;
		GetFromBuffer();
	}
	CPacket(word command, byte* buffer){
		size = 3;
		pos = 3;
		this->command = command;
		this->buffer = buffer;
		SetBuffer();
	}
	~CPacket(){}

	void ResetPacket(word command){
		size = 3;
		pos = 3;
		this->command = command;
		SetBuffer();
	}

	void GetFromBuffer(){
		if(buffer[0] != 0){
			command = *reinterpret_cast<word*>(buffer + 1);
			pos = 3;
		}else{
			command = *reinterpret_cast<word*>(buffer + 3);
			pos = 5;
		}
	}

	void SetBuffer(){
		Set<byte>(size - 1, 0);
		Set<word>(command, 1);
	}

	template <class T> T Get(int pos){
		return *reinterpret_cast<T*>(buffer + pos);
	}

	template <> char* Get<char*>(int pos){
		return reinterpret_cast<char*>(buffer + pos);
	}

	template <class T> T Read(){
		pos += sizeof(T);
		return *reinterpret_cast<T*>(buffer + pos - sizeof(T));
	}

	template <> char* Read<char*>(){
		char* tmp = reinterpret_cast<char*>(buffer + pos);
		pos += strlen(tmp);
		return tmp;
	}

	template <class T> void Add(T val){
		*reinterpret_cast<T*>(buffer + pos) = val;
		pos += sizeof(T);
		size += sizeof(T);
	}

	template <class T> void Set(T val, int pos){
		*reinterpret_cast<T*>(buffer + pos) = val;
	}

	template <> void Set<FixLenStr>(FixLenStr val, int pos){
		int valLen = strlen(val._str);
		if(valLen >= val._len){
			memcpy(buffer + pos, val._str, val._len);
			return;
	}
		memcpy(buffer + pos, val._str, valLen);
		memset(buffer + pos + valLen, 0, val._len - valLen);
	}


	word size;
	word command;
	byte* buffer;
	dword pos;
};