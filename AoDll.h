#include "stdafx.h"
#include "detours.h"
#include <Windows.h>
#include <string>
#include <sstream>
#include <vector>
#include <map>
#include <iterator>
#include <comutil.h>
#include <memory>
//#include "AORConfig.h"
#include "Player.h"
#include <TlHelp32.h>
#include <tchar.h>

#pragma comment(lib, "comsuppw.lib")

//
//// Methods
//
void Hooks();
VOID WINAPI MyRecvData(BSTR recvData);        // My DataHandler Method
VOID WINAPI MySendData(BSTR* sendData);        // My SendData Method
int WINAPI MyLoop();                        // My Loop Method
VOID SendToClient(string message);            // Sends Packets to Client
VOID SendToServer(string message);            // Sends Packets to Server
VOID ShowMap();
VOID SwitchPlayerAutoAim();
VOID ChangeTarget(bool original, Player* player);
VOID AutoAim();
VOID CastRemo(int posX, int posY);
VOID AutoPotas();
BOOL StartsWith(BSTR sValue, const WCHAR* pszSubValue);
BSTR ConvertStringToBSTR(const std::string & str);
string ConvertBSTRToString(BSTR bstr);
string ConvertWCSToMBS(const wchar_t* pstr, long wslen);
VOID CastSpell(int slot);
VOID SpeedHack();
VOID EraseConsole();
VOID AddPlayer(std::vector<string> split);
VOID HideInvisiblePlayers(bool hide);