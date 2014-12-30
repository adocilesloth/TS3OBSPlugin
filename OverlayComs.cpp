/*****************************
2014 <adocilesloth@gmail.com>
*****************************/
#include "TS3Plugin.h"
#include "ending.h"

#include <string>
#include <sstream>
#include <fstream>
#include <codecvt>
#include <vector>

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

	int iname;
	bool bname;

	int iResult;
	char reci1[256];
	char reci2[64];
	vector<char> reci3;
	int reci3Size;

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
	vector<wstring> name;
	vector<bool> talk;
	bool skipname = false;
	bool discon = true;
	bool noserv = true;
	bool rename = true;
	bool bCom = false;
	wstring userName;
	bool bMnD = false;
	bool bSwt = false;

	//File stuff
	wstring path = OBSGetPluginDataPath().Array();
	path.append(L"\\Overlay.txt");
	wofstream fOverlay;
	fOverlay.imbue(loc);
	fOverlay.open(path);	//open and close to clear file
	fOverlay.close();

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

		if(rename)
		{
			if(!bCom)
			{
				userName = Communicate(1, overlay);
				if(userName != L"poop")
				{
					wReplaceAll(userName, L"\\s", L" ");
					bCom = true;
				}
				else
				{
					bCom = false;
				}
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

		//recieve setup
		iname = GetNumberOfNames();
		reci3Size = iname * 360;
		reci3.resize(reci3Size);

		//get channelclientlist
		iResult = send(overlay, cllist, (int)strlen(cllist), 0);	//send channelcli...
		if (iResult == SOCKET_ERROR)
		{
			AppWarning(TEXT("Overlay: channelclientlist Send Failure"));
			CloseConnection(overlay);
			goto skip;
		}
		Sleep(5);					//100//allows full list to be generated and limits loop rate
		iResult = recv(overlay, &reci3[0], reci3.size(), 0);					//recieve channelcli...
		if (iResult == SOCKET_ERROR)
		{
			AppWarning(TEXT("Overlay: channelclientlist Recieve Failure"));
			CloseConnection(overlay);
			goto skip;
		}

		widentstart = L"name=";
		widentend = L"client_type";
		list.assign(reci3.begin(), reci3.end());
		wlist = s2ws(list);

		//generate client list
		for(int i = 0; i < iname; i++)
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
				name.push_back(wtempstr);
			}
			else
			{
				name.push_back(L"");
				skipname = false;
			}
			startpos = wlist.find(talkstart);
			if(startpos == -1)
			{
				talk.push_back(false);
				break;
			}
			else if(wlist.substr(startpos + 20, 1) == L"0")
			{
				talk.push_back(false);
			}
			else if(wlist.substr(startpos + 20, 1) == L"1")
			{
				talk.push_back(true);
			}
			wlist = wlist.substr(startpos+25);
		}
		
		//print client list
		bname = GetHideSelf();
		fOverlay.open(path);
		//fOverlay << bname << endl;		//uncomment to remove the BOOL warning which forces this true or false
		for(int i = 0; i < iname; i++)
		{
			if(name[i] == L"")
			{
				break;
			}
			wReplaceAll(name[i], L"\\s", L" ");
			if(bname && name[i] == userName)
			{
				//do nothing
			}
			else
			{
				wstring wsTmp(name[i].begin(), name[i].end());
	
				if(talk[i])
				{
					fOverlay << L"\u25CF" << wsTmp << endl;
				}
				else
				{
					fOverlay << L"\u25CB" << wsTmp << endl;
				}
			}
		}
		fOverlay.close();

		//reset variables
		memset(reci1, 0, 256);
		memset(reci2, 0, 64);
		reci3.clear();
		cid.clear();
		list.clear();
		name.clear();
		talk.clear();
		identstart.clear();
		identend.clear();
		startpos = 0;
		endpos = 0;
		tempstr.clear();

		CloseConnection(overlay);

skip:
		Sleep(100);
	}

	//unname
	if(!ConnectToHost(25639, adrs, overlay))
	{
		AppWarning(TEXT("StopStream: Connection Failure: Check TS3 is running and ClientQuery Plugin is enabled"));
	}
	Communicate(0, overlay);
	MuteandDeafen(0, overlay);
	ChannelSwitch(0, overlay);
	CloseConnection(overlay);

	return;
}