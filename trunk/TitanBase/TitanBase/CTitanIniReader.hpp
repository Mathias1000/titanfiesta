#ifndef __CTITANFILE_HPP__
#include "CTitanFile.hpp"
#endif

class CTitanIniEntry
{
public:
	string section;
	string key;
	string value;
};

class CTitanIniReader
{
private:
	CTitanFile* fh;
	string currentSection;
	std::vector<CTitanIniEntry*> entries;

public:
	CTitanIniReader(){
		currentSection = NULL;
		fh = NULL;
	}
	CTitanIniReader( string fileName ){
		currentSection = NULL;
		fh = NULL;
		ReadINI(fileName);
	}
	void ReadINI( string fileName ){
		entries.clear();
		if(currentSection != NULL){
			delete[] currentSection;
		}
		if(fh != NULL){
			delete fh;
		}
		fh = new CTitanFile( fileName, FM_READ );
		string buffer;
		while( buffer = fh->ReadLn() )
		{
			//Remove leading whitespaces
			word i;
			for(i = 0; i < strlen(buffer); i++){
				if(buffer[i] != ' '){
					buffer = &buffer[i];
					break;
				}
			}

			if(i == strlen(buffer)) continue;
			if(buffer[0] == '#') continue;
			if(buffer[0] == 0x0a) continue;
			if(buffer[0] == 0x0d) continue;
			if(strlen(buffer) == 0) continue;

			if(buffer[0] == '['){
				currentSection = new char[ strlen(buffer) ];
				strcpy_s(currentSection, strlen(buffer), &buffer[1]);
				for(unsigned short i = 0; i < strlen(currentSection); i++){
					if(currentSection[i] == ']'){
						currentSection[i] = 0;
						break;
					}
				}
			}else{
				string value = strchr(buffer, '=') + 1;
				string key = buffer;
				key[value - key - 1] = 0;
				for(i = 0; i < strlen(key); i++){
					if(key[i] == 0) break;

					if(key[i] == ' '){
						key[i] = 0;
						break;
					}
				}
				for(i = 0; i < strlen(value); i++){
					if(value[i] != ' '){
						value = &value[i];
						break;
					}
				}
				for(i = strlen(value); i > 0; i--){
					if(value[i] == 0) break;

					if(value[i] == ' '){
						value[i] = 0;
						break;
					}
				}
				if(value[0] == '"'){
					value = &value[1];
				}
				if(value[strlen(value)-1] == '"'){
					value[strlen(value)-1] = 0;
				}

				CTitanIniEntry* thisEntry = new CTitanIniEntry();
				thisEntry->section = currentSection;

				thisEntry->key = new char[ strlen(key) + 1];
				strcpy_s(thisEntry->key, strlen(key) + 1, key);

				thisEntry->value = new char[ strlen(value) + 1 ];
				strcpy_s(thisEntry->value, strlen(value) + 1, value);

				entries.push_back( thisEntry );
			}

			delete[] buffer;
		}
		fh->Close();
		delete fh;
	}

	bool GetBool( string key, string section = NULL, bool defaultvalue = NULL ){
		bool doRet;
		for(dword i = 0; i < entries.size(); i++){
			CTitanIniEntry* thisEntry = entries[i];
			doRet = false;
			if(!_strcmpi(thisEntry->key, key)){
				if((section == NULL) || (thisEntry->section == NULL)) continue;
				if((section == NULL) && (thisEntry->section == NULL)) doRet = true;
				if(!_strcmpi(thisEntry->section, section)) doRet = true;
				if(doRet){
					if(strlen(thisEntry->value) > 1){
						if(!_strcmpi(thisEntry->value, "true")) return true;
						if(!_strcmpi(thisEntry->value, "false")) return false;
					}else{
						return (atoi(thisEntry->value)==1);
					}
				}
			}
		}
		return defaultvalue;
	}

	dword GetInt( string key, string section = NULL, dword defaultvalue = NULL ){
		for(dword i = 0; i < entries.size(); i++){
			CTitanIniEntry* thisEntry = entries[i];
			if(!_strcmpi(thisEntry->key, key)){
				if((section == NULL) || (thisEntry->section == NULL)) continue;
				if((section == NULL) && (thisEntry->section == NULL)) return atoi(thisEntry->value);

				if(!_strcmpi(thisEntry->section, section)) return atoi(thisEntry->value);
			}
		}
		return defaultvalue;
	}

	dword GetHexInt( string key, string section = NULL, dword defaultvalue = NULL ){
		for(dword i = 0; i < entries.size(); i++){
			CTitanIniEntry* thisEntry = entries[i];
			if(!_strcmpi(thisEntry->key, key)){
				if((section == NULL) || (thisEntry->section == NULL)) continue;
				if((section == NULL) && (thisEntry->section == NULL)) return strtoul(thisEntry->value, NULL, 16);
				if(!_strcmpi(thisEntry->section, section)) return strtoul(thisEntry->value, NULL, 16);
			}
		}
		return defaultvalue;
	}

	string GetString( string key, string section = NULL, string defaultvalue = NULL ){
		for(dword i = 0; i < entries.size(); i++){
			CTitanIniEntry* thisEntry = entries[i];
			if(!_strcmpi(thisEntry->key, key)){
				if((section == NULL) && (thisEntry->section == NULL)) return thisEntry->value;
				if((section == NULL) || (thisEntry->section == NULL)) continue;

				if(!_strcmpi(thisEntry->section, section)) return thisEntry->value;
			}
		}
		return defaultvalue;
	}
};
