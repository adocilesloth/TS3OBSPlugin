/*****************************
2015 <adocilesloth@gmail.com>
*****************************/
#pragma once
#include "OBSApi.h"
#include "resource.h"
#include <string>

#define WIN32_MEAN_AND_LEAN
#include <winsock2.h>

// Entry points
extern "C" __declspec(dllexport) void ConfigPlugin(HWND);

extern "C" __declspec(dllexport) bool LoadPlugin();
extern "C" __declspec(dllexport) void UnloadPlugin();
extern "C" __declspec(dllexport) CTSTR GetPluginName();
extern "C" __declspec(dllexport) CTSTR GetPluginDescription();

extern "C" __declspec(dllexport) void OnStartStream();
extern "C" __declspec(dllexport) void OnStopStream();

bool ConnectToHost(int, char*, SOCKET&);
void CloseConnection(SOCKET&);

char* getIP();
std::wstring Communicate(int, SOCKET&);
bool MuteandDeafen(int, SOCKET&);
bool ChannelSwitch(int, SOCKET&);
int countSubstring(const std::string&, const std::string&);
int wcountSubstring(const std::wstring&, const std::wstring&);

void ShutdownOverlay();
void ResetOverlay();
void RunOverlay(char*);

HINSTANCE GetHinstance();

void ReplaceAll(std::string&, const std::string&, const std::string&);
void wReplaceAll(std::wstring&, const std::wstring&, const std::wstring&);

std::wstring s2ws(const std::string&);
std::string ws2s(const std::wstring&);

int GetNumberOfNames();
bool GetHideSelf();
bool GetRightOfSymbol();
bool GetOnlyShowTalker();
int GetHideNameAfter();