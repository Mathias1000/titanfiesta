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