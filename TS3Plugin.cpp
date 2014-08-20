<<<<<<< HEAD
/*****************************
2014 <adocilesloth@gmail.com>
*****************************/
#include "TS3Plugin.h"
#include "resource.h"
#include "OverlaySource.h"
=======
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
#include "resource.h"
>>>>>>> 80faf0302e1248b2a6468bfd5f267c90f860ae1d

#include <fstream>
#include <sstream>

<<<<<<< HEAD
=======
#define WIN32_MEAN_AND_LEAN
#include <winsock2.h>

>>>>>>> 80faf0302e1248b2a6468bfd5f267c90f860ae1d
using namespace std;

int countSubstring(const string&, const string&);

SOCKET obs;
//ofstream file;
ifstream settings;

//config popout stuff
HINSTANCE   hInstance;
//cid for config
string cid;

<<<<<<< HEAD
//Overlay Stuff
HANDLE OvrThread;

=======
>>>>>>> 80faf0302e1248b2a6468bfd5f267c90f860ae1d
INT_PTR CALLBACK ConfigDlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	string sip, suid, spref, smute, schan, spw;		//temp strings

	wstring wip, wuid, wpref, wcid, wpw;	//sending strings

	bool bmute, bdeaf, bchan;		//bools for checkboxes
	int length;
<<<<<<< HEAD
	wstring path = OBSGetPluginDataPath().Array();
	wofstream osettings;
	
	//get current information
	settings.open(path + L"\\ts3.ini");
=======
	string path = OBSGetPluginDataPath().CreateUTF8String();
	wofstream osettings;
	
	//get current information
	settings.open(path + "\\ts3.ini");
>>>>>>> 80faf0302e1248b2a6468bfd5f267c90f860ae1d

	getline(settings, sip);
	wip = wstring(sip.begin(), sip.end());

	getline(settings, suid);
	suid = suid.substr(6, suid.length() - 6);	//remove cluid=
	wuid = wstring(suid.begin(), suid.end());

	getline(settings, spref);
	if (spref.length() > 10)
	{
		spref = spref.substr(0, 10);
	}
	wpref = wstring(spref.begin(), spref.end());

	getline(settings, smute);
	if(smute == "1")
	{
		bmute = true;
		bdeaf = false;
	}
	else if(smute == "2")
	{
		bmute = false;
		bdeaf = true;
	}
	else if(smute == "3")
	{
		bmute = true;
		bdeaf = true;
	}
	else
	{
		bmute = false;
		bdeaf = false;
	}

	getline(settings, schan);
	if(schan == "1")
	{
		bchan = true;
	}
	else
	{
		bchan = false;
	}

	getline(settings, spw);		//gets cid=..
	spw.clear();				//so delete it
	getline(settings, spw);		//gets cpw
	spw = spw.substr(5, spw.length() - 5);	//remove  cpw=
	wpw = wstring(spw.begin(), spw.end());

	settings.close();

	switch (message)
	{
	case WM_INITDIALOG:
	{
		if(wip.length() != 0)
		{
			HWND IPInput = GetDlgItem(hWnd, IPEdt);
			SetWindowText(IPInput, wip.c_str());
		}
		if(wuid.length() != 0)
		{
			HWND UIDInput = GetDlgItem(hWnd, UIDEdt);
			SetWindowText(UIDInput, wuid.c_str());
		}
		HWND PREFInput = GetDlgItem(hWnd, PREFEdt);
		SendMessage(PREFInput, EM_LIMITTEXT, 10, 0);	//limit prefix to 10 chars
		if(wpref.length() != 0)
		{
			SetWindowText(PREFInput, wpref.c_str());
		}
		SendMessage(GetDlgItem(hWnd, MUTChk), BM_SETCHECK, bmute ? BST_CHECKED : BST_UNCHECKED, 0);
		SendMessage(GetDlgItem(hWnd, DEFChk), BM_SETCHECK, bdeaf ? BST_CHECKED : BST_UNCHECKED, 0);
		SendMessage(GetDlgItem(hWnd, CHANChk), BM_SETCHECK, bchan ? BST_CHECKED : BST_UNCHECKED, 0);
		if(bchan)
		{
			EnableWindow(GetDlgItem(hWnd, CHANBut), true);
			ShowWindow(GetDlgItem(hWnd, CHANTxt), SW_SHOW);
			EnableWindow(GetDlgItem(hWnd, PWEdt), true);
		}
		if(wpw.length() != 0)
		{
			HWND PWInput = GetDlgItem(hWnd, PWEdt);
			SetWindowText(PWInput, wpw.c_str());
		}
	}

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDOK:
		{
			//get information
<<<<<<< HEAD
			osettings.open(path + L"\\ts3.ini");
=======
			osettings.open(path + "\\ts3.ini");
>>>>>>> 80faf0302e1248b2a6468bfd5f267c90f860ae1d

			//IP
			HWND IPOutput = GetDlgItem(hWnd, IPEdt);
			length = GetWindowTextLength(IPOutput) + 1;
			TCHAR * ipbuf = (TCHAR *)GlobalAlloc(GPTR, length * sizeof(TCHAR));					
			GetWindowText(IPOutput, ipbuf, length);
			osettings << ipbuf << endl;	//write to file	
			GlobalFree(ipbuf);			//clear buffer
			//uid
			HWND UIDOutput = GetDlgItem(hWnd, UIDEdt);
			length = GetWindowTextLength(UIDOutput) + 1;
			TCHAR * uidbuf = (TCHAR *)GlobalAlloc(GPTR, length * sizeof(TCHAR));
			GetWindowText(UIDOutput, uidbuf, length);
			osettings << "cluid="
					  << uidbuf << endl;	//write to file
			GlobalFree(uidbuf);				//clear buffer
			//prefix
			HWND PREFOutput = GetDlgItem(hWnd, PREFEdt);
			length = GetWindowTextLength(PREFOutput) + 1;
			TCHAR * prefbuf = (TCHAR *)GlobalAlloc(GPTR, length * sizeof(TCHAR));
			GetWindowText(PREFOutput, prefbuf, length);
			osettings << prefbuf << endl;	//write to file
			GlobalFree(prefbuf);			//clear buffer
			//mute and deafened
			bmute = SendMessage(GetDlgItem(hWnd, MUTChk), BM_GETCHECK, 0, 0) == BST_CHECKED;
			bdeaf = SendMessage(GetDlgItem(hWnd, DEFChk), BM_GETCHECK, 0, 0) == BST_CHECKED;
			if(!bmute && !bdeaf)
			{
				osettings << "0" << endl;
			}
			else if(bmute && !bdeaf)
			{
				osettings << "1" << endl;
			}
			else if(!bmute && bdeaf)
			{
				osettings << "2" << endl;
			}
			else if(bmute && bdeaf)
			{
				osettings << "3" << endl;
			}
			//channel switch
			bchan = SendMessage(GetDlgItem(hWnd, CHANChk), BM_GETCHECK, 0, 0) == BST_CHECKED;
			if(bchan)
			{
				osettings << "1" << endl;
			}
			else
			{
				osettings << "0" << endl;
			}
			//cid
			wcid = wstring(cid.begin(), cid.end());
			osettings << wcid << endl;
			//password
			HWND PWOutput = GetDlgItem(hWnd, PWEdt);
			length = GetWindowTextLength(PWOutput) + 1;
			TCHAR * pwbuf = (TCHAR *)GlobalAlloc(GPTR, length * sizeof(TCHAR));
			GetWindowText(PWOutput, pwbuf, length);
			osettings << " cpw="
					  << pwbuf;			//write to file
			GlobalFree(pwbuf);			//clear buffer

			//clean up
			osettings.close();
			
			//close dialog box
			EndDialog(hWnd, LOWORD(wParam));
			break;
		}

		case IDCANCEL:
			EndDialog(hWnd, LOWORD(wParam));
			break;

		case CHANChk:
		{
			bchan = SendMessage(GetDlgItem(hWnd, CHANChk), BM_GETCHECK, 0, 0) == BST_CHECKED;
			if(bchan)
			{
				EnableWindow(GetDlgItem(hWnd, CHANBut), true);
				ShowWindow(GetDlgItem(hWnd, CHANTxt), SW_SHOW);
				EnableWindow(GetDlgItem(hWnd, PWEdt), true);
			}
			else if(!bchan)
			{
				EnableWindow(GetDlgItem(hWnd, CHANBut), false);
				ShowWindow(GetDlgItem(hWnd, CHANTxt), SW_HIDE);
				EnableWindow(GetDlgItem(hWnd, PWEdt), false);
			}
		}
		case CHANBut:
		{
			char *adrs = getIP();

<<<<<<< HEAD
			if(!ConnectToHost(25639, adrs, obs))
=======
			if(!ConnectToHost(25639, adrs))
>>>>>>> 80faf0302e1248b2a6468bfd5f267c90f860ae1d
			{
				break;
			}

			int iResult;
			int i = 0;
			char *whoami = "whoami\n";
			char reci[256];
			string truereci;

			do
			{
				iResult = send(obs, whoami, (int)strlen(whoami), 0);	//request whoami
				if (iResult == SOCKET_ERROR)
				{
					AppWarning(TEXT("whoami Send Failure"));
					break;
				}

				iResult = recv(obs, reci, 256, 0);	//recieve result: clid=XX cid=XXXX
				if (iResult == SOCKET_ERROR)
				{
					AppWarning(TEXT("whoami Recieve Failure"));
					break;
				}

				truereci = reci;
				truereci = truereci.substr(0, 5);
				i++;
			}while (truereci != "clid=" && i < 10);	//while reci returns the wrong string
			
			if(truereci != "clid=")
			{
				break;
			}

			cid = reci;
			size_t startpos = cid.find("cid=");
			size_t endpos = cid.find("\n");
			cid = cid.substr(startpos, endpos - startpos);

<<<<<<< HEAD
			CloseConnection(obs);
=======
			CloseConnection();
>>>>>>> 80faf0302e1248b2a6468bfd5f267c90f860ae1d
			break;
		}

		}
	}

	return 0;
}

void ConfigPlugin(HWND hWnd)
{
	DialogBox(hInstance, MAKEINTRESOURCE(IDD_TS3DLG), hWnd, ConfigDlgProc);
}

bool LoadPlugin()
{
<<<<<<< HEAD

	OBSRegisterImageSourceClass(L"OverlaySource", L"TeamSpeak 3 Overlay", (OBSCREATEPROC)CreateOverlaySource, (OBSCONFIGPROC)ConfigureOverlaySource);

	wstring path = OBSGetPluginDataPath().Array();
	settings.open(path + L"\\ts3.ini");

	if (!settings.is_open())
	{
		ofstream create(path + L"\\ts3.ini");
=======
	string path = OBSGetPluginDataPath().CreateUTF8String();
	settings.open(path + "\\ts3.ini");

	if (!settings.is_open())
	{
		ofstream create(path + "\\ts3.ini");
>>>>>>> 80faf0302e1248b2a6468bfd5f267c90f860ae1d
		create << "127.0.0.1" << endl;
		create << "cluid=" << endl;
		create << "*R*" << endl;
		create << "0" << endl;
		create << "0" << endl;
		create << "cid=1" << endl;
		create << " cpw=";
		create.close();		//stop using settings file

		cid = "cid=1";		//set cid string
	}
	else
	{
		for(int i = 0; i < 6; i++)
		{
			getline(settings, cid);	//set cid string
		}
		settings.close();		//stop using settings file
	}

	AppWarning(TEXT("TS3Plugin Loaded"));
	return true;
}

void UnloadPlugin()
{
	//get settings file path
<<<<<<< HEAD
	wstring path = OBSGetPluginDataPath().Array();
	path.append(L"/ts3temp.ini");
=======
	string path = OBSGetPluginDataPath().CreateUTF8String();
	path.append("/ts3temp.ini");
>>>>>>> 80faf0302e1248b2a6468bfd5f267c90f860ae1d
	//test if ts3temp.ini exists. If yes, delete
	settings.open(path);
	if(settings.is_open())
	{
		settings.close();
<<<<<<< HEAD
		stringstream spath;
		spath << path.c_str();
		remove(spath.str().c_str());
=======
		remove(path.c_str());
>>>>>>> 80faf0302e1248b2a6468bfd5f267c90f860ae1d
	}
}

CTSTR GetPluginName()
{
<<<<<<< HEAD
	return TEXT("Teamspeak 3 Plugin");
=======
	return TEXT("TS3 Recording Notifier");
>>>>>>> 80faf0302e1248b2a6468bfd5f267c90f860ae1d
}

CTSTR GetPluginDescription()
{
<<<<<<< HEAD
	return TEXT("Adds *R* (or any other prefix) before TS3 nickname when recording. Options to mute/deafen self and move channel while recording. Undos all effects when recording stops. Also adds a simple ovelay as a source.");
=======
	return TEXT("Adds *R* (or any other prefix) before TS3 nickname when recording. Options to mute/deafen self and move channel while recording. Undos all effects when recording stops.");
>>>>>>> 80faf0302e1248b2a6468bfd5f267c90f860ae1d
}

void OnStartStream()
{
	char *adrs = getIP();
	bool start;

<<<<<<< HEAD
	if(!ConnectToHost(25639, adrs, obs))
=======
	if(!ConnectToHost(25639, adrs))
>>>>>>> 80faf0302e1248b2a6468bfd5f267c90f860ae1d
	{
		return;
	}
	start = Communicate(1);
	start = MuteandDeafen(1);
	start = ChannelSwitch(1);
<<<<<<< HEAD
	CloseConnection(obs);

	OvrThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)RunOverlay, adrs, 0, 0);

=======
	CloseConnection();
>>>>>>> 80faf0302e1248b2a6468bfd5f267c90f860ae1d

	return;
}

void OnStopStream()
{
	char *adrs = getIP();
	bool stop;

<<<<<<< HEAD
	ShutdownOverlay();
	WaitForSingleObject(OvrThread, INFINITE);
	ResetOverlay();

	if(!ConnectToHost(25639, adrs, obs))
=======
	if(!ConnectToHost(25639, adrs))
>>>>>>> 80faf0302e1248b2a6468bfd5f267c90f860ae1d
	{
		return;
	}
	stop = Communicate(0);
	stop = MuteandDeafen(0);
	stop = ChannelSwitch(0);
<<<<<<< HEAD
	CloseConnection(obs);
=======
	CloseConnection();
>>>>>>> 80faf0302e1248b2a6468bfd5f267c90f860ae1d

	return;
}

char* getIP()
{
<<<<<<< HEAD
	wstring path = OBSGetPluginDataPath().Array();
	string IPadrsstr;

	settings.open(path + L"\\ts3.ini");
=======
	string path = OBSGetPluginDataPath().CreateUTF8String();
	string IPadrsstr;

	settings.open(path + "\\ts3.ini");
>>>>>>> 80faf0302e1248b2a6468bfd5f267c90f860ae1d

	getline(settings, IPadrsstr);
	char *IPadrs = new char[IPadrsstr.length() + 1];
	strcpy(IPadrs, IPadrsstr.c_str());

	settings.close();
	return IPadrs;
}

<<<<<<< HEAD
=======
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

	SOCKADDR_IN target;							//Socket address information
    target.sin_family = AF_INET;				// address family Internet
    target.sin_port = htons (port);				//Port to connect on
    target.sin_addr.s_addr = inet_addr (adrs);	//Target IP

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

>>>>>>> 80faf0302e1248b2a6468bfd5f267c90f860ae1d
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
	string space = "\\s";

	int i = 0;	//break while loop counter for reci2
	string truereci2 = "Welcome to";	//bad string for reci2
	int j = 0;	//break while loop counter for reci3
	string truereci3 = "error id=0";	//bad string for reci3

	//debug file
	//file.open("C:/Program Files (x86)/OBS/plugins/outfile.txt");

	//get settings file path
<<<<<<< HEAD
	wstring path = OBSGetPluginDataPath().Array();
	//get cluid and recording prefix from ts3.txt
	settings.open(path + L"\\ts3.ini");
=======
	string path = OBSGetPluginDataPath().CreateUTF8String();
	//get cluid and recording prefix from ts3.txt
	settings.open(path + "\\ts3.ini");
>>>>>>> 80faf0302e1248b2a6468bfd5f267c90f860ae1d
	string cluid;
	getline(settings, cluid);	//first line is ip address
	cluid.clear();				//so we don't want it
	getline(settings, cluid);
	string rec;
	getline(settings, rec);

	//set up the getname call
	string tempgetname = "clientgetnamefromuid ";
	tempgetname.append(cluid);
	tempgetname.append("\n");
	const char *getname = tempgetname.c_str();

	iResult = recv(obs, reci1, 256 ,0);	//get TS3 Client...
	if (iResult == SOCKET_ERROR)
	{
		AppWarning(TEXT("First Recieve Failure"));
		settings.close();
		return false;
	}

	do
	{
		iResult = send(obs, notify, (int)strlen(notify), 0);	//request notifyregister...
		if (iResult == SOCKET_ERROR)
		{
			AppWarning(TEXT("First Send Failure"));
			settings.close();
			return false;
		}

		iResult = recv(obs, reci2, 256, 0);	//recieve result: error id=0...
		if (iResult == SOCKET_ERROR)
		{
			AppWarning(TEXT("Second Recieve Failure"));
			settings.close();
			return false;
		}

		truereci2 = reci2;
		truereci2 = truereci2.substr(0, 10);
		i++;
	}while (truereci2 == "Welcome to" && i < 10);	//while reci2 returns the wrong string

	if (truereci2 == "Welcome to")	//fail request notifyregister...
	{
		AppWarning(TEXT("clientnotifyregister failed after 10 tries"));
		settings.close();
		return false;
	}

	do
	{
		iResult = send(obs, getname, (int)strlen(getname), 0);	//request clientnamefromuid...
		if (iResult == SOCKET_ERROR)
		{
			AppWarning(TEXT("Second Send Failure"));
			settings.close();
			return false;
		}

		iResult = recv(obs, reci3, 256, 0);	//recieve name
		if (iResult == SOCKET_ERROR)
		{
			AppWarning(TEXT("Third Recieve Failure"));
			settings.close();
			return false;
		}
		truereci3 = reci3;
		truereci3 = truereci3.substr(0, 10);
		j++;
	} while (truereci3 == "error id=0" && j < 10);	//while reci3 returns the wrong string

	if (truereci3 == "error id=0")	//fail request clientnamefromuid
	{
		AppWarning(TEXT("clientgetnamefromuid failed after 10 tries"));
		settings.close();
		return false;
	}

	//get name
	string identstart = "name=";
	string name = reci3;
	size_t startpos = name.find(identstart);	//start of name
	int count = countSubstring(name, space);	//number of \s
	name = name.substr(startpos+5 , 30 + count);
	//get name end
	
	if(cont == 1)
	{
		if(name.substr(0, rec.length()) != rec)
		{
			name = name.substr(0, 30 + count - rec.length());
			newname << rec;
		}
	}
	else if(cont == 0)
	{
		if(name.substr(0, rec.length()) == rec)
		{
			name = name.substr(rec.length(), 30 + count - rec.length());
		}
	}

	newname << name << "\n";			//finish name set string
	const string tmp = newname.str();	//set name to string
	const char* recname = tmp.c_str();	//set name to char* so it can be sent

	iResult = send(obs, recname, (int)strlen(recname), 0);
	if (iResult == SOCKET_ERROR)
	{
		AppWarning(TEXT("Third Send Failure"));
		settings.close();
		return false;
	}

	iResult = recv(obs, reci4, 256 ,0);
	if (iResult == SOCKET_ERROR)
	{
		AppWarning(TEXT("Fourth Recieve Failure"));
		settings.close();
		return false;
	}

	//file.close();
	settings.close();
	return true;
}

bool MuteandDeafen(int state)
{
	//get settings file path
<<<<<<< HEAD
	wstring path = OBSGetPluginDataPath().Array();
	//get mute and deafen settings from ts3.ini
	settings.open(path + L"\\ts3.ini");
=======
	string path = OBSGetPluginDataPath().CreateUTF8String();
	//get mute and deafen settings from ts3.ini
	settings.open(path + "\\ts3.ini");
>>>>>>> 80faf0302e1248b2a6468bfd5f267c90f860ae1d
	string mnd;
	getline(settings, mnd);		//ip
	mnd.clear();
	getline(settings, mnd);		//cluid
	mnd.clear();
	getline(settings, mnd);		//prefix
	mnd.clear();
	getline(settings, mnd);		//mute and deafened state

	if(mnd != "1" && mnd != "2" && mnd != "3")	//if not set to mute or deafen
	{
		settings.close();
		return true;
	}

	int iResult;
	stringstream sstate;
	sstate << state << "\n";

	if(mnd == "1" || mnd == "3")	//if set to mute
	{
		char reci1[256];
		int i = 0;
		string truereci1;

		string tempmute = "clientupdate client_input_muted=";
		tempmute.append(sstate.str());
		const char *mute = tempmute.c_str();

		do
		{
			iResult = send(obs, mute, (int)strlen(mute), 0);	//set mute
			if (iResult == SOCKET_ERROR)
			{
				AppWarning(TEXT("Mute Send Failure"));
				settings.close();
				return false;
			}

			iResult = recv(obs, reci1, 256, 0);	//recieve result: error id=0...
			if (iResult == SOCKET_ERROR)
			{
				AppWarning(TEXT("Mute Recieve Failure"));
				settings.close();
				return false;
			}

			truereci1 = reci1;
			truereci1 = truereci1.substr(0, 10);
			i++;
		}while (truereci1 != "error id=0" && i < 10);
	}

	if(mnd == "2" || mnd == "3")
	{
		char reci2[256];
		int j = 0;
		string truereci2;
	
		string tempdeaf = "clientupdate client_output_muted=";
		tempdeaf.append(sstate.str());
		const char *deaf = tempdeaf.c_str();

		do
		{
			iResult = send(obs, deaf, (int)strlen(deaf), 0);	//set deafen
			if (iResult == SOCKET_ERROR)
			{
				AppWarning(TEXT("Deaf Send Failure"));
				settings.close();
				return false;
			}

			iResult = recv(obs, reci2, 256, 0);	//recieve result: error id=0...
			if (iResult == SOCKET_ERROR)
			{
				AppWarning(TEXT("Deaf Recieve Failure"));
				settings.close();
				return false;
			}

			truereci2 = reci2;
			truereci2 = truereci2.substr(0, 10);
			j++;
		}while (truereci2 != "error id=0" && j < 10);
	}
	
	settings.close();
	return true;
}

bool ChannelSwitch(int state)
{
	//get settings file path
<<<<<<< HEAD
	wstring path = OBSGetPluginDataPath().Array();
	//get mute and deafen settings from ts3.ini
	settings.open(path + L"\\ts3.ini");
=======
	string path = OBSGetPluginDataPath().CreateUTF8String();
	//get mute and deafen settings from ts3.ini
	settings.open(path + "\\ts3.ini");
>>>>>>> 80faf0302e1248b2a6468bfd5f267c90f860ae1d
	string swtch;
	getline(settings, swtch);		//ip
	swtch.clear();
	getline(settings, swtch);		//cluid
	swtch.clear();
	getline(settings, swtch);		//prefix
	swtch.clear();
	getline(settings, swtch);		//mute and deafened state
	swtch.clear();
	getline(settings, swtch);		//switch state

	if(swtch != "1")	//if not set to mute or deafen
	{
		settings.close();
		return true;
	}

	int iResult;
	int i = 0;
	char *whoami = "whoami\n";
	char reci[256];
	string truereci, tcid, rcid, clid, cpw;

	do
	{
		iResult = send(obs, whoami, (int)strlen(whoami), 0);	//request whoami
		if (iResult == SOCKET_ERROR)
		{
			AppWarning(TEXT("whoami Send Failure"));
			settings.close();
			return false;
		}

		iResult = recv(obs, reci, 256, 0);	//recieve result: clid=XX cid=XXXX
		if (iResult == SOCKET_ERROR)
		{
			AppWarning(TEXT("whoami Recieve Failure"));
			settings.close();
			return false;
		}

		truereci = reci;
		truereci = truereci.substr(0, 5);
		i++;
	}while (truereci != "clid=" && i < 10);	//while reci returns the wrong string

	clid = reci;
	size_t space = clid.find("cid=");
	clid = clid.substr(0, space);

	string channel = "clientmove ";
	channel.append(clid);

	int j = 0;
	char reci2[256];
	string truereci2;

	if(state == 1)	//switch to channel
	{
		//write return cid to file
		rcid = reci;
		size_t startpos = rcid.find("cid=");
		size_t endpos = rcid.find("\n");
		rcid = rcid.substr(startpos, endpos - startpos);

<<<<<<< HEAD
		ofstream rturn(path + L"\\ts3temp.ini");
=======
		ofstream rturn(path + "\\ts3temp.ini");
>>>>>>> 80faf0302e1248b2a6468bfd5f267c90f860ae1d
		rturn << rcid;
		rturn.close();

		//move channel
		getline(settings, tcid);
		getline(settings, cpw);
		channel.append(tcid);
		channel.append(cpw);
		channel.append("\n");
		const char *move = channel.c_str();

		do
		{
			iResult = send(obs, move, (int)strlen(move), 0);	//set deafen
			if (iResult == SOCKET_ERROR)
			{
				AppWarning(TEXT("Move Send Failure"));
				settings.close();
				return false;
			}

			iResult = recv(obs, reci2, 256, 0);	//recieve result: error id=0...
			if (iResult == SOCKET_ERROR)
			{
				AppWarning(TEXT("Move Recieve Failure"));
				settings.close();
				return false;
			}

			truereci2 = reci2;
			truereci2 = truereci2.substr(0, 10);
			j++;
		}while (truereci2 != "error id=0" && j < 10);
	}

	else	//switch back
	{
		//get return cid
<<<<<<< HEAD
		ifstream rturn(path + L"\\ts3temp.ini");
=======
		ifstream rturn(path + "\\ts3temp.ini");
>>>>>>> 80faf0302e1248b2a6468bfd5f267c90f860ae1d
		if(!rturn.is_open())	//set target as same channel
		{
			rcid = reci;
			size_t startpos = rcid.find("cid=");
			size_t endpos = rcid.find("\n");
			rcid = rcid.substr(startpos, endpos - startpos);	
		}
		else
		{
			getline(rturn, rcid);
		}
		rturn.close();

		channel.append(rcid);
		channel.append("\n");
		const char *move = channel.c_str();

		do
		{
			iResult = send(obs, move, (int)strlen(move), 0);	//set deafen
			if (iResult == SOCKET_ERROR)
			{
				AppWarning(TEXT("Move Send Failure"));
				settings.close();
				return false;
			}

			iResult = recv(obs, reci2, 256, 0);	//recieve result: error id=0...
			if (iResult == SOCKET_ERROR)
			{
				AppWarning(TEXT("Move Recieve Failure"));
				settings.close();
				return false;
			}

			truereci2 = reci2;
			truereci2 = truereci2.substr(0, 10);
			j++;
		}while (truereci2 != "error id=0" && j < 10);
	}

	settings.close();
	return true;
}

// returns count of non-overlapping occurrences of 'sub' in 'str'
int countSubstring(const string& str, const string& sub)
{
	if (sub.length() == 0 || str.length() < sub.length())
	{
		return 0;
	}

	int count = 0;
	for (size_t offset = str.find(sub); offset != std::string::npos; offset = str.find(sub, offset + sub.length()))
	{
		count++;
	}
	return count;
}

BOOL CALLBACK DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	if (fdwReason == DLL_PROCESS_ATTACH)
<<<<<<< HEAD
	{
		hInstance = hinstDLL;
	}
	return TRUE;
}

HINSTANCE GetHinstance()
{
	return hInstance;
}
=======
		hInstance = hinstDLL;

	return TRUE;
}
>>>>>>> 80faf0302e1248b2a6468bfd5f267c90f860ae1d
