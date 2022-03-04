#include <windows.h>
#include <iostream>

#include "clrhost.h"

const CHAR* MODLOADER_PATH = "ModLoader";
const CHAR* APPDOMAIN_ID = "ExanimaMods";

const CHAR* ENTRYPOINT_ASSEMBLY = "ModLoader";
const CHAR* ENTRYPOINT_CLASS = "ModLoader.ModLoader";
const CHAR* ENTRYPOINT_SUBROUTINE = "LoadEntry";

DWORD WINAPI ModLoader(HMODULE hModule) {
	AllocConsole();
	auto open = freopen("CONOUT$", "w", stdout);

	std::cout << "Loading the .NET 5 Runtime..." << endl;
	clrhost clrhost;
	clrhost.LoadManagedAssembly(MODLOADER_PATH,
		APPDOMAIN_ID, ENTRYPOINT_ASSEMBLY,
		ENTRYPOINT_CLASS, ENTRYPOINT_SUBROUTINE);

	return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved) {
	switch (ul_reason_for_call) {
	case DLL_PROCESS_ATTACH: {
		auto handle = CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)ModLoader, hModule, 0, nullptr);
		if (handle != 0) {
			CloseHandle(handle);
		}
	}
	}
	return TRUE;
}