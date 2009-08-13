/* Copyright (C) 2008, 2009 TitanFiesta Dev Team
 * Licensed under GNU GPL v3
 * For license details, see LICENCE in the root folder. */

#include "main.h"
#include "CTitanISC.hpp"

int main(int argc, char* argv[]) 
{
	CTitanIniReader ini("ISCServer.ini");
	CTitanISC server;
	server.Config.BindIp = ini.GetString("IP","ISC Server",TITAN_DEFAULT_ISC_IP);
	server.Config.BindPort = ini.GetInt("Port","ISC Server",TITAN_DEFAULT_ISC_PORT);

	server.Start();
	return 0;
}