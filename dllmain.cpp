// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include "AoDll.h"


BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID lpReserved)
{
	switch (reason)
	{
		case DLL_PROCESS_ATTACH:
			CreateThread(0, 0, (LPTHREAD_START_ROUTINE)Hooks, 0, 0, 0);
		case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}
