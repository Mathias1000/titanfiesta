/* Copyright (C) 2008, 2009 TitanFiesta Dev Team
 * Licensed under GNU GPL v3
 * For license details, see LICENCE in the root folder. */

class CSHN
{
public:
	struct SSHNColumn
	{
		SSHNColumn():showInHex(false){}
		~SSHNColumn(){}
		char name[0x30];
		dword type;
		dword columnSize;
		bool showInHex;
	};
	struct SSHNRowData
	{
		SSHNRowData():data(NULL){}
		~SSHNRowData(){DEL(data);}
		dword length;
		byte* data;
	};
	struct SSHNRow
	{
		SSHNRow(){}
		~SSHNRow(){DELVEC(data);}
		word rowSize;
		std::vector<SSHNRowData*> data;
	};
	CSHN(){}
	~CSHN(){
		DELVEC(rows);
		DELVEC(columns);
	}

	void Open(char* path){
		FILE* fh2;
		fopen_s(&fh2, path, "rb");
		if(fh2 == NULL) return;
		fseek(fh2, 0, SEEK_END);
		long int filesize = ftell(fh2);
		fseek(fh2, 0, SEEK_SET);
		unsigned char* data = new unsigned char[filesize];
		fread(data, filesize, 1, fh2);
		fclose(fh2);

		Decrypt(data + 0x24, filesize - 0x24);

		CTitanFile fh(data, filesize);
		fh.Skip(0x28);
		dword rowCount = fh.Read<dword>();
		fh.Skip(4);
		dword cols = fh.Read<dword>();
		for(dword i = 0; i < cols; i++){
			SSHNColumn* curCol = new SSHNColumn();
			fh.ReadBuffer(reinterpret_cast<byte*>(curCol), 0x38);
			curCol->showInHex = false;
			columns.push_back(curCol);
		}

		for(dword i = 0; i < rowCount; i++){
			SSHNRow* curRow = new SSHNRow();
			curRow->rowSize = fh.Read<word>();
			for(dword j = 0; j < columns.size(); j++){
				SSHNRowData* curData = new SSHNRowData();
				SSHNColumn* curCol = columns[j];
				switch(curCol->type){
					case 1:
						curData->length = 1;
						curData->data = new byte[curData->length];
						curData->data[0] = fh.Read<byte>();
					break;
					case 2:
						curData->length = 2;
						curData->data = new byte[curData->length];
						*reinterpret_cast<word*>(curData->data) = fh.Read<word>();
					break;
					case 3:
						curData->length = 4;
						curData->data = new byte[curData->length];
						*reinterpret_cast<dword*>(curData->data) = fh.Read<dword>();
					break;
					case 9:
						curData->length = curCol->columnSize;
						curData->data = fh.ReadBytes(curCol->columnSize);
					break;
					case 11:
						curData->length = 4;
						curData->data = new byte[curData->length];
						*reinterpret_cast<dword*>(curData->data) = fh.Read<dword>();
					break;
					case 18:
						curData->length = 4;
						curData->data = new byte[curData->length];
						*reinterpret_cast<dword*>(curData->data) = fh.Read<dword>();
					break;
					case 26:
						curData->data = reinterpret_cast<byte*>(fh.ReadNullString());
						curData->length = strlen(reinterpret_cast<char*>(curData->data));
					break;
					case 27:
						curData->length = 4;
						curData->data = new byte[curData->length];
						*reinterpret_cast<dword*>(curData->data) = fh.Read<dword>();
					break;
					default:
						curData->length = curCol->columnSize;
						curData->data = fh.ReadBytes(curData->length);
				}
				curRow->data.push_back(curData);
			}
			rows.push_back(curRow);
		}
		fh.Close();
		DEL(data);
	}
	std::vector<SSHNColumn*> columns;
	std::vector<SSHNRow*> rows;
private:
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
};