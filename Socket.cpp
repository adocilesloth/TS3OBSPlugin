/*****************************
2014 <adocilesloth@gmail.com>
*****************************/
#include "TS3Plugin.h"

using namespace std;

bool ConnectToHost(int port, char* adrs, SOCKET& sock)
{
	WSADATA wsadata;
	int error = WSAStartup(0x0202, &wsadata);	//error on startup?

	if(error)
	{
		return false;
	}
	if (wsadata.wVersion != 0x0202)	//error check winsock version
    {
        WSACleanup(); //Clean up Winsock
        return false;
    }

	SOCKADDR_IN target;							//Socket address information
    target.sin_family = AF_INET;				//address family Internet
    target.sin_port = htons (port);				//Port to connect on
    target.sin_addr.s_addr = inet_addr (adrs);	//Target IP

	sock = socket (AF_INET, SOCK_STREAM, IPPROTO_TCP); //Create socket
    if (sock == INVALID_SOCKET)
    {
        return false; //Couldn't create the socket
    }  

	if (connect(sock, (SOCKADDR *)&target, sizeof(target)) == SOCKET_ERROR) //connect
    {
		AppWarning(TEXT("Connection Failure: Check TS3 is running and ClientQuery Plugin is enabled"));
        return false; //Couldn't connect
    }
    else
	{
        return true; //Success
	}
}

void CloseConnection(SOCKET& sock)
{
    //Close the socket if it exists
    if(sock)
	{
        closesocket(sock);
	}

    WSACleanup(); //Clean up Winsock
}