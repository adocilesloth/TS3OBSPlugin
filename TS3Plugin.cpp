/*****************************
2014 <adocilesloth@gmail.com>
*****************************/
#include "TS3Plugin.h"
#include "resource.h"
#include "OverlaySource.h"

#include <fstream>
#include <sstream>
#include <codecvt>

using namespace std;

int countSubstring(const string&, const string&);

//SOCKET obs;
//ofstream file;
wifstream settings;

//config popout stuff
HINSTANCE   hInstance;
//cid for config
wstring cid;
bool bprefix;

//Overlay Stuff
HANDLE OvrThread;

INT_PTR CALLBACK ConfigDlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	settings.imbue(std::locale(std::locale::empty(), new std::codecvt_utf8<wchar_t,0x10ffff, std::consume_header>));
	SOCKET obs;

	//string sip, suid, spref, smute, schan, spw;		//temp strings

	wstring wip, wuid, wpref, wmute, wchan, wcid, wpw;	//sending strings

	bool bmute, bdeaf, bchan;		//bools for checkboxes
	int length;
	wstring path = OBSGetPluginDataPath().Array();
	wofstream osettings;
	osettings.imbue(std::locale(std::locale::empty(), new std::codecvt_utf8<wchar_t,0x10ffff,std::generate_header>));
	
	//get current information
	settings.open(path + L"\\ts3.ini");

	getline(settings, wip);
	//wip = wstring(sip.begin(), sip.end());

	getline(settings, wuid);
	wuid = wuid.substr(6, wuid.length() - 6);	//remove cluid=
	//wuid = wstring(suid.begin(), suid.end());

	getline(settings, wpref);
	if (wpref.length() > 10)
	{
		wpref = wpref.substr(0, 10);
	}
	//wpref = wstring(spref.begin(), spref.end());

	getline(settings, wmute);
	if(wmute == L"1")
	{
		bmute = true;
		bdeaf = false;
	}
	else if(wmute == L"2")
	{
		bmute = false;
		bdeaf = true;
	}
	else if(wmute == L"3")
	{
		bmute = true;
		bdeaf = true;
	}
	else
	{
		bmute = false;
		bdeaf = false;
	}

	getline(settings, wchan);
	if(wchan == L"1")
	{
		bchan = true;
	}
	else
	{
		bchan = false;
	}

	getline(settings, wpw);		//gets cid=..
	wpw.clear();				//so delete it
	getline(settings, wpw);		//gets cpw
	wpw = wpw.substr(5, wpw.length() - 5);	//remove  cpw=
	//wpw = wstring(spw.begin(), spw.end());

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
			SetWindowTextW(PREFInput, wpref.c_str());
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
			SetWindowTextW(PWInput, wpw.c_str());
		}
		if(!bprefix)
		{
			SendMessage(GetDlgItem(hWnd, IDC_PRE), BM_SETCHECK, false ? BST_CHECKED : BST_UNCHECKED, 0);
			SendMessage(GetDlgItem(hWnd, IDC_SUF), BM_SETCHECK, true ? BST_CHECKED : BST_UNCHECKED, 0);

			ShowWindow(GetDlgItem(hWnd, IDC_EPREFTXT), SW_HIDE);
			ShowWindow(GetDlgItem(hWnd, IDC_PREFTXT), SW_HIDE);
			ShowWindow(GetDlgItem(hWnd, IDC_ESUFFTXT), SW_SHOW);
			ShowWindow(GetDlgItem(hWnd, IDC_SUFFTXT), SW_SHOW);
		}
		else
		{
			SendMessage(GetDlgItem(hWnd, IDC_PRE), BM_SETCHECK, true ? BST_CHECKED : BST_UNCHECKED, 0);
			SendMessage(GetDlgItem(hWnd, IDC_SUF), BM_SETCHECK, false ? BST_CHECKED : BST_UNCHECKED, 0);

			ShowWindow(GetDlgItem(hWnd, IDC_EPREFTXT), SW_SHOW);
			ShowWindow(GetDlgItem(hWnd, IDC_PREFTXT), SW_SHOW);
			ShowWindow(GetDlgItem(hWnd, IDC_ESUFFTXT), SW_HIDE);
			ShowWindow(GetDlgItem(hWnd, IDC_SUFFTXT), SW_HIDE);
		}
	}

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDOK:
		{
			//get information
			osettings.open(path + L"\\ts3.ini");

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
			osettings << L"cluid="
					  << uidbuf << endl;	//write to file
			GlobalFree(uidbuf);				//clear buffer
			//prefix
			HWND PREFOutput = GetDlgItem(hWnd, PREFEdt);
			length = GetWindowTextLength(PREFOutput) + 1;
			TCHAR * prefbuf = (TCHAR *)GlobalAlloc(GPTR, length * sizeof(TCHAR));
			GetWindowTextW(PREFOutput, prefbuf, length);
			osettings << prefbuf << endl;	//write to file
			GlobalFree(prefbuf);			//clear buffer
			//mute and deafened
			bmute = SendMessage(GetDlgItem(hWnd, MUTChk), BM_GETCHECK, 0, 0) == BST_CHECKED;
			bdeaf = SendMessage(GetDlgItem(hWnd, DEFChk), BM_GETCHECK, 0, 0) == BST_CHECKED;
			if(!bmute && !bdeaf)
			{
				osettings << L"0" << endl;
			}
			else if(bmute && !bdeaf)
			{
				osettings << L"1" << endl;
			}
			else if(!bmute && bdeaf)
			{
				osettings << L"2" << endl;
			}
			else if(bmute && bdeaf)
			{
				osettings << L"3" << endl;
			}
			//channel switch
			bchan = SendMessage(GetDlgItem(hWnd, CHANChk), BM_GETCHECK, 0, 0) == BST_CHECKED;
			if(bchan)
			{
				osettings << L"1" << endl;
			}
			else
			{
				osettings << L"0" << endl;
			}
			//cid
			wcid = wstring(cid.begin(), cid.end());
			osettings << wcid << endl;
			//password
			HWND PWOutput = GetDlgItem(hWnd, PWEdt);
			length = GetWindowTextLength(PWOutput) + 1;
			TCHAR * pwbuf = (TCHAR *)GlobalAlloc(GPTR, length * sizeof(TCHAR));
			GetWindowText(PWOutput, pwbuf, length);
			osettings << L" cpw="
					  << pwbuf << endl;	//write to file
			GlobalFree(pwbuf);			//clear buffer

			bool bpre = SendMessage(GetDlgItem(hWnd, IDC_PRE), BM_GETCHECK, 0, 0) == BST_CHECKED;
			osettings << bpre;			//write pefix
			bprefix = bpre;				//and reassign global

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

			if(!ConnectToHost(25639, adrs, obs))
			{
				break;
			}

			int iResult;
			int i = 0;
			char *whoami = "whoami\n";
			char reci[256];
			string truereci, scid;

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

			scid = reci;
			cid = s2ws(scid);
			size_t startpos = cid.find(L"cid=");
			size_t endpos = cid.find(L"\n");
			cid = cid.substr(startpos, endpos - startpos);

			CloseConnection(obs);
			break;
		}

		case IDC_PRE:
		case IDC_SUF:
		{
			bool bpre = SendMessage(GetDlgItem(hWnd, IDC_PRE), BM_GETCHECK, 0, 0) == BST_CHECKED;
			bool bsuf = SendMessage(GetDlgItem(hWnd, IDC_SUF), BM_GETCHECK, 0, 0) == BST_CHECKED;
			if(bpre)
			{
				ShowWindow(GetDlgItem(hWnd, IDC_EPREFTXT), SW_SHOW);
				ShowWindow(GetDlgItem(hWnd, IDC_PREFTXT), SW_SHOW);
				ShowWindow(GetDlgItem(hWnd, IDC_ESUFFTXT), SW_HIDE);
				ShowWindow(GetDlgItem(hWnd, IDC_SUFFTXT), SW_HIDE);
			}
			else if(bsuf)
			{
				ShowWindow(GetDlgItem(hWnd, IDC_EPREFTXT), SW_HIDE);
				ShowWindow(GetDlgItem(hWnd, IDC_PREFTXT), SW_HIDE);
				ShowWindow(GetDlgItem(hWnd, IDC_ESUFFTXT), SW_SHOW);
				ShowWindow(GetDlgItem(hWnd, IDC_SUFFTXT), SW_SHOW);
			}
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
	settings.imbue(std::locale(std::locale::empty(), new std::codecvt_utf8<wchar_t,0x10ffff, std::consume_header>));

	OBSRegisterImageSourceClass(L"OverlaySource", L"TeamSpeak 3 Overlay", (OBSCREATEPROC)CreateOverlaySource, (OBSCONFIGPROC)ConfigureOverlaySource);

	wstring path = OBSGetPluginDataPath().Array();
	settings.open(path + L"\\ts3.ini");

	if (!settings.is_open())
	{
		wofstream create(path + L"\\ts3.ini");
		create.imbue(std::locale(std::locale::empty(), new std::codecvt_utf8<wchar_t,0x10ffff,std::generate_header>));
		create << L"127.0.0.1" << endl;
		create << L"cluid=" << endl;
		create << L"*R*" << endl;
		create << L"0" << endl;
		create << L"0" << endl;
		create << L"cid=1" << endl;
		create << L" cpw=" << endl;
		create << L"1";
		create.close();		//stop using settings file

		cid = L"cid=1";		//set cid string
		bprefix = 1;
	}
	else
	{
		for(int i = 0; i < 6; i++)
		{
			getline(settings, cid);	//set cid string
		}
		wstring tmp;
		getline(settings, tmp);	//get rid of " cpw="
		settings >> bprefix;	//get if prefix or suffix
		settings.close();		//stop using settings file
	}

	AppWarning(TEXT("TS3Plugin Loaded"));
	return true;
}

void UnloadPlugin()
{
	//get settings file path
	wstring path = OBSGetPluginDataPath().Array();
	path.append(L"/ts3temp.ini");
	//test if ts3temp.ini exists. If yes, delete
	ofstream ssettings;
	ssettings.open(path);
	if(ssettings.is_open())
	{
		ssettings.close();
		_wremove(path.c_str());
	}
}

CTSTR GetPluginName()
{
	return TEXT("Teamspeak 3 Plugin");
}

CTSTR GetPluginDescription()
{
	return TEXT("Adds *R* (or any other prefix) before TS3 nickname when recording. Options to mute/deafen self and move channel while recording. Undos all effects when recording stops. Also adds a simple ovelay as a source.");
}

void OnStartStream()
{
	char *adrs = getIP();

	OvrThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)RunOverlay, adrs, 0, 0);

	return;
}

void OnStopStream()
{
	char *adrs = getIP();

	ShutdownOverlay();
	WaitForSingleObject(OvrThread, INFINITE);
	ResetOverlay();

	return;
}

char* getIP()
{
	settings.imbue(std::locale(std::locale::empty(), new std::codecvt_utf8<wchar_t,0x10ffff, std::consume_header>));

	wstring path = OBSGetPluginDataPath().Array();
	wstring wIPadrsstr;

	settings.open(path + L"\\ts3.ini");
	getline(settings, wIPadrsstr);

	string IPadrsstr(wIPadrsstr.begin(), wIPadrsstr.end());
	char *IPadrs = new char[IPadrsstr.length() + 1];
	strcpy(IPadrs, IPadrsstr.c_str());

	settings.close();
	return IPadrs;
}

wstring getIPwstring()
{
	settings.imbue(std::locale(std::locale::empty(), new std::codecvt_utf8<wchar_t,0x10ffff, std::consume_header>));

	wstring path = OBSGetPluginDataPath().Array();
	wstring wIPadrsstr;

	settings.open(path + L"\\ts3.ini");
	getline(settings, wIPadrsstr);

	settings.close();
	return wIPadrsstr;
}

wstring Communicate(int cont, SOCKET &obs)
{
	AppWarning(TEXT("Communicate"));

	settings.imbue(std::locale(std::locale::empty(), new std::codecvt_utf8<wchar_t,0x10ffff, std::consume_header>));

	wstring poop = L"poop";	//failure return

	int iResult;
	char *notify = "clientnotifyregister schandlerid=1 event=notifyclientnamefromuid\n";
	wstringstream wnewname;
	wnewname << L"clientupdate client_nickname=";
	char reci1[256];
	char reci2[256];
	char reci3[256];
	char reci4[256];
	string space = "\\s";
	wstring wspace = L"\\s";

	//debug file
	//file.open("C:/Program Files (x86)/OBS/plugins/outfile.txt");

	//get settings file path
	wstring path = OBSGetPluginDataPath().Array();
	//get cluid and recording prefix from ts3.ini
	settings.open(path + L"\\ts3.ini");
	wstring cluid;
	getline(settings, cluid);	//first line is ip address
	cluid.clear();				//so we don't want it
	getline(settings, cluid);
	wstring rec;
	getline(settings, rec);
	wReplaceAll(rec, L" ", wspace);		//replace spaces with \s
	int modcount = wcountSubstring(rec, wspace);	//number of \s

	//set up the getname call
	wstring wtempgetname = L"clientgetnamefromuid ";
	wtempgetname.append(cluid);
	wtempgetname.append(L"\n");
	string tempgetname(wtempgetname.begin(), wtempgetname.end());
	const char *getname = tempgetname.c_str();

	//debug string
	wstringstream DEBUG;

	if(cont != 1)
	{
		iResult = recv(obs, reci1, 256 ,0);	//get TS3 Client...
		if (iResult == SOCKET_ERROR)
		{
			AppWarning(TEXT("Communicate: First Recieve Failure"));
			AppWarning(TEXT("SOCKET_ERROR"));
			wstringstream code;
			code << WSAGetLastError();
			AppWarning(code.str().c_str());
			settings.close();
			return poop;
		}

		AppWarning(getIPwstring().c_str());		//use this to stop the next if going bad.
		if(getIPwstring() != L"127.0.0.1")
		{
			AppWarning(TEXT("Communicate: Non-Local Shutdown"));
			iResult = recv(obs, reci1, 256 ,0);	//get TS3 Client...
			if (iResult == SOCKET_ERROR)
			{
				AppWarning(TEXT("Communicate: Remote First Recieve Failure"));
				AppWarning(TEXT("SOCKET_ERROR"));
				wstringstream code;
				code << WSAGetLastError();
				AppWarning(code.str().c_str());
				settings.close();
				return poop;
			}
		}
	}

	//notifyregister
	iResult = send(obs, notify, (int)strlen(notify), 0);	//request notifyregister...
	if (iResult == SOCKET_ERROR)
	{
		AppWarning(TEXT("Communicate: notifyregister Send Failure"));
		AppWarning(TEXT("SOCKET_ERROR"));
		wstringstream code;
		code << WSAGetLastError();
		AppWarning(code.str().c_str());
		settings.close();
		return poop;
	}
	Sleep(50);
	iResult = recv(obs, reci2, 256, 0);	//recieve result: error id=0...
	if (iResult == SOCKET_ERROR)
	{
		AppWarning(TEXT("Communicate: notifyregister Recieve Failure"));
		AppWarning(TEXT("SOCKET_ERROR"));
		wstringstream code;
		code << WSAGetLastError();
		AppWarning(code.str().c_str());
		settings.close();
		return poop;
	}

	for(int i = 0; i < 10; i++)
	{
		DEBUG << reci2[i];
	}
	AppWarning(DEBUG.str().c_str());	//should not be Welcome to
	DEBUG.str(L"");						//should be error id=0

	//clientnamefromuid
	iResult = send(obs, getname, (int)strlen(getname), 0);	//request clientnamefromuid...
	if (iResult == SOCKET_ERROR)
	{
		AppWarning(TEXT("Communicate: clientnamefromuid Send Failure"));
		AppWarning(TEXT("SOCKET_ERROR"));
		wstringstream code;
		code << WSAGetLastError();
		AppWarning(code.str().c_str());
		settings.close();
		return poop;
	}
	Sleep(50);
	iResult = recv(obs, reci3, 256, 0);	//recieve name
	if (iResult == SOCKET_ERROR)
	{
		AppWarning(TEXT("Communicate: clientnamefromuid Recieve Failure"));
		AppWarning(TEXT("SOCKET_ERROR"));
		wstringstream code;
		code << WSAGetLastError();
		AppWarning(code.str().c_str());
		settings.close();
		return poop;
	}

	for(int i = 0; i < 10; i++)
	{
		DEBUG << reci3[i];
	}
	AppWarning(DEBUG.str().c_str());	//should not be error id=0
	DEBUG.str(L"");						//should be notifyclie

	//get name
	wstring identstart = L"name=";
	wstring identend = L"\n";
	string name = reci3;
	wstring wname = s2ws(name);

	size_t startpos = wname.find(identstart);	//start of name
	if(startpos == -1)
	{
		AppWarning(TEXT("Communicate: startpos == -1"));
		return poop;
	}
	size_t endpos = wname.find(identend);
	if(endpos < startpos)
	{
		AppWarning(TEXT("Communicate: endpos < startpos"));
		return poop;
	}
	wname = wname.substr(startpos+5, endpos-startpos-5);
	int count = wcountSubstring(wname, wspace);	//number of \s
	//wname = wname.substr(startpos+5 , 30 + count);
	//get name end

	if(!bprefix)	//if using suffix
	{
		size_t spc = wname.find(L"\n");
		wname = wname.substr(0, spc);
		int nstrt = wname.length();
		nstrt = nstrt - rec.length();
		if(nstrt < 0)
		{
			nstrt = 0;
		}
		int nlen = wname.length() - count;

		if(cont == 1)			//adding modifier
		{
			if(wname.substr(nstrt) != rec)
			{
				if(nlen > 30)
				{
					wname = wname.substr(0, 30 + count - rec.length());
				}
				wnewname << wname
						 << rec << L"\n";		//finish name set string
			}
		}
		else if(cont == 0)		//removing modifier
		{
			if(wname.substr(nstrt) == rec)
			{
				wname = wname.substr(0, nlen + count - rec.length());
			}
			wnewname << wname << L"\n";		//finish name set string
		}
	}
	else	//if using prefix
	{
		if(cont == 1)			//adding modifier
		{
			if(wname.substr(0, rec.length()) != rec)
			{
				wname = wname.substr(0, 30 + count - rec.length());
				wnewname << rec;	//finish name set string
			}
		}
		else if(cont == 0)		//removing modifier
		{
			if(wname.substr(0, rec.length()) == rec)
			{
				wname = wname.substr(rec.length(), 30 + count - modcount - rec.length());
			}
		}
		wnewname << wname << L"\n";			//finish name set string
	}

	//name for return
	wstring rname = rec;
	rname.append(wname);

	AppWarning(wnewname.str().c_str());		//print name being sent

	const string tmp = ws2s(wnewname.str());	//set name to string
	const char* recname = tmp.c_str();	//set name to char* so it can be sent

	//clientupdate
	iResult = send(obs, recname, (int)strlen(recname), 0);
	if (iResult == SOCKET_ERROR)
	{
		AppWarning(TEXT("Communicate: clientupdate Send Failure"));
		AppWarning(TEXT("SOCKET_ERROR"));
		wstringstream code;
		code << WSAGetLastError();
		AppWarning(code.str().c_str());
		settings.close();
		return poop;
	}
	Sleep(50);
	iResult = recv(obs, reci4, 256 ,0);
	if (iResult == SOCKET_ERROR)
	{
		AppWarning(TEXT("Communicate: clientupdate Recieve Failure"));
		AppWarning(TEXT("SOCKET_ERROR"));
		wstringstream code;
		code << WSAGetLastError();
		AppWarning(code.str().c_str());
		settings.close();
		return poop;
	}

	for(int i = 0; i < 50; i++)
	{
		DEBUG << reci4[i];
	}
	AppWarning(DEBUG.str().c_str());	//should be error id=0
	DEBUG.str(L"");

	//file.close();
	settings.close();
	return rname;
}

bool MuteandDeafen(int state, SOCKET &obs)
{
	AppWarning(TEXT("MuteandDeafen"));

	settings.imbue(std::locale(std::locale::empty(), new std::codecvt_utf8<wchar_t,0x10ffff, std::consume_header>));

	//get settings file path
	wstring path = OBSGetPluginDataPath().Array();
	//get mute and deafen settings from ts3.ini
	settings.open(path + L"\\ts3.ini");
	wstring mnd;
	getline(settings, mnd);		//ip
	mnd.clear();
	getline(settings, mnd);		//cluid
	mnd.clear();
	getline(settings, mnd);		//prefix
	mnd.clear();
	getline(settings, mnd);		//mute and deafened state

	if(mnd != L"1" && mnd != L"2" && mnd != L"3")	//if not set to mute or deafen
	{
		settings.close();
		return true;
	}

	int iResult;
	stringstream sstate;
	sstate << state << "\n";

	if(mnd == L"1" || mnd == L"3")	//if set to mute
	{
		char reci1[256];;

		string tempmute = "clientupdate client_input_muted=";
		tempmute.append(sstate.str());
		const char *mute = tempmute.c_str();

		iResult = send(obs, mute, (int)strlen(mute), 0);	//set mute
		if (iResult == SOCKET_ERROR)
		{
			AppWarning(TEXT("MuteandDeafen: Mute Send Failure"));
			AppWarning(TEXT("SOCKET_ERROR"));
			wstringstream code;
			code << WSAGetLastError();
			AppWarning(code.str().c_str());
			settings.close();
			return false;
		}
		Sleep(50);
		iResult = recv(obs, reci1, 256, 0);	//recieve result: error id=0...
		if (iResult == SOCKET_ERROR)
		{
			AppWarning(TEXT("MuteandDeafen: Mute Recieve Failure"));
			AppWarning(TEXT("SOCKET_ERROR"));
			wstringstream code;
			code << WSAGetLastError();
			AppWarning(code.str().c_str());
			settings.close();
			return false;
		}
	}

	if(mnd == L"2" || mnd == L"3")
	{
		char reci2[256];
	
		string tempdeaf = "clientupdate client_output_muted=";
		tempdeaf.append(sstate.str());
		const char *deaf = tempdeaf.c_str();

		iResult = send(obs, deaf, (int)strlen(deaf), 0);	//set deafen
		if (iResult == SOCKET_ERROR)
		{
			AppWarning(TEXT("MuteandDeafen: Deaf Send Failure"));
			AppWarning(TEXT("SOCKET_ERROR"));
			wstringstream code;
			code << WSAGetLastError();
			AppWarning(code.str().c_str());
			settings.close();
			return false;
		}
		Sleep(50);
		iResult = recv(obs, reci2, 256, 0);	//recieve result: error id=0...
		if (iResult == SOCKET_ERROR)
		{
			AppWarning(TEXT("MuteandDeafen: Deaf Recieve Failure"));
			AppWarning(TEXT("SOCKET_ERROR"));
			wstringstream code;
			code << WSAGetLastError();
			AppWarning(code.str().c_str());
			settings.close();
			return false;
		}
	}
	
	settings.close();
	return true;
}

bool ChannelSwitch(int state, SOCKET &obs)
{
	AppWarning(TEXT("ChannelSwitch"));

	ifstream ssettings;
	ssettings.imbue(std::locale(std::locale::empty(), new std::codecvt_utf8<wchar_t,0x10ffff, std::consume_header>));
	//get settings file path
	wstring path = OBSGetPluginDataPath().Array();
	//get mute and deafen settings from ts3.ini
	ssettings.open(path + L"\\ts3.ini");
	string swtch;
	getline(ssettings, swtch);		//ip
	swtch.clear();
	getline(ssettings, swtch);		//cluid
	swtch.clear();
	getline(ssettings, swtch);		//prefix
	swtch.clear();
	getline(ssettings, swtch);		//mute and deafened state
	swtch.clear();
	getline(ssettings, swtch);		//switch state

	if(swtch != "1")	//if not set to switch
	{
		ssettings.close();
		return true;
	}

	int iResult;
	char *whoami = "whoami\n";
	char reci[256];
	string truereci, tcid, rcid, clid, cpw;

	iResult = send(obs, whoami, (int)strlen(whoami), 0);	//request whoami
	if (iResult == SOCKET_ERROR)
	{
		AppWarning(TEXT("ChannelSwitch: whoami Send Failure"));
		AppWarning(TEXT("SOCKET_ERROR"));
		wstringstream code;
		code << WSAGetLastError();
		AppWarning(code.str().c_str());
		ssettings.close();
		return false;
	}
	Sleep(50);
	iResult = recv(obs, reci, 256, 0);	//recieve result: clid=XX cid=XXXX
	if (iResult == SOCKET_ERROR)
	{
		AppWarning(TEXT("ChannelSwitch: whoami Recieve Failure"));
		AppWarning(TEXT("SOCKET_ERROR"));
		wstringstream code;
		code << WSAGetLastError();
		AppWarning(code.str().c_str());
		ssettings.close();
		return false;
	}

	clid = reci;
	size_t space = clid.find("cid=");
	if(space == -1)
	{
		ssettings.close();
		AppWarning(TEXT("ChannelSwitch: space == -1"));
		return false;
	}
	clid = clid.substr(0, space);

	string channel = "clientmove ";
	channel.append(clid);

	char reci2[256];

	if(state == 1)	//switch to channel
	{
		//write return cid to file
		rcid = reci;
		size_t startpos = rcid.find("cid=");
		if(startpos == -1)
		{
			AppWarning(TEXT("ChannelSwitch: startpos == -1"));
			ssettings.close();
			return false;
		}
		size_t endpos = rcid.find("\n");
		if(endpos - startpos < 0)
		{
			AppWarning(TEXT("ChannelSwitch: endpos - startpos < 0"));
			ssettings.close();
			return false;
		}
		rcid = rcid.substr(startpos, endpos - startpos);

		ofstream rturn(path + L"\\ts3temp.ini");
		rturn << rcid;
		rturn.close();

		//move channel
		getline(ssettings, tcid);
		getline(ssettings, cpw);
		channel.append(tcid);
		channel.append(cpw);
		channel.append("\n");
		const char *move = channel.c_str();

		iResult = send(obs, move, (int)strlen(move), 0);	//set deafen
		if (iResult == SOCKET_ERROR)
		{
			AppWarning(TEXT("ChannelSwitch: Move Send Failure"));
			AppWarning(TEXT("SOCKET_ERROR"));
			wstringstream code;
			code << WSAGetLastError();
			AppWarning(code.str().c_str());
			ssettings.close();
			return false;
		}
		Sleep(50);
		iResult = recv(obs, reci2, 256, 0);	//recieve result: error id=0...
		if (iResult == SOCKET_ERROR)
		{
			AppWarning(TEXT("ChannelSwitch: Move Recieve Failure"));
			AppWarning(TEXT("SOCKET_ERROR"));
			wstringstream code;
			code << WSAGetLastError();
			AppWarning(code.str().c_str());
			ssettings.close();
			return false;
		}
	}
	else	//switch back
	{
		//get return cid
		ifstream rturn(path + L"\\ts3temp.ini");
		if(!rturn.is_open())	//set target as same channel
		{
			rcid = reci;
			size_t startpos = rcid.find("cid=");
			if(startpos == -1)
			{
				AppWarning(TEXT("ChannelSwitch: startpos == -1"));
				ssettings.close();
				return false;
			}
			size_t endpos = rcid.find("\n");
			if(endpos - startpos < 0)
			{
				AppWarning(TEXT("ChannelSwitch: endpos - startpos < 0"));
				ssettings.close();
				return false;
			}
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

		iResult = send(obs, move, (int)strlen(move), 0);	//set deafen
		if (iResult == SOCKET_ERROR)
		{
			AppWarning(TEXT("ChannelSwitch: Move Send Failure"));
			AppWarning(TEXT("SOCKET_ERROR"));
			wstringstream code;
			code << WSAGetLastError();
			AppWarning(code.str().c_str());
			ssettings.close();
			return false;
		}
		Sleep(50);
		iResult = recv(obs, reci2, 256, 0);	//recieve result: error id=0...
		if (iResult == SOCKET_ERROR)
		{
			AppWarning(TEXT("ChannelSwitch: Move Recieve Failure"));
			AppWarning(TEXT("SOCKET_ERROR"));
			wstringstream code;
			code << WSAGetLastError();
			AppWarning(code.str().c_str());
			ssettings.close();
			return false;
		}
	}

	ssettings.close();
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

int wcountSubstring(const wstring& str, const wstring& sub)
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
	{
		hInstance = hinstDLL;
	}
	return TRUE;
}

HINSTANCE GetHinstance()
{
	return hInstance;
}

void ReplaceAll(string &str, const string& from, const string& to)
{
    size_t start_pos = 0;
    while((start_pos = str.find(from, start_pos)) != string::npos)
	{
        str.replace(start_pos, from.length(), to);
        start_pos += to.length(); // Handles case where 'to' is a substring of 'from'
    }
    return;
}

void wReplaceAll(wstring &str, const wstring& from, const wstring& to)
{
    size_t start_pos = 0;
    while((start_pos = str.find(from, start_pos)) != string::npos)
	{
        str.replace(start_pos, from.length(), to);
        start_pos += to.length(); // Handles case where 'to' is a substring of 'from'
    }
    return;
}

wstring s2ws(const string& str)
{
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
    wstring wstrTo( size_needed, 0 );
    MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
    return wstrTo;
}

string ws2s(const wstring& wstr)
{
	int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
    std::string strTo( size_needed, 0 );
    WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &strTo[0], size_needed, NULL, NULL);
    return strTo;
}