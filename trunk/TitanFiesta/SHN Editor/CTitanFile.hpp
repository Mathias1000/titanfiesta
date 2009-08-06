/* Copyright (C) 2008, 2009 TitanFiesta Dev Team
 * Licensed under GNU GPL v3
 * For license details, see LICENCE in the root folder. */

class CTitanFile
{
public:
	CTitanFile() : _Open(false), data(NULL) {}
	CTitanFile(char* path, char* mode):data(NULL){ _Open = false; Open(path, mode); }
	CTitanFile(byte* buffer, int bufferSize){ data = buffer; dataSize = bufferSize; dataPos = 0; _Open = true; }
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
		if(data == NULL){
			fread(&value, sizeof(T), 1, _fh);
		}else{
			memcpy(&value, data + dataPos, sizeof(T));
			dataPos += sizeof(T);
		}
		return value;
	}
	
	unsigned char* ReadBytes(dword bytes){
		if(!_Open) return 0;
		unsigned char* value = new unsigned char[bytes];
		if(data == NULL){
			fread(value, sizeof(unsigned char), bytes, _fh);
		}else{
			memcpy(value, data + dataPos, bytes);
			dataPos += bytes;
		}
		return value;
	}
	
	void ReadBuffer(byte* value, dword bytes){
		if(!_Open) return;
		if(data == NULL){
			fread(value, sizeof(unsigned char), bytes, _fh);
		}else{
			memcpy(value, data + dataPos, bytes);
			dataPos += bytes;
		}
	}


	char* ReadString(dword bytes){
		if(!_Open) return 0;
		char* value = new char[bytes+1];
		if(data == NULL){
			fread(value, sizeof(char), bytes, _fh);
		}else{
			memcpy(value, data + dataPos, bytes);
			dataPos += bytes;
		}
		value[bytes] = 0;
		return value;
	}

	char* ReadNullString(){
		if(!_Open) return 0;

		char* value = new char[255];
		if(data == NULL){
			for(word i = 0; i < 255; i++){
				fread(&value[i], sizeof(char), 1, _fh);
				if(value[i] == 0) break;
			}
		}else{
			strcpy_s(value, 255, reinterpret_cast<char*>(data+dataPos));
			dataPos += strlen(value);
		}
		return value;
	}

	void Skip(dword bytes){
		if(!_Open) return;

		if(data == NULL){
			fseek(_fh, bytes, SEEK_CUR);
		}else{
			dataPos += bytes;
		}
	}

	void Seek(dword position){
		if(!_Open) return;

		if(data == NULL){
			fseek(_fh, position, SEEK_SET);
		}else{
			dataPos = position;
		}
	}

	dword Size(){
		if(!_Open) return 0;
		if(data != NULL) return dataSize;
		
		dword position = ftell(_fh);
    fseek(_fh, 0, SEEK_END);
    dword size = ftell(_fh);
		fseek(_fh, position, SEEK_SET);
		return size;
	}

	dword Pos(){
		if(!_Open) return 0;

		if(data == NULL)
			return ftell(_fh);
		else
			return dataPos;
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
		if(data == NULL)
			fclose(_fh);
		else
			data = NULL;
	}
private:
	FILE* _fh;
	bool _Open;

	byte* data;
	int dataSize;
	int dataPos;
};
