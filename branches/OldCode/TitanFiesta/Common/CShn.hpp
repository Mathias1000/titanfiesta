#pragma once

#define ReadDword() dataRead<dword>(data, &readPos);
#define ReadWord() dataRead<word>(data, &readPos);
#define ReadByte() dataRead<byte>(data, &readPos);
#define Skip(x) readPos += x;

class CShnColumn {
public:
	char name[0x30];
	dword type;
	dword columnSize;
};

class CShnData {
public:
	union {
		byte btData;
		word wData;
		dword dwData;
		char* strData;
	};
};

class CShnRow {
public:
	CShnRow():cells(NULL),colCount(0){}
	~CShnRow(){DEL2DARR(cells, colCount);}

	dword colCount;//so we can delete cells properly
	CShnData** cells;
};

class CShn {
public:
	CShn(){}
	~CShn(){}

	bool Open(char* path, int theIdMap = -1){
		FILE* fh;
		fopen_s(&fh, path, "rb");
		if(fh == NULL) return false;
		fseek(fh, 0, SEEK_END);
		long filesize = ftell(fh);
		fseek(fh, 0, SEEK_SET);
		byte* data = new byte[filesize];
		fread(data, filesize, 1, fh);
		fclose(fh);

		useIdMap = theIdMap;

		data += 0x24;
		filesize -= 0x24;
		byte esp10 = data[filesize - 1];
		byte dl;
		dword eax = filesize;
		for(long i = filesize - 1; i > 0; --i){
			dl = data[i];
			dl ^= (eax & 0xFF);
			eax &= ~0xFF;
			eax |= i & 0xFF;
			data[i] = dl;
			char tmp = eax & 0xff;
			short tmp2 = tmp * 0x0b;
			eax &= ~0xFFFF;
			eax |= *reinterpret_cast<word*>(&tmp2);
			dl = i & 0xff;
			dl &= 0x0f;
			dl += 0x55;
			dl ^= eax & 0xFF;
			dl ^= esp10;
			dl ^= 0xaa;
			eax &= ~0xFF;
			eax |= dl & 0xFF;
			esp10 = eax & 0xFF;
		}
		data -= 0x24;
		filesize += 0x24;

		dword readPos = 0x28;
		dword rowCount = ReadDword();
		Skip(4);
		dword colCount = ReadDword();

		for(dword i = 0; i < colCount; i++){
			CShnColumn* curCol = new CShnColumn();
			memcpy(curCol, data + readPos, 0x38);
			columns.push_back(curCol);
			readPos += 0x38;
		}

		for(dword i = 0; i < rowCount; i++){
			word rowSize = ReadWord();
			CShnRow* curRow = new CShnRow();
			curRow->colCount = colCount;
			curRow->cells = new CShnData*[curRow->colCount];
			for(dword j = 0; j < colCount; j++){
				CShnColumn* curCol = columns[j];
				CShnData* curData = new CShnData();
				switch(curCol->type){
					case 1:
						curData->btData = ReadByte();
					break;
					case 2:
						curData->wData = ReadWord();
					break;
					case 3:
						curData->dwData = ReadDword();
					break;
					case 9:
					{
						char* tmp = new char[curCol->columnSize + 1];
						memcpy(tmp, data + readPos, curCol->columnSize);
						tmp[curCol->columnSize] = 0;
						readPos += curCol->columnSize;
						curData->strData = tmp;
					}
					break;
					case 11:
						curData->dwData = ReadDword();
					break;
					case 18:
						curData->dwData = ReadDword();
					break;
					case 26:
					{
						//null terminated str
						dword len = strlen(reinterpret_cast<char*>(data + readPos));
						char* tmp = new char[len + 1];
						memcpy(tmp, data + readPos, len);
						tmp[len] = 0;
						readPos += len;
						curData->strData = tmp;
					}
					break;
					case 27:
						curData->dwData = ReadDword();
					break;
					default:
						Skip(curCol->columnSize);
						curData->dwData = 0;
				}
				curRow->cells[j] = curData;
			}
			rows.push_back(curRow);
			if(useIdMap != -1)
				idMap[curRow->cells[useIdMap]->wData] = curRow;
		}

		delete [] data;
		return true;
	}

	byte GetByte(dword row, dword column){
		return rows[row]->cells[column]->btData;
	}

	word GetWord(dword row, dword column){
		return rows[row]->cells[column]->wData;
	}

	dword GetDword(dword row, dword column){
		return rows[row]->cells[column]->dwData;
	}

	char* GetString(dword row, dword column){
		return rows[row]->cells[column]->strData;
	}

	byte GetByteId(word id, dword column){
		std::map<word, CShnRow*>::iterator cur = idMap.find(id);
		return (cur == idMap.end())?0:cur->second->cells[column]->btData;
	}

	word GetWordId(word id, dword column){
		std::map<word, CShnRow*>::iterator cur = idMap.find(id);
		return (cur == idMap.end())?0:cur->second->cells[column]->wData;
	}

	dword GetDwordId(word id, dword column){
		std::map<word, CShnRow*>::iterator cur = idMap.find(id);
		return (cur == idMap.end())?0:cur->second->cells[column]->dwData;
	}

	char* GetStringId(word id, dword column){
		std::map<word, CShnRow*>::iterator cur = idMap.find(id);
		return (cur == idMap.end())?0:cur->second->cells[column]->strData;
	}

private:
	template <typename T> inline T dataRead(byte* data, dword* position){
		(*position) += sizeof(T);
		return *reinterpret_cast<T*>(data + (*position) - sizeof(T));
	}

	std::vector<CShnColumn*> columns;
	std::vector<CShnRow*> rows;
	int useIdMap;
	std::map<word, CShnRow*> idMap;
};
