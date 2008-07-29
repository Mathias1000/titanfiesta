class CTitanFile
{
public:
	CTitanFile() : _Open(false) {}
	CTitanFile(char* path, char* mode){ _Open = false; Open(path, mode); }
	~CTitanFile(){Close();}

	bool Open(char* path, char* mode){
		if(_Open) return false;

		fopen_s(&_fh, path, mode);
		if(_fh == NULL) return false;

		_Open = true;
		return true;
	}
	
	template <class T> void Add(T value){
		if(!_Open) return;

		fwrite((const void*)&value, sizeof(T), 1, _fh);
	}
		
	void Print(char * format, ...){
		if(!_Open) return;

		va_list args;
		va_start(args, format);
		vfprintf(_fh, format, args);
		va_end(args);
		fprintf(_fh, "\r\n");
	}
	
	void AddBytes(byte* value, dword size){
		if(!_Open) return;
		if(size == 0) return;

		fwrite(value, size, 1, _fh);
	}

	template <class T> void AddStringLen(char* value){
		if(!_Open) return;

		Add<T>(strlen(value));
		fwrite(value, strlen(value), 1, _fh);
	}

	template <class T> T Read(){
		if(!_Open) return 0;

		T value;
		fread(&value, sizeof(T), 1, _fh);
		if(Pos() > Size()){
			throw(1);
		}
		return value;
	}
	
	unsigned char* ReadBytes(dword bytes){
		if(!_Open) return 0;

		unsigned char* value = new unsigned char[bytes];
		fread(value, sizeof(unsigned char), bytes, _fh);
		return value;
	}

	char* ReadString(dword bytes){
		if(!_Open) return 0;

		char* value = new char[bytes+1];
		fread(value, sizeof(char), bytes, _fh);
		value[bytes] = 0;
		return value;
	}

	char* ReadNullString(){
		if(!_Open) return 0;

		char* value = new char[255];
		for(word i = 0; i < 255; i++){
			fread(&value[i], sizeof(char), 1, _fh);
			if(value[i] == 0) break;
		}
		return value;
	}

	void Skip(dword bytes){
		if(!_Open) return;

		fseek(_fh, bytes, SEEK_CUR);
	}

	void Seek(dword position){
		if(!_Open) return;

		fseek(_fh, position, SEEK_SET);
	}

	dword Size(){
		if(!_Open) return 0;
		
		dword position = ftell(_fh);
    fseek(_fh, 0, SEEK_END);
    dword size = ftell(_fh);
		fseek(_fh, position, SEEK_SET);
		return size;
	}

	dword Pos(){
		if(!_Open) return 0;

		return ftell(_fh);
	}

	void Rewind(){
		if(!_Open) return;
		rewind(_fh);
	}

	void Flush(){
		if(!_Open) return;
		fflush(_fh);
	}

	bool IsOpen(){ return _Open; }

	void Close(){
		if(!_Open) return;

		_Open = false;
		fclose(_fh);
	}
private:
	FILE* _fh;
	bool _Open;
};
