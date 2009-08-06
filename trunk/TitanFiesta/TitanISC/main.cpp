/* Copyright (C) 2008, 2009 TitanFiesta Dev Team
 * Licensed under GNU GPL v3
 * For license details, see LICENCE in the root folder. */

#include "main.h"
#include "CTitanISC.hpp"

int main(int argc, char* argv[]) 
{
	CTitanISC server;
	server.Config.BindIp = TITAN_DEFAULT_ISC_IP;
	server.Config.BindPort = TITAN_DEFAULT_ISC_PORT;

	server.Start();
	return 0;
}