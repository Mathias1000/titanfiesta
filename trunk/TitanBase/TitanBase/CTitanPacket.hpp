class CTitanPacket
{
public:
	CTitanPacket(dword command = 0){
		//_Buffer = (byte*)malloc(DEFAULT_PACKET_SIZE);
		_Size = PACKET_HEADER_SIZE;
		_Command = command;
		_CurPos = PACKET_HEADER_SIZE;
		//_CurBufSize = DEFAULT_PACKET_SIZE;
	}
	CTitanPacket(byte* myBuffer,bool iAmHereToShowThatMyBufferIsAPointerNotAdWord){
		//_Buffer = myBuffer;
		_Size = PACKET_HEADER_SIZE;
		_Command = 0;
		_CurPos = PACKET_HEADER_SIZE;
	}
	~CTitanPacket(){
		/*if(_Buffer != NULL){
			free(_Buffer);
			_Buffer = NULL;
		}*/
	}

	template <typename T> void Fill( T val, dword count ){
		for(dword i = 0; i < count; i++){
			Add<T>(val);
		}
	}

	template <typename T> void Add( T val )
	{
		/*if((_Size + sizeof( T )) > _CurBufSize){
			_CurBufSize += 0x100;
			if((_Size + sizeof( T )) > _CurBufSize){
				_CurBufSize = _Size + sizeof( T );
			}
			_Buffer = (byte*)realloc(_Buffer, _CurBufSize);
		}*/
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
		for(dword i = 0; i < strlen(val); i++)
			Add<byte>((byte)val[i]);
	}

	void AddBytes(byte* val, dword len){
		for(dword i = 0; i < len; i++)
			Add<byte>((byte)val[i]);
	} 
	
	void AddFixLenStr(char* str, int len){
		int strleng = strlen(str);
		if(strleng > len){
			for(int i = 0; i < len; i++)
				Add<byte>((byte)str[i]);
			return;
		}
		for(int i = 0; i < strleng; i++)
			Add<byte>((byte)str[i]);
		int extra = len - strleng;
		if(extra > 0)
			Fill<byte>(0, extra);
	}

#ifdef TITAN_USING_ISC
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
		*((T*)&_Buffer[pos + offset]) = val;
	}

	template <typename T> T Get( word pos, word offset = PACKET_HEADER_SIZE )
	{
		return *((T*)&_Buffer[pos + offset]);
	}

	template <typename T> T Read( )
	{
		if( _CurPos + sizeof( T ) > Size() ) return (T)0;
		_CurPos += sizeof( T );
		return *((T*)&_Buffer[_CurPos - sizeof( T )]);
	}

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

	/*void Buffer( byte* nBuffer ){
		_Buffer = nBuffer;
	}*/
	byte* Buffer(){
		return _Buffer;
	}
protected:
	byte _Buffer[1024];
	dword _Size;
	dword _Command;
	dword _CurPos;

	//dword _CurBufSize;
};