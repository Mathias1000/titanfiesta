#include <stdio.h>
#include <windows.h>
#include <vector>

typedef unsigned __int8 byte;
typedef unsigned __int16 word;
typedef unsigned __int32 dword;
#include "CTitanFile.hpp"

class CSHNReader
{
public:
	struct SSHNColumn
	{
		char name[0x30];
		dword type;
		dword columnSize;
	};
	CSHNReader(){}
	~CSHNReader(){}

	void Open(char* path){
		CTitanFile fh(path, "rb");
		fh.Skip(0x28);
		rows = fh.Read<dword>();
		fh.Skip(4);
		dword cols = fh.Read<dword>();
		for(dword i = 0; i < cols; i++){
			SSHNColumn* curCol = reinterpret_cast<SSHNColumn*>(fh.ReadBytes(0x38));
			printf("Column Name: %s Type: %d Size: %d\n", curCol->name, curCol->type, curCol->columnSize);
			columns.push_back(curCol);
		}
		for(dword i = 0; i < rows; i++){
			word rowSize = fh.Read<word>();
			for(dword j = 0; j < columns.size(); j++){
				SSHNColumn* curCol = columns[j];
				switch(curCol->type){
					case 1:
						if(curCol->columnSize != 1){
							printf("curCol->Type == 1 && curCol->columnSize == %d\n", curCol->columnSize);
						}
						printf("%02x ", fh.Read<byte>());
					break;
					case 2:
						if(curCol->columnSize != 2){
							printf("curCol->Type == 2 && curCol->columnSize == %d\n", curCol->columnSize);
						}
						printf("%04x ", fh.Read<word>());
					break;
					case 3:
						printf("%08x ", fh.Read<dword>());
					break;
					case 9:
						printf("%s ", fh.ReadBytes(curCol->columnSize));
					break;
					case 11:
						printf("%08x ", fh.Read<dword>());
					break;
					case 18:
						printf("%08x ", fh.Read<dword>());
					break;
					case 26:
						printf("%s ", fh.ReadNullString());
					break;
					case 27:
						if(curCol->columnSize != 4){
							printf("curCol->Type == 27 && curCol->columnSize == %d\n", curCol->columnSize);
						}
						printf("%08x ", fh.Read<dword>());
					break;
					default:
						printf("\n====WARNING UNKNOWN TYPE %d====\n", curCol->type);
						fh.Skip(curCol->columnSize);
				}
			}
			printf("\n");
		}
		fh.Close();
	}
private:
	dword rows;
	std::vector<SSHNColumn*> columns;
};

int main(int argc, char** argv){
	if(argc < 2){
		return 0;
	}
	FILE* fh;
	fopen_s(&fh, argv[1], "rb");
	if(fh == NULL){
		printf("Could not open file\n");
		return 0;
	}
	fseek(fh, 0, SEEK_END);
	long int filesize = ftell(fh);
	fseek(fh, 0, SEEK_SET);
	unsigned char* data = new unsigned char[filesize];
	fread(data, filesize, 1, fh);
	fclose(fh);

	data += 0x24;
	filesize -= 0x24;
	unsigned char esp10 = data[filesize - 1];
	unsigned char dl;
	unsigned int eax = filesize;
	for(long int i = filesize - 1; i > 0; --i){
		dl = data[i];
		dl ^= (eax & 0xFF);
		eax &= ~0xFF;
		eax |= i & 0xFF;
		data[i] = dl;
		char tmp = eax & 0xff;
		short tmp2 = tmp * 0x0b;
		eax &= ~0xFFFF;
		eax |= *reinterpret_cast<unsigned short*>(&tmp2);
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

	{
		char tempPath[256];
		sprintf_s(tempPath, 256, "%s.dec", argv[1]);
		FILE* fh;
		fopen_s(&fh, tempPath, "wb");
		if(fh == NULL){
			printf("Could not open file\n");
			return 0;
		}
		fwrite(data, filesize, 1, fh);
		fclose(fh);
		
		CSHNReader reader;
		reader.Open(tempPath);
	}

	return 0;
}
