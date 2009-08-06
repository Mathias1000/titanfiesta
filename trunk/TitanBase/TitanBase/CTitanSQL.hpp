/* Copyright (C) 2008, 2009 TitanFiesta Dev Team
 * Licensed under GNU GPL v3
 * For license details, see LICENCE in the root folder. */

#pragma comment(lib,"libmysql.lib")
#include <mysql.h>

inline byte GetCharVal(char character){
	dword offset = 0;
	if(character <= 57){
		offset = 48;
	}else if(character >= 65){
		offset = 55;
	}else if(character >= 97){
		offset = 87;
	}
	return character - offset;
}

class CTitanSQL {
private:
	MYSQL _MySql;
	string _Server;
	string _Usermame;
	string _Password;
	string _Database;

public:
	CTitanSQL(string server = NULL, string username = NULL, string password = NULL, string database = NULL){
		_Server = server;
		_Usermame = username;
		_Password = password;
		_Database = database;
		mysql_init( &_MySql );
	}
	~CTitanSQL(){	}

	bool Connect(string server, string username, string password, string database){
		_Server = server;
		_Usermame = username;
		_Password = password;
		_Database = database;
		return (mysql_real_connect( &_MySql, _Server, _Usermame, _Password, _Database, 0, NULL, 0 ) != 0);
	}
	bool Connect(){
		return (mysql_real_connect( &_MySql, _Server, _Usermame, _Password, _Database, 0, NULL, 0 ) != 0);
	}

	string MakeSQLSafe( string sqlString ){
		dword bsize = (strlen(sqlString)+1)*2;
		string result = (string)malloc(bsize);
		if( !result ) return NULL;
		dword resultpos = 0;
		memset( result, 0, bsize );
		for(unsigned int i = 0; i < strlen(sqlString); i++){
			char cur = sqlString[i];
			if( cur == '"' || cur == '\'' || cur == ';' || cur == '=' || cur == '-' )
			{
				result[resultpos] = '\\';
				resultpos++;
			}
			result[resultpos] = cur;
			resultpos++;
		}
		result = (string)realloc( result, resultpos+1 );
		return result;
	}

	string MakeSQLSafe( string sqlString, dword sqlLength ){
		string result = (string)malloc(sqlLength * 2 + 1);
		if ( !result ) return NULL;
		mysql_real_escape_string(&_MySql, result, sqlString, sqlLength);

		return result;
	}

	MYSQL_RES* DoSQL(char *Format, ...){		
		string Buffer;
		int retval;
		int reqSize;
		va_list ap;

		va_start( ap, Format );
		reqSize = _vscprintf( Format, ap ) + 1;
		Buffer = new char[reqSize];

		vsprintf_s( Buffer, reqSize, Format, ap ); 
		va_end( ap );

 		retval = mysql_query( &_MySql, Buffer );

		if( retval != 0 ) Log( MSG_ERROR, "MySQL Query Error '%s'", mysql_error( &_MySql ) );
		return (retval==0)?(mysql_store_result( &_MySql )):(NULL);
	}

	dword ExecSQL(char *Format, ...){		
		string Buffer;
		int retval;
		int reqSize;
		va_list ap;

		va_start( ap, Format );
		reqSize = _vscprintf( Format, ap ) + 1;
		Buffer = new char[reqSize];

		vsprintf_s( Buffer, reqSize, Format, ap ); 
		va_end( ap );

 		retval = mysql_query( &_MySql, Buffer );

		if( retval != 0 ) Log( MSG_ERROR, "MySQL Query Error '%s'", mysql_error( &_MySql ) );
		
		return (retval==0)?(mysql_affected_rows(&_MySql)):(0);
	}

	dword LastInsertId() {
		return mysql_insert_id(&_MySql);
	}

	void QFree(MYSQL_RES*& result) {
		if (result != NULL) {
			mysql_free_result( result );
			result = NULL;
		}
	}

	char* EncodeBinary(void* data, dword datalen){
		char* retVal = new char[(datalen * 2) + 1];
		EncodeBinary(retVal, data, datalen);
		return retVal;
	}

	void EncodeBinary(char* encoded, void* voidData, dword datalen){
		dword curbit = 0;
		byte* data = (byte*)voidData;
		for(dword i = 0; i < datalen; i++){
			encoded[curbit] = "0123456789ABCDEF"[data[i]>>4];
			encoded[curbit+1] = "0123456789ABCDEF"[data[i]&0xf];
			curbit += 2;
		}
		encoded[curbit] = 0;
	}

	void DecodeBinary(char* encoded, void* voidData){
		dword curbyte = 0;
		byte* data = (byte*)voidData;
		for(dword i = 0; i < (dword)strlen(encoded); i+=2){		
			data[curbyte] = GetCharVal(encoded[i]) << 4;
			data[curbyte] += GetCharVal(encoded[i+1]);
			curbyte++;
		}
	}

	/*----------------*\
	| Variable Get/Set |
	\*----------------*/
	void Server( string server ){
		_Server = server;
	}
	string Server( ){
		return _Server;
	}

	void Username( string username ){
		_Usermame = username;
	}
	string Username( ){
		return _Usermame;
	}

	void Password( string password ){
		_Password = password;
	}
	string Password( ){
		return _Password;
	}

	void Database( string database ){
		_Database = database;
	}
	string Database( ){
		return _Database;
	}

	MYSQL* GetMySQL( ){
		return &_MySql;
	}
};