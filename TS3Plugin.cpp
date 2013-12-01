/********************************************************************************
Copyright (C) 2013 William Pearson <adocilesloth@gmail.com>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.
********************************************************************************/
#include "TS3Plugin.h"

#include <fstream>
#include <string>
#include <sstream>

#define WIN32_MEAN_AND_LEAN
#include <winsock2.h>

using namespace std;

SOCKET obs;
ofstream file;
ifstream settings;

bool LoadPlugin()
{
	AppWarning(TEXT("TS3Plugin Loaded"));
	//pluginLocale = new LocaleStringLookup;
	return true;
}

void UnloadPlugin()
{
	//delete pluginLocale;
}

//gives annoying errors that magically fix if this bit is not in.
/*CTSTR GetPluginName()
{
	//return PluginStr("Plugin.Name");
	return Str("TS3.Recording.Notifier");
}

CTSTR GetPluginDescription()
{
	//return PluginStr("Plugin.Description");
	return Str("Adds.*R*.before.TS3.nickname.when.recording.and.removes.it.once.recording.stops.");
}*/

void OnStartStream()
{
	char* adrs = "127.0.0.1";
	if(!ConnectToHost(25639, adrs))
	{
		return;
	}
	if (Communicate(1))
	{
		return;
	}
	CloseConnection();

	return;
}

void OnStopStream()
{
	char* adrs = "127.0.0.1";
	if(!ConnectToHost(25639, adrs))
	{
		return;
	}
	if(!Communicate(0))
	{
		return;
	}
	CloseConnection();

	return;
}

bool ConnectToHost(int port, char* adrs)
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

	SOCKADDR_IN target;				//Socket address information
    target.sin_family = AF_INET;	// address family Internet
    target.sin_port = htons (port); //Port to connect on
    target.sin_addr.s_addr = inet_addr (adrs); //Target IP

	obs = socket (AF_INET, SOCK_STREAM, IPPROTO_TCP); //Create socket
    if (obs == INVALID_SOCKET)
    {
        return false; //Couldn't create the socket
    }  

	if (connect(obs, (SOCKADDR *)&target, sizeof(target)) == SOCKET_ERROR) //connect
    {
		AppWarning(TEXT("Connection Failure: Check TS3 is running and ClientQuery Plugin is enabled"));
        return false; //Couldn't connect
    }
    else
	{
        return true; //Success
	}
}

void CloseConnection()
{
    //Close the socket if it exists
    if(obs)
	{
        closesocket(obs);
	}

    WSACleanup(); //Clean up Winsock
}

bool Communicate(int cont)
{
	int iResult;
	char *notify = "clientnotifyregister schandlerid=1 event=notifyclientnamefromuid\n";
	stringstream newname;
	newname << "clientupdate client_nickname=";
	char reci1[256];
	char reci2[256];
	char reci3[256];
	char reci4[256];

	//debug file, remove if not needed
	file.open("C:/Program Files (x86)/OBS/plugins/outfile.txt");

	//get settings file path
	string path = OBSGetPluginDataPath().CreateUTF8String();
	file << path << endl;
	//gt cluid and recording prefix from ts3.txt
	settings.open(path + "\\ts3.txt");
	string cluid;
	getline(settings, cluid);
	string rec;
	getline(settings, rec);
	//set up the getname call
	string tempgetname = "clientgetnamefromuid ";
	tempgetname.append(cluid);
	tempgetname.append("\n");
	const char *getname = tempgetname.c_str();
	//get the recording prefix
	rec = rec.substr(7, 10);

	iResult = recv(obs, reci1, 256 ,0);	//get TS3 Client...
	if (iResult == SOCKET_ERROR)
	{
		AppWarning(TEXT("First Recieve Failure"));
		return false;
	}

	iResult = send(obs, notify, (int)strlen(notify), 0);	//request notifyregister...
	if (iResult == SOCKET_ERROR)
	{
		AppWarning(TEXT("First Send Failure"));
		return false;
	}
	
	iResult = recv(obs, reci2, 256 ,0);	//recieve result: error id=0...
	if (iResult == SOCKET_ERROR)
	{
		AppWarning(TEXT("Second Recieve Failure"));
		return false;
	}

	iResult = send(obs, getname, (int)strlen(getname), 0);	//request clientnamefromuid...
	if (iResult == SOCKET_ERROR)
	{
		AppWarning(TEXT("Second Send Failure"));
		return false;
	}

	iResult = recv(obs, reci3, 256 ,0);	//recieve name
	if (iResult == SOCKET_ERROR)
	{
		AppWarning(TEXT("Third Recieve Failure"));
		return false;
	}

	//get name
	string identstart = "name=";
	string name = reci3;
	size_t startpos = name.find(identstart); //start of name
	name = name.substr(startpos+5 , 30);
	//get name end
	
	if(cont == 1)
	{
		if(name.substr(0, rec.length()) != rec)
		{
			name = name.substr(0, 30 - rec.length());
			newname << rec;
		}
	}
	else if(cont == 0)
	{
		if(name.substr(0, rec.length()) == rec)
		{
			name = name.substr(rec.length(), 30);
		}
	}

	newname << name << "\n";		//finish name set string
	const string tmp = newname.str();	//set name to string
	const char* recname = tmp.c_str();	//set name to char* so it can be sent

	iResult = send(obs, recname, (int)strlen(recname), 0);
	if (iResult == SOCKET_ERROR)
	{
		AppWarning(TEXT("Third Send Failure"));
		return false;
	}

	iResult = recv(obs, reci4, 256 ,0);
	if (iResult == SOCKET_ERROR)
	{
		AppWarning(TEXT("Fourth Recieve Failure"));
		return false;
	}

	file.close();
	settings.close();
	return true;
}
