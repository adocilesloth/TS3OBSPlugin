/*****************************
2014 <adocilesloth@gmail.com>
*****************************/
#ifndef OVERLAYSOURCE_H
#define OVERLAYSOURCE_H
#include "OBSApi.h"
#include "TS3Plugin.h"

inline DWORD GetAlphaVal(UINT);
struct ConfigOverlaySourceInfo;
ImageSource* STDCALL CreateOverlaySource(XElement*);
int CALLBACK OvrFontEnumProcThingy(ENUMLOGFONTEX*, NEWTEXTMETRICEX*, DWORD, ConfigOverlaySourceInfo*);
void OvrDoCancelStuff(HWND);
UINT OvrFindFontFace(ConfigOverlaySourceInfo*, HWND, CTSTR);
UINT OvrFindFontName(ConfigOverlaySourceInfo*, HWND, CTSTR);
CTSTR OvrGetFontFace(ConfigOverlaySourceInfo*, HWND);
INT_PTR CALLBACK ConfigureOverlayProc(HWND, UINT, WPARAM, LPARAM);
bool STDCALL ConfigureOverlaySource(XElement*, bool);

#endif //OVERLAYSOURCE_H