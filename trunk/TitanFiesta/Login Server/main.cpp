/* Copyright (C) 2008, 2009 TitanFiesta Dev Team
 * Licensed under GNU GPL v3
 * For license details, see LICENCE in the root folder. */

#include "main.h"
#include "CLoginServer.hpp"

int main(int argc, char* argv[]) 
{
	CTitanIniReader ini("LoginServer.ini");
	CLoginServer server;
	server.Config.BindIp = ini.GetString("IP","Login Server",NULL);
	server.Config.BindPort = ini.GetInt("Port","Login Server",9010);

	server.Config.ISCIp = ini.GetString("IP","ISC Server","127.0.0.1");
	server.Config.ISCPort = ini.GetInt("Port","ISC Server",1337);

	server.Start();
	return 0;
}