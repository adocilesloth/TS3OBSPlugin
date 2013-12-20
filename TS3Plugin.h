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
#pragma once
#include "OBSApi.h"
#include "resource.h"
#include <string>

// Entry points
extern "C" __declspec(dllexport) void ConfigPlugin(HWND);

extern "C" __declspec(dllexport) bool LoadPlugin();
extern "C" __declspec(dllexport) void UnloadPlugin();
extern "C" __declspec(dllexport) CTSTR GetPluginName();
extern "C" __declspec(dllexport) CTSTR GetPluginDescription();

extern "C" __declspec(dllexport) void OnStartStream();
extern "C" __declspec(dllexport) void OnStopStream();

char* getIP();
bool ConnectToHost(int, char*);
void CloseConnection();
bool Communicate(int);
bool MuteandDeafen(int);
bool ChannelSwitch(int);
int countSubstring(const std::string&, const std::string&);
