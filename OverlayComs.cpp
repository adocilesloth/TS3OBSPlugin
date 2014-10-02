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
	wstring widentstart;
	string identend;
	wstring widentend;
	wstring talkstart = L"client_flag_talking=";
	size_t startpos;
	size_t endpos;
	string tempstr;
	wstring wtempstr;

	//useful things
	string cid;
	string list;
	wstring wlist;
	wstring name[10];
	bool talk[10] = {0};
	bool skipname = false;
	bool discon = true;
	bool noserv = true;
	bool rename = true;
	bool bCom = false;
	bool bMnD = false;
	bool bSwt = false;

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
				//force Communicate, MuteandDeafen and ChannelSwitch
				bCom = false;
				bMnD = false;
				bSwt = false;
				rename = true;
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
				//force Communicate, MuteandDeafen and ChannelSwitch
				bCom = false;
				bMnD = false;
				bSwt = false;
				rename = true;
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

		widentstart = L"name=";
		widentend = L"client_type";
		list = reci3;
		wlist = s2ws(list);
		//generate client list
		for(int i = 0; i < 10; i++)
		{
			startpos = wlist.find(widentstart);
			if(startpos == -1)
			{
				skipname = true;
			}
			endpos = wlist.find(widentend);
			if(endpos < 0)
			{
				skipname = true;
			}
			if(!skipname)
			{
				wtempstr = wlist.substr(startpos + 5, endpos-startpos-6);
				name[i] = wtempstr;
			}
			else
			{
				name[i] = L"";
				skipname = false;
			}
			startpos = wlist.find(talkstart);
			if(startpos == -1)
			{
				talk[i] = false;
				break;
			}
			else if(wlist.substr(startpos + 20, 1) == L"0")
			{
				talk[i] = false;
			}
			else if(wlist.substr(startpos + 20, 1) == L"1")
			{
				talk[i] = true;
			}
			wlist = wlist.substr(startpos+25);
		}
		
		//print client list
		fOverlay.open(path);
		for(int i = 0; i < 10; i++)
		{
			if(name[i] == L"")
			{
				break;
			}
			wReplaceAll(name[i], L"\\s", L" ");
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

		if(rename)
		{
			if(!bCom)
			{
				bCom = Communicate(1, overlay);
			}
			if(!bMnD)
			{
				bMnD = MuteandDeafen(1, overlay);
			}
			if(!bSwt)
			{
				bSwt = ChannelSwitch(1, overlay);
			}
			if(bCom && bMnD && bSwt)
			{
				bCom = false;
				bMnD = false;
				bSwt = false;
				rename = false;
			}
		}

		CloseConnection(overlay);

skip:
		Sleep(100);
	}

	return;
}