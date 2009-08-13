/* Copyright (C) 2008, 2009 TitanFiesta Dev Team
 * Licensed under GNU GPL v3
 * For license details, see LICENCE in the root folder. */

#pragma once

#include <map>
#include <vector>
#include <string>

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
	~CShn(){
		// Properly delete data.
		for (dword i = 0; i < rows.size(); i++) {
			for (dword j = 0; j < columns.size(); j++) {
				// Free memory used in char arrays
				if (columns[j]->type == 9 || columns[j]->type == 26) {
					delete[] rows[i]->cells[j]->strData;
					rows[i]->cells[j]->strData = NULL;
				}
			}
			// Free memory used by row
			delete rows[i];
			rows[i] = NULL;
		}
		for (dword j = 0; j < columns.size(); j++) {
			// Free memory used by column
			delete columns[j];
			columns[j] = NULL;
		}
		columns.clear();
		rows.clear();
	}

	bool Open(char* path, int theIdMap = -1, int theStrMap = -1){
		FILE* fh;
		fopen_s(&fh, path, "rb");
		if(fh == NULL) return false;
		fseek(fh, 0, SEEK_END);
		long filesize = ftell(fh);
		fseek(fh, 0, SEEK_SET);
		byte* data = new byte[filesize];
		fread(data, filesize, 1, fh);
		fclose(fh);

		// Apparently this tells you if it's an SHN
		if (data[0x1C] != 0x01) {
			delete[] data;
			return false;
		}

		useIdMap = theIdMap;
		useStrMap = theStrMap;

		Decrypt(data + 0x24, filesize - 0x24);

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
			// Setup the Map if needed. Can be by either an Id, or a String
			if(useIdMap != -1)
				idMap[curRow->cells[useIdMap]->wData] = curRow;
			if (useStrMap != -1 && (columns[useStrMap]->type == 26 || columns[useStrMap]->type == 9))
				strMap.insert( std::make_pair( std::string(curRow->cells[useStrMap]->strData), curRow ) );
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

	dword ColCount() {
		return columns.size();
	}

	dword RowCount() {
		return rows.size();
	}

	byte ColType(dword column) {
		if (column < columns.size())
			return columns[column]->type;
		else
			return 0;
	}

	CShnColumn* Column(dword column) {
		if (column < columns.size())
			return columns.at(column);
		else
			return NULL;
	}

	CShnRow* Row(dword row) {
		if (row < rows.size())
			return rows.at(row);
		else
			return NULL;
	}

	CShnRow* RowByString(char* str) {
		std::map<std::string, CShnRow*>::iterator cur = strMap.find(std::string(str));
		return (cur == strMap.end()) ? NULL : cur->second;
	}

	CShnRow* RowById(word id) {
		std::map<word, CShnRow*>::iterator cur = idMap.find(id);
		return (cur == idMap.end()) ? NULL : cur->second;
	}

private:
	template <typename T> inline T dataRead(byte* data, dword* position){
		(*position) += sizeof(T);
		return *reinterpret_cast<T*>(data + (*position) - sizeof(T));
	}

	unsigned char* Decrypt(unsigned char* data, long int filesize) {
		byte key = filesize & 0xFF;

		for(long int i = filesize - 1; i > 0; --i){
			data[i] ^= key;
			byte tmp1 = ( 0x0b * (byte)i );
			byte tmp2 = ( i & 0x0f ) + 0x55;
			key ^= (tmp1 ^ tmp2) ^ 0xaa;
		}
		return data;
	}

	std::vector<CShnColumn*> columns;
	std::vector<CShnRow*> rows;
	int useIdMap;
	int useStrMap;
	std::map<word, CShnRow*> idMap;
	std::map<std::string, CShnRow*> strMap;
};
