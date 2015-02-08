/*****************************
2015 <adocilesloth@gmail.com>
*****************************/
#ifndef SOCKETSRALL_H
#define SOCKETSRALL_H

#include "TS3Plugin.h"
#include <sstream>
#include <vector>

using namespace std;

class SendAll
{
public:
	bool send_all(SOCKET &sock, char *buffer, int length, int flag)
	{
		char *ptr = (char*)buffer;
		while(length > 0)
		{
			int i = send(sock, ptr, length, flag);
			if(i < 0)
			{
				return false;
			}
			ptr += i;
			length -= i;
		}
		return true;
	}

	bool send_all(SOCKET &sock, const char *buffer, int length, int flag)
	{
		const char *ptr = (const char*)buffer;
		while(length > 0)
		{
			int i = send(sock, ptr, length, flag);
			if(i < 0)
			{
				return false;
			}
			ptr += i;
			length -= i;
		}
		return true;
	}
};

class RecvAll
{
public:
	bool recv_all(SOCKET &sock, char *buffer, int length, int flag)
	{
		char *ptr = (char*)buffer;
		while(length > 0)
		{
			int i = recv(sock, ptr, length, flag);
			if(i < 0)
			{
				return false;
			}
			else if(i == 0)
			{
				break;
			}
			ptr += i;
			length -= i;
		}
		return true;
	}

	bool recv_all(SOCKET &sock, vector<char> &vbuffer, int length, int flag)
	{
		vector<char> vtemp;
		int lastPlace = 0;
		while(length > 0)
		{
			vtemp.resize(length);
			int i = recv(sock, &vtemp[0], length, flag);
			if(i < 0)
			{
				return false;
			}
			else if(i == 0)
			{
				break;
			}
			
			for(int j = 0; j < i; j++)
			{
				vbuffer[j + lastPlace] = vtemp[j];
			}

			lastPlace += i;
			length -= i;
			vtemp.clear();
		}
		return true;
	}

	bool recv_all(SOCKET &sock, char *buffer, int length, int flag, string endid)
	{
		stringstream incomming;
		int endpos = -1;
		char *ptr = (char*)buffer;
		while(length > 0)
		{
			int i = recv(sock, ptr, length, flag);
			if(i < 0)
			{
				return false;
			}
			else if(i == 0)
			{
				break;
			}
			ptr += i;
			length -= i;

			incomming << buffer;
			endpos = incomming.str().find(endid);
			if(endpos > 0)
			{
				break;
			}
			incomming.str("");

		}
		return true;
	}
};

#endif //SOCKETSRALL_H