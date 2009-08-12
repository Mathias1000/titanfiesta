/* Copyright (C) 2008, 2009 TitanFiesta Dev Team
 * Licensed under GNU GPL v3
 * For license details, see LICENCE in the root folder. */
#pragma once

class CTitanPacket
{
public:
	CTitanPacket(dword command = 0){
		_Size = PACKET_HEADER_SIZE;
		_Command = command;
		_CurPos = PACKET_HEADER_SIZE;
		_HeaderSize = PACKET_HEADER_SIZE;
	}
	~CTitanPacket(){
	}

	template <typename T> void Fill( T val, dword count ){
		for(dword i = 0; i < count; i++)
			Add<T>(val);
	}

	template <typename T> void Add( T val ){
		*((T*)&_Buffer[_Size]) = val;
		_Size += sizeof( T );
	}

	template <> void Add<string>( string val )
	{
#ifndef TITAN_ADDSTRING_NULLTERMINATED
		Add<TITAN_ADDSTRING_STRLEN_TYPE>(strlen((const char*)val));
#endif
		for(TITAN_ADDSTRING_STRLEN_TYPE i = 0; i < strlen((const char*)val); i++){
			Add<byte>(val[i]);
		}
#ifdef TITAN_ADDSTRING_NULLTERMINATED
		Add<byte>(0);
#endif
	}

	template <typename T> inline void AddStringLen(char* val){
		Add<T>(strlen(val));
		AddString(val);
	}

	inline void AddStringNull(char* val){
		AddString(val);
		Add<byte>(0);
	}

	void AddString(char* val){
		memcpy(_Buffer + _Size, val, strlen(val));
		_Size += strlen(val);
	}

	void AddBytes(byte* val, dword len){
		memcpy(_Buffer + _Size, val, len);
		_Size += len;
	} 
	
	void AddFixLenStr(char* str, int len){
		int strleng = strlen(str);
		if(strleng > len){
			memcpy(_Buffer + _Size, str, len);
			_Size += len;
			return;
		}
		memcpy(_Buffer + _Size, str, strleng);
		_Size += strleng;
		int extra = len - strleng;
		if(extra > 0){
			memset(_Buffer + _Size, 0, extra);
			_Size += extra;
		}
	}

	bool AddFile(char* path){
		FILE* fh;
		fopen_s(&fh, path, "rb");
		if(!fh) return false;
		fseek(fh, 0, SEEK_END);
		long fileSize = ftell(fh);
		fseek(fh, 0, SEEK_SET);
		fread(_Buffer + _Size, fileSize, 1, fh);
		_Size += fileSize;
		fclose(fh);
		return true;
	}

#if defined(TITAN_USING_ISC) || defined(TITAN_IS_ISC_SERVER)
	template <> void Add<CServerData*>(CServerData* srv){
		Add<byte>(srv->type);
		Add<word>(srv->iscid);
		Add<word>(srv->id);
		Add<word>(srv->owner);
		Add<word>(srv->status);
		Add<string>(srv->name);
		Add<string>(srv->ip);
		Add<dword>(srv->port);
		Add<qword>(srv->flags);
		Add<dword>(srv->currentusers);
		Add<dword>(srv->maxusers);
	}
#endif

	template <> void Add<struct CItem>(struct CItem item);

	template <typename T> void Set( T val, word pos, word offset = PACKET_HEADER_SIZE )
	{
		if (offset == PACKET_HEADER_SIZE) offset = _HeaderSize;
		*((T*)&_Buffer[pos + offset]) = val;
	}

	template <typename T> T Get( word pos, word offset = PACKET_HEADER_SIZE )
	{
		if (offset == PACKET_HEADER_SIZE) offset = _HeaderSize;
		return *((T*)&_Buffer[pos + offset]);
	}

	template <typename T> T Read( )
	{
		if( _CurPos + sizeof( T ) > Size() ) return (T)0;
		_CurPos += sizeof( T );
		return *((T*)&_Buffer[_CurPos - sizeof( T )]);
	}

	template <typename T> CTitanPacket& operator<<(T val){
		*reinterpret_cast<T*>(_Buffer + _Size) = val;
		_Size += sizeof( T );
		return (*this);
	}

	template <> CTitanPacket& operator<<(char* val){
		memcpy(_Buffer + _Size, val, strlen(val));
		_Size += strlen(val);
		return (*this);
	}

	template <> CTitanPacket& operator<<(FixLenStr val){
		int strleng = strlen(val._str);
		if(strleng > val._len){
			memcpy(_Buffer + _Size, val._str, val._len);
			_Size += val._len;
			return (*this);
		}
		memcpy(_Buffer + _Size, val._str, strleng);
		_Size += strleng;
		int extra = val._len - strleng;
		if(extra > 0){
			memset(_Buffer + _Size, 0, extra);
			_Size += extra;
		}
		return (*this);
	}

	template <typename T> CTitanPacket& operator>>(T& val){
		val = *reinterpret_cast<T*>(_Buffer + _CurPos);
		_CurPos += sizeof( T );
		return (*this);
	}

#if defined(TITAN_USING_ISC) || defined(TITAN_IS_ISC_SERVER)
	template <> CTitanPacket& operator<<(CServerData* srv){
		Add<byte>(srv->type);
		Add<word>(srv->iscid);
		Add<word>(srv->id);
		Add<word>(srv->owner);
		Add<word>(srv->status);
		Add<string>(srv->name);
		Add<string>(srv->ip);
		Add<dword>(srv->port);
		Add<qword>(srv->flags);
		Add<dword>(srv->currentusers);
		Add<dword>(srv->maxusers);
	}
#endif
#ifdef TITAN_USING_CVECTOR2F
	template <> CVector2F Read( )
	{
		if( _CurPos + sizeof( CVector2F ) > Size() ) return CVector2F(0,0);
		_CurPos += sizeof( CVector2F );
		return *((CVector2F*)&_Buffer[_CurPos - sizeof( CVector2F )]);
	}
#endif

	template <> string Read<string>(){
#ifdef TITAN_ADDSTRING_NULLTERMINATED
		dword start = _CurPos;
		char tempChar = NULL;
		while(_CurPos <= _Size){
			tempChar = Read<char>();
			if(tempChar == 0){
				dword size = _CurPos - start;
				string Buffer = new char[size];
				memcpy_s(Buffer,size,&_Buffer[start], size);
				return Buffer;
			}
		}
		return NULL;
#else
		TITAN_ADDSTRING_STRLEN_TYPE length = Read<TITAN_ADDSTRING_STRLEN_TYPE>();
		string buffer = new char[length + 1];
		if( !buffer ) return NULL;
		memset(buffer, 0, length + 1);
		for(TITAN_ADDSTRING_STRLEN_TYPE i = 0; i < length; i++){
			buffer[i] = Read<char>();
		}
		return buffer;
#endif
	}

	byte* ReadBytes(dword length, bool AddNull = false){
		byte* val;
		if(AddNull)
			val = new byte[length+1];
		else
			val = new byte[length];

		for(dword i = 0; i < length; i++){
			val[i] = Read<byte>();
		}
		if(AddNull)
			val[length] = 0;
		return val;
	}

#if defined(TITAN_USING_ISC) || defined(TITAN_IS_ISC_SERVER)
	template <> CServerData* Read<CServerData*>(){
		CServerData* srv = new CServerData();
		srv->type = Read<byte>();
		srv->iscid = Read<word>();
		srv->id = Read<word>();
		srv->owner = Read<word>();
		srv->status = Read<word>();
		srv->name = Read<string>();
		srv->ip = Read<string>();
		srv->port = Read<dword>();
		srv->flags = Read<qword>();
		srv->currentusers = Read<dword>();
		srv->maxusers = Read<dword>();
		return srv;
	}
#endif

	bool eof(){
		if(_CurPos >= _Size){
			return true;
		}
		return false;
	}

	void Command( dword nCommand ){
		_Command = nCommand;
	}
	dword Command(){
		return _Command;
	}

	void Size( dword nSize ){
		_Size = nSize;
	}
	dword Size(){
		return _Size;
	}

	void HeaderSize( dword nSize ){
		_HeaderSize = nSize;
	}
	dword HeaderSize(){
		return _HeaderSize;
	}

	void Pos( dword nPos ){
		_CurPos = nPos;
	}
	dword Pos(){
		return _CurPos;
	}

	void Skip( dword nPos ){
		_CurPos += nPos;
	}

	void Seek( dword nPos ){
		_CurPos = nPos;
	}

	byte* Buffer(){
		return _Buffer;
	}

	byte* Data(){
		return (byte*)(_Buffer + _HeaderSize);
	}

protected:
	byte _Buffer[0x8000];

	dword _Size;
	dword _Command;
	dword _CurPos;
	dword _HeaderSize;
};