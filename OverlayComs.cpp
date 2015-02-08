/*****************************
2015 <adocilesloth@gmail.com>
*****************************/
#include "TS3Plugin.h"
#include "ending.h"
#include "socketSRall.h"

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
	AppWarning(TEXT("Overlay: ShutdownOverlay"));
	close.now();
	return;
}

void ResetOverlay()
{
	AppWarning(TEXT("Overlay: ResetOverlay"));
	close.nolonger();
	return;
}

void RunOverlay(char* adrs)
{
	std::locale loc(std::locale::classic(), new codecvt_utf8<wchar_t>);
	//file.imbue(loc);

	int iname;
	bool bname;
	bool bright;
	bool btalker;
	int italker;
	vector<wstring> vname;
	vector<int> vtime;
	bool bsaid = false;

	SendAll sa;
	RecvAll ra;
	bool iResult;
	char reci1[181];
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
	int namesize;
	vector<bool> talk;
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
				AppWarning(TEXT("SOCKET_ERROR"));
				wstringstream code;
				code << WSAGetLastError();
				AppWarning(code.str().c_str());

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

		iResult = ra.recv_all(overlay, reci1, 181 ,0);	//get TS3 Client...
		if (!iResult)
		{
			AppWarning(TEXT("Overlay: First Recieve Failure"));
			AppWarning(TEXT("SOCKET_ERROR"));
			wstringstream code;
			code << WSAGetLastError();
			AppWarning(code.str().c_str());
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
		iResult = sa.send_all(overlay, whoami, (int)strlen(whoami), 0);	//send whoami
		if (!iResult)
		{
			AppWarning(TEXT("Overlay: whoami Send Failure"));
			AppWarning(TEXT("SOCKET_ERROR"));
			wstringstream code;
			code << WSAGetLastError();
			AppWarning(code.str().c_str());
			goto skip;
		}
		iResult = ra.recv_all(overlay, reci2, 64 ,0 ,"msg=");	//get whoami
		if (!iResult)
		{
			AppWarning(TEXT("Overlay: whoami Recieve Failure"));
			AppWarning(TEXT("SOCKET_ERROR"));
			wstringstream code;
			code << WSAGetLastError();
			AppWarning(code.str().c_str());
			goto skip;
		}
		tempstr = reci2;
		identstart = "msg=";
		startpos = tempstr.find(identstart);
		if(tempstr.substr(startpos, 5) == "msg=n")
		{
			if(noserv)	//if initial disconnection
			{
				AppWarning(TEXT("Overlay: Not Connected to TS3 Server"));
				noserv = false;
				//force Communicate, MuteandDeafen and ChannelSwitch
				bCom = false;
				bMnD = false;
				bSwt = false;
				rename = false;
			}
			memset(reci2, 0, 64);
			fOverlay.open(path);			//empty client list text file if not on server
			fOverlay.close();
			goto skip;
		}
		tempstr = "";

		if(!noserv)
		{
			AppWarning(TEXT("Overlay: Now Connected to TS3 Server"));
			rename = true;
			noserv = true;
		}

		identstart = "cid=";
		identend = "\n";
		cid = reci2;
		startpos = cid.find(identstart);

		if(startpos == -1)
		{
			AppWarning(TEXT("Overlay: startpos == -1"));
			goto skip;
		}
		endpos = cid.find(identend);
		if(endpos < 0)
		{
			AppWarning(TEXT("Overlay: endpos < 0"));
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
		iResult = sa.send_all(overlay, cllist, (int)strlen(cllist), 0);	//send channelcli...
		if (!iResult)
		{
			AppWarning(TEXT("Overlay: channelclientlist Send Failure"));
			AppWarning(TEXT("SOCKET_ERROR"));
			wstringstream code;
			code << WSAGetLastError();
			AppWarning(code.str().c_str());
			goto skip;
		}
		iResult = ra.recv_all(overlay, &reci3[0], reci3.size(), 0, "msg=");					//recieve channelcli...
		if (!iResult)
		{
			AppWarning(TEXT("Overlay: channelclientlist Recieve Failure"));
			AppWarning(TEXT("SOCKET_ERROR"));
			wstringstream code;
			code << WSAGetLastError();
			AppWarning(code.str().c_str());	
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
				break;
			}
			endpos = wlist.find(widentend);
			if(endpos < 0)
			{
				break;
			}

			wtempstr = wlist.substr(startpos + 5, endpos-startpos-6);
			name.push_back(wtempstr);

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
		
		//only show talker time keeping
		btalker = GetOnlyShowTalker();
		if(btalker)
		{
			italker = GetHideNameAfter()/100;
			if(static_cast<int>(vtime.size()) > 0)
			{
				for(int i = static_cast<int>(vtime.size()) - 1; i > -1; i--)
				{
					if(vtime[i] > italker)
					{
						vtime.erase(vtime.begin() + i);
						vname.erase(vname.begin() + i);
					}
					else
					{
						vtime[i]++;
					}
				}
			}
		}

		//print client list
		bname = GetHideSelf();
		bright = GetRightOfSymbol();
		fOverlay.open(path);
		//fOverlay << bname << endl;		//uncomment to remove the BOOL warning which forces this true or false

		namesize = name.size();
		if(namesize < 1)
		{
			goto skip;
		}
		else if(namesize < iname)
		{
			iname = namesize;
		}

		//normal
		if(!btalker)
		{
			for(int i = 0; i < iname; i++)
			{
				if(name[i].empty())
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
					if(!bright)
					{
						if(talk[i])
						{
							fOverlay << L"\u25CF" << wsTmp << endl;
						}
						else
						{
							fOverlay << L"\u25CB" << wsTmp << endl;
						}
					}
					else
					{
						if(talk[i])
						{
							fOverlay << wsTmp << L"\u25CF" << endl;
						}
						else
						{
							fOverlay << wsTmp << L"\u25CB" << endl;
						}
					}
				}
			}//end for(int i...
		}
		//Only show talkers
		else
		{
			for(int i = 0; i < iname; i++)
			{
				if(name[i].empty())
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
						bsaid = false;
						if(static_cast<int>(vname.size()) > 0)
						{
							for(int j = 0; j < static_cast<int>(vname.size()); j++)
							{
								if(wsTmp == vname[j])
								{
									vtime[j] = 0;
									bsaid = true;
									break;
								}
							}
						}
						if(!bsaid)
						{
							vname.push_back(wsTmp);
							vtime.push_back(0);
						}
						else
						{
							bsaid = false;
						}
					}//end if(talk[i])
				}
			}//end for(int i...

			//Print
			if(static_cast<int>(vname.size()) > 0)
			{
				for(int j = 0; j < static_cast<int>(vname.size()); j++)
				{
					fOverlay << vname[j] << endl;
				}
			}
		}//end Only Show talkers

		fOverlay.close();

		//reset variables
		memset(reci1, 0, 181);
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

skip:
		CloseConnection(overlay);
		Sleep(100);
	}

	if(!ConnectToHost(25639, adrs, overlay))
	{
		AppWarning(TEXT("StopStream: Connection Failure: Check TS3 is running and ClientQuery Plugin is enabled"));
		AppWarning(TEXT("SOCKET_ERROR"));
		wstringstream code;
		code << WSAGetLastError();
		AppWarning(code.str().c_str());
		return;
	}

	iResult = ra.recv_all(overlay, reci1, 181 ,0);	//get TS3 Client...
	if (!iResult)
	{
		AppWarning(TEXT("StopStream: First Recieve Failure"));
		AppWarning(TEXT("SOCKET_ERROR"));
		wstringstream code;
		code << WSAGetLastError();
		AppWarning(code.str().c_str());
	}

	Communicate(0, overlay);
	MuteandDeafen(0, overlay);
	ChannelSwitch(0, overlay);
	CloseConnection(overlay);

	return;
}