/*****************************
2014 <adocilesloth@gmail.com>
*****************************/
#include "TS3Plugin.h"
#include "ending.h"

#include <string>
#include <sstream>
#include <fstream>
#include <codecvt>

using namespace std;

SOCKET overlay;
ending close;

//ofstream file;

void ShutdownOverlay()
{
	close.now();
	return;
}

void ResetOverlay()
{
	close.nolonger();
	return;
}

void RunOverlay(char* adrs)
{
	std::locale loc(std::locale::classic(), new codecvt_utf8<wchar_t>);
	//file.imbue(loc);

	int iResult;
	char reci1[256];
	char reci2[64];
	char reci3[4096];
	memset(reci3, 0, 4096);

	char* whoami = "whoami\n";

	//string location
	string identstart;
	string identend;
	string talkstart = "client_flag_talking=";
	size_t startpos;
	size_t endpos;
	string tempstr;

	//useful things
	string cid;
	string list;
	string name[10];
	bool talk[10] = {0};
	bool skipname = false;
	bool discon = true;
	bool noserv = true;

	//File stuff
	wstring path = OBSGetPluginDataPath().Array();
	path.append(L"\\Overlay.txt");
	wofstream fOverlay;
	fOverlay.imbue(loc);

	AppWarning(TEXT("Overlay: Started"));

	//Overlay Loop
	while(!close.state())
	{
		//connect
		if(!ConnectToHost(25639, adrs, overlay))
		{
			if(discon)	//if initial disconnection
			{
				AppWarning(TEXT("Overlay: Connection Failure: Check TS3 is running and ClientQuery Plugin is enabled"));
				discon = false;
			}
			fOverlay.open(path);		//empty client list text file if not connected
			fOverlay.close();
			goto skip;
		}

		discon = true;

		Sleep(5);				//to allow full mesage to be created
		iResult = recv(overlay, reci1, 256 ,0);	//get TS3 Client...
		if (iResult == SOCKET_ERROR)
		{
			AppWarning(TEXT("Overlay: First Recieve Failure"));
			CloseConnection(overlay);
			goto skip;
		}

		//get cid
		iResult = send(overlay, whoami, (int)strlen(whoami), 0);	//send whoami
		if (iResult == SOCKET_ERROR)
		{
			AppWarning(TEXT("Overlay: whoami Send Failure"));
			CloseConnection(overlay);
			goto skip;
		}
		Sleep(1);								//let message be fully sent
		iResult = recv(overlay, reci2, 64 ,0);	//get whoami
		if (iResult == SOCKET_ERROR)
		{
			AppWarning(TEXT("Overlay: whoami Recieve Failure"));
			CloseConnection(overlay);
			goto skip;
		}
		tempstr = reci2;
		if(tempstr.substr(0, 13) == "error id=1794")
		{
			if(noserv)	//if initial disconnection
			{
				AppWarning(TEXT("Overlay: Not Connected to TS3 Server"));
				noserv = false;
			}
			CloseConnection(overlay);
			fOverlay.open(path);			//empty client list text file if not on server
			fOverlay.close();
			goto skip;
		}

		noserv = true;

		identstart = "cid=";
		identend = "\n";
		cid = reci2;
		startpos = cid.find(identstart);
		if(startpos == -1)
		{
			goto skip;
		}
		endpos = cid.find(identend);
		if(endpos < 0)
		{
			goto skip;
		}
		cid = cid.substr(startpos, endpos - startpos);
		
		//set up channelclientlist
		tempstr = "channelclientlist ";
		tempstr.append(cid);
		tempstr.append(" -voice\n");
		const char *cllist = tempstr.c_str();

		//get channelclientlist
		iResult = send(overlay, cllist, (int)strlen(cllist), 0);	//send channelcli...
		if (iResult == SOCKET_ERROR)
		{
			AppWarning(TEXT("Overlay: channelclientlist Send Failure"));
			CloseConnection(overlay);
			goto skip;
		}
		Sleep(5);					//100//allows full list to be generated and limits loop rate
		iResult = recv(overlay, reci3, 4096 ,0);					//recieve channelcli...
		if (iResult == SOCKET_ERROR)
		{
			AppWarning(TEXT("Overlay: channelclientlist Recieve Failure"));
			CloseConnection(overlay);
			goto skip;
		}

		identstart = "name=";
		identend = "client_type";
		list = reci3;
		//generate client list
		for(int i = 0; i < 10; i++)
		{
			startpos = list.find(identstart);
			if(startpos == -1)
			{
				skipname = true;
			}
			endpos = list.find(identend);
			if(endpos < 0)
			{
				skipname = true;
			}
			if(!skipname)
			{
				tempstr = list.substr(startpos + 5, endpos-startpos-6);
				name[i] = tempstr;
			}
			else
			{
				name[i] = "";
				skipname = false;
			}
			startpos = list.find(talkstart);
			if(startpos == -1)
			{
				talk[i] = false;
				break;
			}
			else if(list.substr(startpos + 20, 1) == "0")
			{
				talk[i] = false;
			}
			else if(list.substr(startpos + 20, 1) == "1")
			{
				talk[i] = true;
			}
			list = list.substr(startpos+25);
		}
		
		//print client list
		fOverlay.open(path);
		for(int i = 0; i < 10; i++)
		{
			if(name[i] == "")
			{
				break;
			}
			ReplaceAll(name[i], "\\s", " ");
			wstring wsTmp(name[i].begin(), name[i].end());
			/*print to screen here*/
			if(talk[i])
			{
				fOverlay << L"\u25CF" << wsTmp << endl;
			}
			else
			{
				fOverlay << L"\u25CB" << wsTmp << endl;
			}
		}
		fOverlay.close();

		//reset variables
		memset(reci1, 0, 256);
		memset(reci2, 0, 64);
		memset(reci3, 0, 4096);
		cid.clear();
		list.clear();
		for(int i = 0; i < 10; i++)
		{
			name[i].clear();
			talk[i] = false;
		}
		identstart.clear();
		identend.clear();
		startpos = 0;
		endpos = 0;
		tempstr.clear();

		CloseConnection(overlay);

skip:
		Sleep(100);
	}

	return;
}