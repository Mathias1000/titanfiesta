#include "main.h"
#include "CGameServer.hpp"

bool CGameServer::OnServerReady(){
	CTitanIniReader ini("GameServer.ini");
	db = new CTitanSQL(ini.GetString("Server","MySQL","127.0.0.1"), ini.GetString("Username","MySQL","root"), ini.GetString("Password","MySQL",""), ini.GetString("Database","MySQL","titanfiesta"));
	if(!db->Connect()){
		Log(MSG_STATUS, "Failed to connect to MYSQL Server");
	}else{
		Log(MSG_STATUS, "Connected to MYSQL Server");
	}
	
	ServerData.flags = 0;
	ServerData.id = ini.GetInt("Id","Game Server",3);
	ServerData.ip = Config.BindIp;
	ServerData.iscid = 99;
	ServerData.name = ini.GetString("Name","Game Server","Game Server");
	ServerData.owner = ini.GetInt("Owner","Game Server",2);
	ServerData.port = Config.BindPort;
	ServerData.status = 9;
	ServerData.type = 3;

	CISCPacket pakout(TITAN_ISC_IDENTIFY);
	pakout.Add<CServerData*>( &ServerData );
	SendISCPacket( &pakout );

	return true;
}

void CGameServer::OnReceivePacket( CTitanClient* baseclient, CTitanPacket* pak ){
	CGameClient* thisclient = (CGameClient*)baseclient;
	Log(MSG_INFO,"Received packet: Command: %04x Size: %04x", pak->Command(), pak->Size());

	switch(pak->Command()){
		case 0x1801:
			if (thisclient->id == -1)
				PACKETRECV(pakUserLogin);
		break;
		default:
			pak->Pos(0);
			printf("Unhandled Packet, Command: %04x Size: %04x:\n", pak->Command(), pak->Size());
			for(dword i = 0; i < pak->Size(); i++)
				printf("%02x ", pak->Read<byte>());
			printf("\n");
		break;
	}
}

const static byte packet1802[238] = {
0xa3, 0x32, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0a, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x05, 0x00, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x04, 0x00, 
0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x12, 0x72, 0x00, 0x04, 0x00, 0x00, 0x00, 0x04, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x2e, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0x00, 
0x00, 0x00, 0x0b, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1e, 0x25, 0x00, 0x00, 0xb9, 0x0c, 0x00, 0x00 
};

PACKETHANDLER(pakUserLogin){
	char charname[0x13];
	charname[0x12] = 0;
	memcpy(charname, pak->Buffer() + 5, 0x12);

	thisclient->charname = db->MakeSQLSafe(charname);
	thisclient->loginid = pak->Get<word>(0x03, 0);
	if(_strcmpi(thisclient->charname, charname)){
		Log(MSG_DEBUG, "MySql Safe login %s != %s", thisclient->charname, charname);
		return false;
	}

	Log(MSG_DEBUG, "Charname: %s, Unique Id: %d", thisclient->charname, thisclient->loginid);

	MYSQL_RES* result = db->DoSQL("SELECT u.`id`,u.`username`,u.`loginid`,u.`accesslevel`,u.`lastslot`,c.`id` FROM `users` AS `u`, `characters` AS `c` WHERE c.`charname`='%s' AND u.`username`=c.`owner`", thisclient->charname);
	if(!result || mysql_num_rows(result) != 1){
		Log(MSG_DEBUG, "SELECT returned bollocks");
		goto authFail;
	}
	
	MYSQL_ROW row = mysql_fetch_row(result);
	if(atoi(row[2]) != thisclient->loginid){
		Log(MSG_DEBUG, "Incorrect loginid");
		thisclient->id = -1;
		goto authFail;
	}

	thisclient->id = atoi(row[0]);
	thisclient->username = row[1];
	thisclient->accesslevel = atoi(row[3]);
	thisclient->lastslot = atoi(row[4]);
	thisclient->charid = atoi(row[5]);

	if(thisclient->accesslevel < 1){
		Log(MSG_DEBUG, "thisclient->accesslevel < 1");
		thisclient->id = -1;
		goto authFail;
	}
	db->QFree(result);

	{	// Char Info
		CPacket pakout(0x1038);
			pakout.Add<word>(thisclient->charid);
			pakout.Add<word>(0x000f);
			pakout.AddFixLenStr(thisclient->username, 0x10);
			pakout.Add<byte>(thisclient->lastslot); // Slot
			pakout.Add<byte>(0x01); // Level
			pakout.Add<qword>(0x00); // Total Exp
			pakout.Add<dword>(0x00); // Unk
			pakout.Add<word>(0x000e); // 0x15 for Ex
			pakout.Add<word>(0x000a); // 0x21 for Ex
			pakout.Add<dword>(0x002e); // HP
			pakout.Add<dword>(0x0020); // SP
			pakout.Add<dword>(0x0000); // Fame
			pakout.Add<qword>(0x1d1a); // Money
			pakout.AddFixLenStr("Rou", 0x0C); // Cur map
			pakout.Add<dword>(0x251e); // Pos X
			pakout.Add<dword>(0x0cb9); // Pos Y
			pakout.Add<byte>(0x5a); // No farking clue 0x5A for both
			pakout.Add<byte>(0x00); // STR+
			pakout.Add<byte>(0x00); // END+
			pakout.Add<byte>(0x00); // DEX+
			pakout.Add<byte>(0x00); // INT+
			pakout.Add<byte>(0x00); // SPR+
			pakout.Fill<byte>(0x00, 0x0D); // Unk
		SendPacket(thisclient, &pakout);
	}

	{	
		CPacket pakout(0x1039);
			pakout.Add<dword>(0x0121);
		SendPacket(thisclient, &pakout);
	}

	{	// Quests?
		CPacket pakout(0x103a);
			pakout.Add<word>(thisclient->charid);
			pakout.Add<word>(0x0f);
			pakout.Add<byte>(0x01);
			pakout.Add<byte>(0x00); // Count
		SendPacket(thisclient, &pakout);
	}

	{	// Basic Action?
		CPacket pakout(0x103b);
			pakout.Add<word>(thisclient->charid);
			pakout.Add<word>(0x0f);
			pakout.Add<word>(0x00);
		SendPacket(thisclient, &pakout);
	}

	{	// Active Skill list
		CPacket pakout(0x103d);
		pakout.Add<byte>(0x00); // Unk
		pakout.Add<word>(thisclient->charid); // Char ID
		pakout.Add<word>(0x000f); // Unk
		pakout.Add<word>(0x01); // Num of skills
		{ // For num of skills
			pakout.Add<word>(0x18D8); // Skill ID [Inferno01]
			pakout.Add<word>(0x0000); // unk
			pakout.Add<word>(0x0000); // Unk
			pakout.Add<word>(0x0000); // Empowerment (dmg | (sp << 4) | (time << 8) | (cool << 12)
			pakout.Add<word>(0x000F); // Uses
			pakout.Add<word>(0x0000); // Unk
		}
		SendPacket(thisclient, &pakout);
	}

	{	// Passive skill list
		CPacket pakout(0x103e);
			pakout.Add<word>(0x00);
		SendPacket(thisclient, &pakout);
	}

	{	// Inventory
		CPacket pakout(0x1047);
			pakout.Add<byte>(0x00); // Count
			pakout.Add<byte>(0x09); // Type
		SendPacket(thisclient, &pakout);
	}

	{	// Equips
		CPacket pakout(0x1047);
			pakout.Add<byte>(0x00); // Count
			pakout.Add<byte>(0x08); // Type
		SendPacket(thisclient, &pakout);
	}

	{	// House
		CPacket pakout(0x1047);
			pakout.Add<byte>(0x01); // Count
			pakout.Add<byte>(0x0C); // Type
			pakout.Add<byte>(0x08); // Unk
			pakout.Add<byte>(0x00); // Unk
			pakout.Add<byte>(0xC0); // Unk
			{ // For Count
				pakout.Add<word>(0x7918); // ItemID [Mushroom House]
				pakout.Add<byte>(0xFF); // Quantity
				pakout.Add<byte>(0xEC); // Unk
				pakout.Add<byte>(0xBB); // Slot
				pakout.Add<byte>(0x76); // Sep
			}
		SendPacket(thisclient, &pakout);
	}

	{
		CPacket pakout(0x1049);
			pakout.Add<word>(0x00);
			pakout.Add<word>(0x00);
			pakout.Add<word>(0x00);
		SendPacket(thisclient, &pakout);
	}

	{
		CPacket pakout(0x104A);
			pakout.Add<word>(0x00);
		SendPacket(thisclient, &pakout);
	}

	{
		CPacket pakout(0x1048);
			pakout.Add<word>(0xFF);
		SendPacket(thisclient, &pakout);
	}

	{
		CPacket pakout(0x1802);
			pakout.AddBytes((byte*)packet1802, 238);
		SendPacket(thisclient, &pakout);
	}



	return true;
authFail:
	{
		CPacket pakout(0x0C09);
		pakout.Add<word>(0x44);
		SendPacket(thisclient, &pakout);
	}
	db->QFree(result);
	return true;
}

void CGameServer::ReceivedISCPacket( CISCPacket* pak ){
	Log(MSG_INFO,"Received ISC Packet: Command: %04x Size: %04x", pak->Command(), pak->Size());
	switch(pak->Command()){
		case TITAN_ISC_SETISCID:
		{
			ServerData.iscid = pak->Read<word>();
		}
		break;
		case TITAN_ISC_IDENTIFY:
		case TITAN_ISC_REMOVE:
		case TITAN_ISC_UPDATEUSERCNT:
		break;
	}
}
