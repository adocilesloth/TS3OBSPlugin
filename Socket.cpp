/*****************************
2014 <adocilesloth@gmail.com>
*****************************/
#include "TS3Plugin.h"
#include <sstream>

using namespace std;

bool ConnectToHost(int port, char* adrs, SOCKET& sock)
{
	WSADATA wsadata;
	int error = WSAStartup(0x0202, &wsadata);	//error on startup?

	if(error)
	{
		AppWarning(TEXT("error"));
		return false;
	}
	if (wsadata.wVersion != 0x0202)	//error check winsock version
    {
		AppWarning(TEXT("!= 0x0202"));
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
		AppWarning(TEXT("INVALID_SOCKET"));
        return false; //Couldn't create the socket
    }  

	if (connect(sock, (SOCKADDR *)&target, sizeof(target)) == SOCKET_ERROR) //connect
    {
		AppWarning(TEXT("SOCKET_ERROR"));
		wstringstream code;
		code << WSAGetLastError();
		AppWarning(code.str().c_str());
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