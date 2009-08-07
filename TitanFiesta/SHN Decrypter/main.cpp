/* Copyright (C) 2008, 2009 TitanFiesta Dev Team
 * Licensed under GNU GPL v3
 * For license details, see LICENCE in the root folder. */

#include <stdio.h>
#include <windows.h>
#include <vector>

typedef unsigned __int8 byte;
typedef unsigned __int16 word;
typedef unsigned __int32 dword;
#include "CTitanFile.hpp"

std::string ReplaceCharInString(  
    const std::string & source, 
    char charToReplace, 
    const std::string replaceString 
    ) 
{ 
    std::string result; 
 
    // For each character in source string: 
    const char * pch = source.c_str(); 
    while ( *pch != '\0' ) 
    { 
        // Found character to be replaced? 
        if ( *pch == charToReplace ) 
        { 
            result += replaceString; 
        } 
        else 
        { 
            // Just copy original character 
            result += (*pch); 
        } 
 
        // Move to next character 
        ++pch; 
    } 
 
    return result; 
} 
 
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

	void Open(char* path, char* tablename){
		CTitanFile fh(path, "rb");
		fh.Skip(0x28);
		rows = fh.Read<dword>();
		fh.Skip(4);
		dword cols = fh.Read<dword>();
		printf("CREATE TABLE `%s` (\n", tablename);
		for(dword i = 0; i < cols; i++){
			SSHNColumn* curCol = reinterpret_cast<SSHNColumn*>(fh.ReadBytes(0x38));
			printf("`%s`", curCol->name);
			switch(curCol->type){
				case 1:
					printf(" tinyint(1) NOT NULL");
				break;
				case 2:
					printf(" smallint(2) UNSIGNED NOT NULL");
				break;
				case 3:
					printf(" int(4) NOT NULL");
				break;
				case 9:
					printf(" char(%d) NOT NULL", curCol->columnSize);
				break;
				case 11:
					printf(" int(4) NOT NULL");
				break;
				case 18:
					printf(" int(4) NOT NULL");
				break;
				case 26:
					printf(" varchar(255) NOT NULL");
				break;
				case 27:
					printf(" int(4) NOT NULL");
				break;
			}
			if(i < cols - 1)
				printf(",\n");
			else
				printf("\n");
			columns.push_back(curCol);
		}
		printf(") ENGINE=MyISAM  DEFAULT CHARSET=utf8 ;\n\n");

		const int rowLimit = 100;
		for(dword i = 0; i < rows; i++){
			word rowSize = fh.Read<word>();
			if(i % rowLimit == 0)
				printf("INSERT INTO `%s` VALUES ", tablename);
			printf(" (", tablename);
			for(dword j = 0; j < columns.size(); j++){
				SSHNColumn* curCol = columns[j];
				switch(curCol->type){
					case 1:
						printf("%d", fh.Read<byte>());
					break;
					case 2:
						printf("%d", fh.Read<word>());
					break;
					case 3:
						printf("%d", fh.Read<dword>());
					break;
					case 9:
					{
						char* tmp = fh.ReadString(curCol->columnSize);
						std::string curStr;
						curStr.assign(tmp);
						curStr = ReplaceCharInString(curStr, '\'', "\\'");
						printf("'%s'", curStr.c_str());
						delete [] tmp;
					}
					break;
					case 11:
						printf("%d", fh.Read<dword>());
					break;
					case 18:
						printf("%d", fh.Read<dword>());
					break;
					case 26:
					{
						char* tmp = fh.ReadNullString();
						std::string curStr;
						curStr.assign(tmp);
						curStr = ReplaceCharInString(curStr, '\'', "\\'");
						printf("'%s'", curStr.c_str());
						delete [] tmp;
					}
					break;
					case 27:
						printf("%d", fh.Read<dword>());
					break;
					default:
						fh.Skip(curCol->columnSize);
				}
				
				if(j < cols - 1)
					printf(", ");
				else
					printf(" ");
			}
			if(i >= rows - 1 || (i % rowLimit) >= rowLimit - 1)
				printf(");\n\n");
			else
				printf("),\n");
		}
		fh.Close();
	}
private:
	dword rows;
	std::vector<SSHNColumn*> columns;
};

unsigned char* ShnDecrypt(unsigned char* data, long int filesize) {
	byte key = filesize & 0xFF;

	for(long int i = filesize - 1; i > 0; --i){
		data[i] ^= key;
		byte tmp1 = ( 0x0b * (byte)i );
		byte tmp2 = ( i & 0x0f ) + 0x55;
		key ^= (tmp1 ^ tmp2) ^ 0xaa;
	}
	return data;
}

int main(int argc, char** argv){
	if(argc < 3){
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

	ShnDecrypt(data + 0x24, filesize - 0x24);

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
		reader.Open(tempPath, argv[2]);
	}

	return 0;
}
