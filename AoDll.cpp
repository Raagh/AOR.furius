﻿#include "stdafx.h"
#include <thread>
#include "AoDll.h"
#include <algorithm>
#include <vector>
#include "Toggle.h"
#include "KeyToggle.h"
#include <iostream>

using namespace std;

//
//// Variables
//
int selectedPlayerId = 0;
int cheaterPosX = 0;
int cheaterPosY = 0;
bool playerCStatus = true;
bool statusSpeedHack = false;
bool hideCheat = false;
bool cheatStatus = false;
bool verInvisOn = false;
bool cheaterParalizado = false;
DWORD oldTTeclas = GetTickCount();
DWORD ttAutoPot = GetTickCount();
HANDLE handleAim;
HANDLE handlePotas;
string playerName = "";
string selectedPlayerName = "";
string positionApoca;
string positionRemo;
string positionInmo;
string positionDescarga;

FILE *fpstdin = stdin, *fpstdout = stdout, *fpstderr = stderr;

vector<string> &split(const string &s, char delim, vector<string> &elems)
{
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim))
    {
        elems.push_back(item);
    }
    return elems;
}
vector<string> split(const string &s, char delim)
{
    std::vector<std::string> elems;
    split(s, delim, elems);
    return elems;
}

//
//// Maps
//
map<int /*ID*/, Player * /*Entity*/> CharactersInMap;
map<int /*ID*/, Player * /*Entity*/> CharactersInRange;
vector<string /*Packet*/> Packets;

//
//// Pointer Functions Declarations
//
typedef VOID(WINAPI *PRecvData)(BSTR data);    //Pointer Definition - Takes BSTR and returns VOID
PRecvData PFunctionRecv = (PRecvData)0x731220; //Pointer to where the original HandleData() starts

typedef VOID(WINAPI *PSendData)(BSTR *data);   //Pointer Definition - Takes BSTR* and returns VOID
PSendData PFunctionSend = (PSendData)0x7867D0; //Pointer to where the original SendData() starts

typedef int(WINAPI *PLoop)();
HMODULE dllModule = LoadLibraryA("MSVBVM60.DLL");
PLoop PFunctionLoop = (PLoop)GetProcAddress(dllModule, "rtcDoEvents");

void Hooks()
{
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    DetourAttach(&(PVOID &)PFunctionRecv, &MyRecvData); // Hook DataHandler() to MyRecvData()
    DetourAttach(&(PVOID &)PFunctionSend, &MySendData); // Hook SendData() to MySendData()
    DetourAttach(&(PVOID &)PFunctionLoop, &MyLoop);     // Hook DoEvents AKA Loop() to MyLoop()
    DetourTransactionCommit();
}

VOID WINAPI MyRecvData(BSTR dataRecv)
{
    __asm PUSHAD;
    __asm PUSHFD;

    try
    {
        OutputDebugStringW(ConvertStringToBSTR("Recv: " + ConvertBSTRToString(dataRecv)));

        //
        ////Game masters are taking picture of your computer
        //
        if (StartsWith(dataRecv, L"PAIN"))
        {
            hideCheat = true;
            EraseConsole();
        }

        //
        ////If the players throws Invisibilidad
        //
        if (StartsWith(dataRecv, L"V3"))
        {
            wstring convertedRecv = dataRecv;
            string str(convertedRecv.begin(), convertedRecv.end());
            vector<string> playerInfo = split(str, ',');
            int playerId = stoi(playerInfo[0].substr(2));
            int isInvisible = stoi(playerInfo[1]);

            auto foundPlayer = CharactersInMap.find(playerId);
            if (foundPlayer != CharactersInMap.end())
            {
                string name = foundPlayer->second->name;
                int posX = foundPlayer->second->posX;
                int posY = foundPlayer->second->posY;
                int faction = foundPlayer->second->faction;
                foundPlayer->second->isInvisible = (isInvisible == 1);

                if (isInvisible == 1)
                {
                    string packetExit = "QDL" + to_string(playerId);
                    string packetInvi = "RM,,," + to_string(playerId) + "," + to_string(posX) + "," + to_string(posY) + ",10,1,0,999,3," + name + " [INVI]" + "," + to_string(faction) + ",0,0";
                    SendToClient(packetExit);
                    SendToClient(packetInvi);
                    dataRecv = ConvertStringToBSTR("V3" + to_string(playerId) + ",0");
                }
                else if (isInvisible == 0)
                {
                    string packetExit = "QDL" + to_string(playerId);
                    string packetInvi = "RM,,," + to_string(playerId) + "," + to_string(posX) + "," + to_string(posY) + ",10,1,0,999,3," + name + "," + to_string(faction) + ",0,0";
                    SendToClient(packetExit);
                    SendToClient(packetInvi);
                }
            }
        }

        //////
        ////////Checks if there is any Chest in the map
        //////
        if (!hideCheat)
        {
            if (StartsWith(dataRecv, L"HO"))
            {
                wstring convertedRecv = dataRecv;
                string str(convertedRecv.begin(), convertedRecv.end());
                vector<string> info = split(str, ',');
                string itemID = info[0];
                itemID = itemID.substr(2);

                if (wcsstr(dataRecv, L"24078") != NULL)
                {
                    SendToClient("||AOR> Cofre encontrado en x:" + info[1] + " y: " + info[2] + "~253~103~3~0~0");
                }
            }
        }

        ////
        //// Gets Players Data when you enter a new Map
        ////
        if (StartsWith(dataRecv, L"RM"))
        {
            wstring convertedRecv = dataRecv;
            string str(convertedRecv.begin(), convertedRecv.end());
            vector<string> charactersInfo = split(str, ',');

            if (charactersInfo.size() > 11)
            {
                if (wcsstr(dataRecv, L"Staff") == NULL)
                {
					enum eFacciones
					{
						CIUDA = 2, //Poner el número que se obtiene desde RM
						CRIMI = 3,
						NEUTRAL = 5,
						NEW = 4,
					};

					int faction = stoi(charactersInfo[12]);
					std::string strFaction = "";

					switch (faction)
					{
					case eFacciones::CIUDA:
						strFaction = "Ciudadano";

						break;
					case eFacciones::CRIMI:
						strFaction = "Criminal";

						break;
					case eFacciones::NEW:
						strFaction = "Newibe";

						break;
					case eFacciones::NEUTRAL:
						strFaction = "Neutral";

						break;
					}

					int posX = stoi(charactersInfo[4]);
					int posY = stoi(charactersInfo[5]);
                    string name = charactersInfo[11];
                    if (name != playerName)
                    {
						if (!playerCStatus)
						{
							SendToClient("||Player> Nick: " + name + " Faction: " + strFaction + " X:" + to_string(posX) + " Y:" + to_string(posY) + " ~253~103~3~0~0");
						}

                        AddPlayer(charactersInfo);
                        if (stoi(charactersInfo[13]) == 1)
                        {
                            BSTR playerInvi = ConvertStringToBSTR(charactersInfo[0] + "," + charactersInfo[1] + "," + charactersInfo[2] + "," + charactersInfo[3] + "," + charactersInfo[4] + "," + charactersInfo[5] + "," + charactersInfo[6] + "," + charactersInfo[7] + "," + charactersInfo[8] + "," + charactersInfo[9] + "," + charactersInfo[10] + "," + charactersInfo[11] + " [INVI]" + "," + charactersInfo[12] + "," + "0" + "," + charactersInfo[14]);
                            dataRecv = playerInvi;
                        }
                    }
                }
                else if (wcsstr(dataRecv, L"Staff") != NULL &&
                         wcsstr(dataRecv, L"Esclavista") == NULL &&
                         wcsstr(dataRecv, L"Mansel") == NULL)
                {
                    int posX = stoi(charactersInfo[4]);
                    int posY = stoi(charactersInfo[5]);
                    string name = charactersInfo[11];
                    statusSpeedHack = false;
                    if (!hideCheat)
                    {
                        SendToClient("||AOR> GM ->  " + name + " posx:" + to_string(posX) + " posy:" + to_string(posY) + " ~253~103~3~0~0");
                        if (stoi(charactersInfo[13]) == 1)
                        {
                            BSTR newGm = ConvertStringToBSTR(charactersInfo[0] + "," + charactersInfo[1] + "," + charactersInfo[2] + "," + charactersInfo[3] + "," + charactersInfo[4] + "," + charactersInfo[5] + "," + charactersInfo[6] + "," + charactersInfo[7] + "," + charactersInfo[8] + "," + charactersInfo[9] + "," + charactersInfo[10] + "," + charactersInfo[11] + " [INVI]" + "," + charactersInfo[12] + "," + "0" + "," + charactersInfo[14]);
                            dataRecv = newGm;
                        }
                    }
                }
            }
        }

        ////
        ////// Gets the Players Moving in your Vision Range
        ////
        if (StartsWith(dataRecv, L"MP"))
        {
            wstring convertedRecv = dataRecv;
            string str(convertedRecv.begin(), convertedRecv.end());

            vector<string> splitVector = split(str, ',');

            if (splitVector.size() != 0)
            {
                string idPlayerString = splitVector[0];
                idPlayerString = idPlayerString.substr(2);
                int idPlayer = stoi(idPlayerString);
                int posX = stoi(splitVector[1]);
                int posY = stoi(splitVector[2]);
                auto itMap = CharactersInMap.find(idPlayer);
                if (itMap != CharactersInMap.end())
                {
                    string name = itMap->second->name;
                    int alianza = itMap->second->faction;
                    if (CharactersInRange.size() == 0)
                    {
                        selectedPlayerName = name;
                        selectedPlayerId = idPlayer;
                        Player *newPlayer = new Player();
                        newPlayer->id = idPlayer;
                        newPlayer->posX = posY;
                        newPlayer->posY = posX;
                        newPlayer->name = name;
                        newPlayer->faction = alianza;
                        CharactersInRange.insert(std::pair<int, Player *>(idPlayer, newPlayer));
                        ChangeTarget(false, newPlayer);
                    }
                    else if (CharactersInRange.size() > 0)
                    {
                        auto itRange = CharactersInRange.find(idPlayer);
                        if (itRange != CharactersInRange.end())
                        {
                            itRange->second->posX = posY;
                            itRange->second->posY = posX;
                        }
                        else
                        {
                            Player *newPlayer = new Player();
                            newPlayer->name = name;
                            newPlayer->posX = posY;
                            newPlayer->posY = posX;
                            newPlayer->faction = alianza;
                            CharactersInRange.insert(std::pair<int, Player *>(idPlayer, newPlayer));
                        }
                    }
                }
            }
        }

        ////
        ////// Cheater exits the map
        ////
        if (StartsWith(dataRecv, L"CM"))
        {
            CharactersInMap.clear();
            CharactersInRange.clear();
            selectedPlayerId = 0;
            selectedPlayerName = "";
        }

        ////
        ////// Players exit the map
        ////
        if (StartsWith(dataRecv, L"QDL"))
        {
            wstring convertedRecv = dataRecv;
            string str(convertedRecv.begin(), convertedRecv.end());

            int playerExit = stoi(str.substr(3));
            if (CharactersInRange.size() > 0)
            {
                auto it = CharactersInRange.find(playerExit);
                if (it != CharactersInRange.end())
                {
                    if (playerExit == selectedPlayerId)
                    {
                        SwitchPlayerAutoAim();
                    }
                    CharactersInRange.erase(it);
                }
            }
        }

        ////
        ////// If you Get Removed from Inmo spell
        ////
        if (wcsstr(dataRecv, L"P8") != NULL)
        {
            cheaterParalizado = false;
        }

        ////
        ////// If you Get paralized
        ////
        if (wcsstr(dataRecv, L"P9") != NULL)
        {
            cheaterParalizado = true;
        }

        ////
        ////// If the player Gets paralized, Cast Remover Paralisis.
        ////
        if (StartsWith(dataRecv, L"PU"))
        {
            if (cheaterParalizado)
            {
                wstring convertedRecv = dataRecv;
                string str(convertedRecv.begin(), convertedRecv.end());
                vector<string> splitVector = split(str, ',');

                int posY = stoi(splitVector[0].substr(2));
                int posX = stoi(splitVector[1]);
                cheaterPosX = posY;
                cheaterPosY = posX;	
            }
        }

        ////
        //////Gets the spells positions
        ////
        if (StartsWith(dataRecv, L"SHS"))
        {
            wstring convertedRecv = dataRecv;
            string str(convertedRecv.begin(), convertedRecv.end());

            vector<string> splitVector = split(str, ',');
            string spellPosition = splitVector[0];
            string spellName = splitVector[2];
            spellPosition = spellPosition.substr(3);
            if (wcsstr(dataRecv, L"Apocalipsis") != NULL)
            {
                positionApoca = spellPosition;
            }
            if (wcsstr(dataRecv, L"Inmovilizar") != NULL)
            {
                positionInmo = spellPosition;
            }
            if (wcsstr(dataRecv, L"Remover paralisis") != NULL)
            {
                positionRemo = spellPosition;
            }
            if (wcsstr(dataRecv, L"Descarga") != NULL)
            {
                positionDescarga = spellPosition;
            }
        }

        //
        //// Returns control to the Original Function
        //
        PFunctionRecv(dataRecv);
    }
    catch (int e)
    {
        OutputDebugStringW(ConvertStringToBSTR("ERROR-> Place: HandleData()  Id: " + to_string(e)));
    }

    __asm POPFD;
    __asm POPAD;
}

VOID WINAPI MySendData(BSTR *dataSend)
{
    __asm PUSHAD;
    __asm PUSHFD;

    try
    {
        bool sendOk = true;

        //OutputDebugStringW(ConvertStringToBSTR("Send: " + ConvertBSTRToString(*dataSend)));

        //
        //// If you LogOut clear the players and sends
        //
        if (wcsstr(*dataSend, L"/salir") != NULL)
        {
            CharactersInRange.clear();
            CharactersInMap.clear();
            Packets.clear();
            selectedPlayerId = 0;
            selectedPlayerName = "";
            cheatStatus = false;
        }
		
		//
		//// Players In Console
		//
		if (wcsstr(*dataSend, L"/playic") != NULL)
		{
			if (!playerCStatus)
			{
				playerCStatus = true;
				SendToClient("||PlayerIC> Disabled!~255~3~3~1~0");
			}
			else if (playerCStatus)
			{
				playerCStatus = false;
				SendToClient("||PlayerIC> Enabled!~255~3~3~1~0");
			}
		}

		if (wcsstr(*dataSend, L"/name") != NULL)
		{
			AllocConsole();
			SetConsoleTitle(_T("AOR"));
			freopen_s(&fpstdin, "CONIN$", "r", stdin);
			freopen_s(&fpstdout, "CONOUT$", "w", stdout);
			freopen_s(&fpstderr, "CONOUT$", "w", stderr);
			{
				cout << "Ingrese su nick: " << endl;
				cin >> playerName;
				system("CLS");
				cout << "Nick Guardado como " << playerName << "." << endl;
				Sleep(1000);
				system("CLS");
				cout << "Puedes cerrar esta ventana." << endl;
				FreeConsole();
			}
		}


        //
        //// Reading Process
        //
        if (wcsstr(*dataSend, L"PRC") != NULL)
        {
            *dataSend = ConvertStringToBSTR("PRC @ Inicio:65672 @ Furius AO V 11.0.5.:1704776 @ FúriusAO:2032840 @ Skype™ - Matux:1573518 @ Program Manager:131206");
        }

        if (wcsstr(*dataSend, L"PRR") != NULL)
        {
            *dataSend = ConvertStringToBSTR("PRRC:\\FuriusAO\\FuriusAO.exe%C:\\Program Files(x86)\\Google\\Chrome\\Application\\chrome.exe%C:\\Program Files(x86)\\Skype\\Phone\\Skype.exe%C:\\Program Files(x86)\\Skype\\Phone\\Skype.exe%C:\\Program Files(x86)\\Google\\Chrome\\Application\\chrome.exe%C:\\Program Files(x86)\\Google\\Chrome\\Application\\chrome.exe%C:\\Program Files(x86)\\Google\\Chrome\\Application\\chrome.exe%C:\\Progm Files(x86)\\Google\\Chrome\\Application\\chrome.exe%C:\\Program Files(x86)\\Google\\Chrome\\Application\\chrome.exe%C:\\Program Files(x86)\\Google\\Chrome\\Application\\chrome.exe%C:\\Program Files(x86)\\Google\\Chrome\\Application\\chrome.exe%C:\\Program Files(x86)\\Google\\Chrome\\Application\\chrome.exe%C:\\Program Files(x86)\\Google\\Chrome\\Application\\chrome.exe%C:\\Program Files(x86)\\Google\\Chrome\\Application\\chrome.exe%C:\\Program Files(x86)\\Google\\Chrome\\Application\\chrome.exe%C:\\Program Files(x86)\\Google\\Chrome\\Application\\chrome.exe%C:\\Program Files(x86)\\Google\\Chrome\\Application\\chrome.exe%C:\\Program Files(x86)\\Google\\Chrome\\Application\\chrome.exe%C:\\Program Files(x86)\\Google\\Chrome\\Application\\chrome.exe%C:\\Program Files(x86)\\Google\\Chrome\\Application\\chrome.exe%C:\\Program Files(x86)\\Google\\Chrome\\Application\\chrome.exe%C:\\Program Files(x86)\\Google\\Chrome\\Application\\chrome.exe%C:\\Program Files(x86)\\Google\\Chrome\\Application\\chrome.exe%C:\\Program Files(x86)\\Google\\Chrome\\Application\\chrome.exe%C:\\Program Files(x86)\\Google\\Chrome\\Application\\chrome.exe%C:\\Program Files(x86)\\Google\\Chrome\\Application\\chrome.exe%C:\\Program Files(x86)\\Google\\Chrome\\Application\\chrome.exe%C:\\Program Files(x86)\\Google\\Chrome\\Application\\chrome.exe%C:\\Program Files(x86)\\Google\\Chrome\\Application\\chrome.exe%C:\\Program Files(x86)\\Google\\Chrome\\Application\\chrome.exe%C:\\Program Files(x86)\\Google\\Chrome\\Application\\chrome.exe%C:\\Program Files(x86)\\Google\\Chrome\\Application\\chrome.exe%C:\\Program Files(x86)\\Common Files\\Java\\Java Update\\jusched.exe%C:\\Program Files(x86)\\Common Files\\Java\\Java Update\\jusched.exe%C:\\Program Files(x86)\\Common Files\\Java\\Java Update\\jusched.exe%C:\\Program Files(x86)\\360\\Total Security\\safemon\\QHSafeTray.exe%C:\\Program Files(x86)\\AVG Web TuneUp\\vprot.exe%C:\\Program Files(x86)\\Intel\\Intel(R) USB 3.0 eXtensible Host Controller Driver\\Application\\iusb3mon.exe%C:\\Program Files(x86)\\Hotkey\\HkeyTray.exe%C:\\Program Files(x86)\\Hotkey\\HkeyTray.exe%C:\\Program Files(x86)\\Google\\Chrome\\Application\\chrome.exe%C:\\Users\\Pferraggi\\AppData\\Local\\Microsoft\\BingSvc\\BingSvc.exe%C:\\Program Files(x86)\\ChiconyCam\\CECAPLF.exe%");
        }

        //
        //// Turns on/off speedhack
        //
        if (wcsstr(*dataSend, L"/speed") != NULL)
        {
            if (!statusSpeedHack)
            {             
                statusSpeedHack = true;             
            }
            else if (statusSpeedHack)
            {
                statusSpeedHack = false;
            }
        }

        //
        //// This GameConsole Commands are used for UnitTesting
        //
        if (wcsstr(*dataSend, L"/map") != NULL)
        {
            ShowMap();
        }

        if (wcsstr(*dataSend, L"/player") != NULL)
        {
            auto it = CharactersInRange.find(selectedPlayerId);
            if (it != CharactersInRange.end())
            {
                Player *player = it->second;
                BSTR message = ConvertStringToBSTR("SelectedPlayer = " + player->name + " posx:" + to_string(player->posX) + " posy:" + to_string(player->posY));
                OutputDebugStringW(message);
            }
        }

        //
        //// Returns control to the Original Function
        //
        if (sendOk)
        {
            PFunctionSend(dataSend);
        }
    }
    catch (int e)
    {
        OutputDebugStringW(ConvertStringToBSTR("ERROR-> Place: SendData()  Id: " + to_string(e)));
    }

    __asm POPFD;
    __asm POPAD;
}

int WINAPI MyLoop()
{
	__asm PUSHAD;
	__asm PUSHFD;

	//
	//// Toggle Keys
	//
	toggle_AutoPotas.mKey = 0x51; // "Q"
	toggle_VInvi.mKey = 0x62; // "Numpad2"
	toggle_Remo.mKey = 0x04; // "MButton"
	toggle_Switch.mKey = 0x02; // "RButton"
	toggle_Apoca.mKey = 0x07; // "?"
	toggle_Descarga.mKey = 0x07; // "?"
	toggle_Inmo.mKey = 0x43; // "C"

	try
	{
		//
		//// SpeedHack
		//
		if (statusSpeedHack)
		{
			SpeedHack();
		}

		//
		//// Here goes every event that needs a trigger from keyboard
		//
		DWORD newTick = GetTickCount();
		if (newTick - oldTTeclas > 100)
		{

			//
			//// TEST KEY
			//
			if ((GetKeyState(VK_NUMPAD9) & 0x100) != 0)
			{
				if (hideCheat)
					hideCheat = false;
				else if (!hideCheat)
				{
					hideCheat = true;
					EraseConsole();
				}
			}

			if ((GetKeyState(VK_NUMPAD8) & 0x100) != 0) 
			{
				    EraseConsole();
			}

			oldTTeclas = GetTickCount();
		}
			//
			//// If i hit RClick it change selected character for AutoAim
			//
			if (toggle_Switch)
			{
				if (cheatStatus)
				{
					SwitchPlayerAutoAim();
				}
			}

			//
			//// AutoAim Triggers
			//
			if (cheatStatus)
			{
				AutoAim();
			}

			//
			//// Remo trigger
			//
			if (toggle_Remo)
			{
					CastRemo(cheaterPosX, cheaterPosY);
			}

			//
			//// Turns On/Off features
			//
			if (toggle_AutoPotas)
			{
				if (!cheatStatus)
				{
					SendToClient("||Autopotas> Enabled!~255~3~3~1~0");
					handlePotas = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)AutoPotas, 0, 0, 0);
					cheatStatus = true;
				}
				else if (cheatStatus)
				{
					SendToClient("||AutoPotas> Disabled!~255~3~3~1~0");
					TerminateThread(handlePotas, 0);
					cheatStatus = false;
				}
			}

			//
			//// Turns On/Off VerInvis
			//
			if (toggle_VInvi)
			{
				if (!verInvisOn)
				{
					verInvisOn = true;
					HideInvisiblePlayers(false);
					SendToClient("||Invis> Enabled!~255~3~3~1~0");
				}
				else if (verInvisOn)
				{
					verInvisOn = false;
					HideInvisiblePlayers(true);
					SendToClient("||Invis> Disabled!~255~3~3~1~0");
				}
			}

			
		

		//
		//// Sends all the packets that are on hold
		//
		if (Packets.size() != 0)
		{
			for (int i = 0; i < Packets.size(); ++i)
			{
				SendToServer(Packets[i]);
			}
			Packets.clear();
		}

		//
		//// Returns control to Original Function
		//
		PFunctionLoop();
	}
	catch (int e)
	{
		OutputDebugStringW(ConvertStringToBSTR("ERROR-> Place: Loop()  Id: " + to_string(e)));
	}

	__asm POPFD;
	__asm POPAD;
}

//
//// Cheating Methods
//

VOID SendToClient(string message)
{
    try
    {
        //
        //// Create Packet
        //
        BSTR recvPacket = ConvertStringToBSTR(message);
        OutputDebugStringW(ConvertStringToBSTR("SendToClient: " + ConvertBSTRToString(recvPacket)));
        //
        //// Send Packet
        //
        PFunctionRecv(recvPacket);
        //
        //// Free Packet
        //
        SysFreeString(recvPacket);
    }
    catch (int e)
    {
        OutputDebugStringW(ConvertStringToBSTR("ERROR-> Place: SendToClient()  Id: " + to_string(e)));
    }
}

VOID SendToServer(string message)
{
    try
    {
        //
        //// Create Packet
        //
        BSTR convertedSend = ConvertStringToBSTR(message);
        OutputDebugStringW(ConvertStringToBSTR("SendToServer: " + ConvertBSTRToString(convertedSend)));
        //
        //// Send Packet
        //
        PFunctionSend(&convertedSend);
        //
        //// Free Packet
        //
        SysFreeString(convertedSend);
    }
    catch (int e)
    {
        OutputDebugStringW(ConvertStringToBSTR("ERROR-> Place: SendToServer()  Id: " + to_string(e)));
    }
}

VOID ShowMap()
{
    try
    {
        string send = "SelectedPlayer: " + to_string(selectedPlayerId) + "/" + selectedPlayerName;
        BSTR _send = ConvertStringToBSTR(send);
        OutputDebugStringW(_send);

        for (auto it = CharactersInMap.begin(); it != CharactersInMap.end(); it++)
        {
            Player *player = it->second;
            string playerSTR = "ID:" + to_string(player->id) + " Name:" + player->name + " PosX:" + to_string(player->posX) + " PosY:" + to_string(player->posY);
            _bstr_t prueba(playerSTR.c_str());
            OutputDebugStringW(prueba);
            SysFreeString(prueba);
        }
    }
    catch (int e)
    {
    }
}

VOID SwitchPlayerAutoAim()
{
    try
    {
        if (CharactersInRange.size() > 1)
        {
            auto it = CharactersInRange.find(selectedPlayerId);
            auto itEnd = CharactersInRange.rbegin();
            if (it->first == itEnd->first && it != CharactersInRange.end())
            {
                //
                //// First we return the original Selected Player to normal
                //
                Player *foundPlayer = it->second;
                ChangeTarget(true, foundPlayer);
                //
                //// We select new Player
                //
                selectedPlayerId = CharactersInRange.begin()->first;
                selectedPlayerName = CharactersInRange.begin()->second->name;
                //
                //// Now we change the new selected Player
                //
                foundPlayer = CharactersInRange.find(selectedPlayerId)->second;
                ChangeTarget(false, foundPlayer);
            }
            else if (it->first != itEnd->first && it != CharactersInRange.end())
            {
                //
                //// First we return the original Selected Player to normal
                //
                Player *foundPlayer = it->second;
                ChangeTarget(true, foundPlayer);
                auto nextIt = next(it, 1);
                //
                //// We select new Player
                //
                selectedPlayerId = nextIt->first;
                selectedPlayerName = nextIt->second->name;
                //
                //// Now we change the new selected Player
                //
                foundPlayer = nextIt->second;
                ChangeTarget(false, foundPlayer);
            }
        }
    }
    catch (int e)
    {
        OutputDebugStringW(ConvertStringToBSTR("ERROR-> Place: SwitchPlayerInAutoAim()  Id: " + to_string(e)));
    }
}

VOID ChangeTarget(bool original, Player *player)
{
    try
    {
        if (!hideCheat)
        {
            if (!original)
            {
                SendToClient("||AOR> SelectedPlayer = " + player->name + " posx:" + to_string(player->posX) + " posy:" + to_string(player->posY) + "~255~0~247~0~0");
            }
        }
    }
    catch (int e)
    {
        OutputDebugStringW(ConvertStringToBSTR("ERROR-> Place: ChangeTarget()  Id: " + to_string(e)));
    }
}

VOID AutoAim()
{
    try
    {
        //
        //// AutoAim Apoca
        //
        if (toggle_Apoca)
        {
            int slot = stoi(positionApoca);
            CastSpell(slot);
        }

        //
        //// AutoAim Descarga
        //
        if (toggle_Descarga)
        {
            int slot = stoi(positionDescarga);
            CastSpell(slot);
        }

        //
        //// AutoAim Inmo
        //
        if (toggle_Inmo)
        {
            int slot = stoi(positionInmo);
            CastSpell(slot);
        }
    }
    catch (int e)
    {
        OutputDebugStringW(ConvertStringToBSTR("ERROR-> Place: AutoAim()  Id: " + to_string(e)));
    }
}

VOID CastRemo(int posX, int posY)
{
    try
    {
        int slot = stoi(positionRemo);
        SendToServer("LH" + to_string(slot));
        SendToServer("UK1");
        string VLC = "WLC" + to_string(posX) + "," + to_string(posY - 1) + ",1";
        SendToServer(VLC);
    }
    catch (int e)
    {
        OutputDebugStringW(ConvertStringToBSTR("ERROR-> Place: CastRemo()  Id: " + to_string(e)));
    }
}

VOID AutoPotas()
{
    try
    {
		DWORD *hpMaxAddress = (DWORD *)(0x894738);
		DWORD *hpActAddress = (DWORD *)(0x89473C);
		DWORD *mpMaxAddress = (DWORD *)(0x894740);
		DWORD *mpActAddress = (DWORD *)(0x894744);
        int *HPMAX = (int *)hpMaxAddress;
        int *HPACT = (int *)hpActAddress;
        int *MPMAX = (int *)mpMaxAddress;
        int *MPACT = (int *)mpActAddress;
		
        while (true)
        {
            if (*HPACT != 0)
            {
                if (*HPACT != *HPMAX)
				{
                    string message = "USEUf?=";
                    Packets.push_back(message);
					Sleep(300);
                    message = "USA>O=:";
                    Packets.push_back(message);
                }
                else if (*MPACT != *MPMAX)
                {
                    string message = "USE=S<C";
                    Packets.push_back(message);
					Sleep(300);
                    message = "USA*@;:";
                    Packets.push_back(message);
                }
            }
            Sleep(300);
        }
    }
    catch (int e)
    {
        OutputDebugStringW(ConvertStringToBSTR("ERROR-> Place: AutoPotas()  Id: " + to_string(e)));
    }
}

VOID CastSpell(int slot)
{
    try
    {
        SendToServer("LH" + to_string(slot));
        SendToServer("UK1");
        auto it = CharactersInRange.find(selectedPlayerId);
        if (it != CharactersInRange.end())
        {
            Player *targetPlayer = it->second;
            string VLC = "WLC" + to_string(targetPlayer->posX) + "," + to_string(targetPlayer->posY) + ",1";
            SendToServer(VLC);
        }
        else
        {
            if (!hideCheat)
                SendToClient("||AOR> No player in range for AutoAim ~0~119~255~0~0");
        }
    }
    catch (int e)
    {
        OutputDebugStringW(ConvertStringToBSTR("ERROR-> Place: CastSpell()  Id: " + to_string(e)));
    }
}

VOID SpeedHack()
{
    if ((GetKeyState(VK_UP) & 0x100) != 0)
    {
        SendToServer("M1");
        SendToServer("RPU");
    }
    if ((GetKeyState(VK_DOWN) & 0x100) != 0)
    {
        SendToServer("M3");
        SendToServer("RPU");
    }
    if ((GetKeyState(VK_LEFT) & 0x100) != 0)
    {
        SendToServer("M4");
        SendToServer("RPU");
    }
    if ((GetKeyState(VK_RIGHT) & 0x100) != 0)
    {
        SendToServer("M2");
        SendToServer("RPU");
    }
}

VOID EraseConsole()
{
    for (int i = 0; i <= 5; ++i)
    {
        SendToClient("||       ~253~103~3~0~0");
    }
}

VOID AddPlayer(vector<string> split)
{
    try
    {
        Player *newPlayer = new Player();
        newPlayer->id = stoi(split[3]);
        newPlayer->posX = stoi(split[4]);
        newPlayer->posY = stoi(split[5]);
        newPlayer->name = split[11];
        newPlayer->faction = stoi(split[12]);
        newPlayer->isInvisible = (stoi(split[13]) == 1);
        auto it = CharactersInMap.find(newPlayer->id);
        if (it == CharactersInMap.end())
        {
            CharactersInMap.insert(std::pair<int, Player *>(newPlayer->id, newPlayer));
        }
    }
    catch (int e)
    {
        OutputDebugStringW(ConvertStringToBSTR("ERROR-> Place: AddPlayer()  Id: " + to_string(e)));
    }
}

VOID HideInvisiblePlayers(bool hide)
{
    try
    {
        for (auto const &it : CharactersInMap)
        {
            if (it.second->isInvisible)
            {
                int playerId = it.second->id;
                string name = it.second->name;
                int posX = it.second->posX;
                int posY = it.second->posY;
                int faction = it.second->faction;
                string packetInvi = "";
                string packetExit = "QDL" + to_string(playerId);

                if (hide)
                {
                    packetInvi = "RM,,," + to_string(playerId) + "," + to_string(posX) + "," + to_string(posY) + ",10,1,0,999,3," + name + "," + to_string(faction) + ",1,0";
                }
                else
                {
                    packetInvi = "RM,,," + to_string(playerId) + "," + to_string(posX) + "," + to_string(posY) + ",10,1,0,999,3," + name + " [INVI]" + "," + to_string(faction) + ",0,0";
                }

                SendToClient(packetExit);
                SendToClient(packetInvi);
            }
        }
    }
    catch (int e)
    {
        OutputDebugStringW(ConvertStringToBSTR("ERROR-> Place: HideInsiviblePlayers()  Id: " + to_string(e)));
    }
}

//
//// Auxiliar Methods
//

BOOL StartsWith(BSTR sValue, const WCHAR *pszSubValue)
{
    if (!sValue)
        return FALSE;
    return wcsncmp(sValue, pszSubValue, wcslen(pszSubValue)) == 0;
}

string ConvertWCSToMBS(const wchar_t *pstr, long wslen)
{
    int len = ::WideCharToMultiByte(CP_ACP, 0, pstr, wslen, NULL, 0, NULL, NULL);

    std::string dblstr(len, '\0');
    len = ::WideCharToMultiByte(CP_ACP, 0 /* no flags */,
                                pstr, wslen /* not necessary NULL-terminated */,
                                &dblstr[0], len,
                                NULL, NULL /* no default char */);

    return dblstr;
}

string ConvertBSTRToString(BSTR bstr)
{
    int wslen = ::SysStringLen(bstr);
    return ConvertWCSToMBS((wchar_t *)bstr, wslen);
}

BSTR ConvertStringToBSTR(const std::string &str)
{
    int wslen = ::MultiByteToWideChar(CP_ACP, 0 /* no flags */,
                                      str.data(), str.length(),
                                      NULL, 0);

    BSTR wsdata = ::SysAllocStringLen(NULL, wslen);
    ::MultiByteToWideChar(CP_ACP, 0 /* no flags */,
                          str.data(), str.length(),
                          wsdata, wslen);
    return wsdata;
}