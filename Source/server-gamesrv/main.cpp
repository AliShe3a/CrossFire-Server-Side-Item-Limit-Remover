#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <iostream>
#include <process.h>
#include "MinHook.h"
#pragma comment(lib, "MinHook.x86.lib")
#include <fstream>
#include <string>
#include "xor.h"
#include <thread>
#include <ctime>
#include <chrono>
#include <winhttp.h>
#include <wininet.h>
#pragma comment(lib, "winhttp.lib")
#pragma comment(lib, "wininet.lib")


HANDLE GameSrvHandle = NULL;
typedef BOOL(WINAPI* _CloseHandle)(HANDLE);
typedef BOOL(WINAPI* _CreateDirectoryA)(LPCSTR, LPSECURITY_ATTRIBUTES);
typedef void(__stdcall* _ExitProcess)(DWORD);
typedef DWORD(WINAPI* _GetModuleFileNameA)(HMODULE, LPSTR, DWORD);
typedef HANDLE(WINAPI* _GetCurrentProcess)(VOID);
typedef HANDLE(WINAPI* _OpenProcess)(DWORD, BOOL, DWORD);
typedef int (WINAPI* _MessageBox)(HWND, LPCSTR, LPCSTR, UINT);
typedef BOOL(WINAPI* _VirtualProtect)(LPVOID, SIZE_T, DWORD, PDWORD);
typedef LONG(WINAPI* _NtQueryInformationProcess)(HANDLE, ULONG, PVOID, ULONG, PULONG);
typedef unsigned long(__stdcall* _NtQuerySystemInformation)(unsigned long, void*, unsigned long, unsigned long*);
typedef void (WINAPI* EnterCriticalSection_t) (LPCRITICAL_SECTION lpCriticalSection);

DWORD GameSrvExe = 0x0;
DWORD GShell = 0x0;

#define ITEMS_COUNT 5120 * 2
#define ITEMS_SIZE 0xEA8

typedef char(__thiscall* initInGameItems_fn)(void* This);
initInGameItems_fn oinitInGameItems = nullptr;

typedef int(__stdcall* itemmem_t)(int a1, int a2, int a3, void(__thiscall* a4)(int));
itemmem_t original_itemmem = nullptr;

typedef int(__cdecl* initItemsArray_fn)(int a1);
initItemsArray_fn oItemsArray = nullptr;

typedef WPARAM(__thiscall* GameSrvStart_fn)(void* This, HINSTANCE hInstance, int nCmdShow, int a4, int a5);
GameSrvStart_fn oGameSrvStart = nullptr;

void Log(const std::string& message)
{
	//std::ofstream log("C:\\log.txt", std::ios::app);  // Absolute path to C:\log.txt
	//if (log.is_open())
	//{
	//	log << message << std::endl;
	//	log.close();
	//}
}

bool IsAddressValid(const void* ptr, size_t size = 4) {
	MEMORY_BASIC_INFORMATION mbi;
	if (VirtualQuery(ptr, &mbi, sizeof(mbi)) == 0)
		return false;

	bool isCommitted = mbi.State == MEM_COMMIT;
	bool hasAccess =
		(mbi.Protect & PAGE_EXECUTE_READ) ||
		(mbi.Protect & PAGE_EXECUTE_READWRITE) ||
		(mbi.Protect & PAGE_READONLY) ||
		(mbi.Protect & PAGE_READWRITE);

	bool isNotGuardOrNoAccess =
		!(mbi.Protect & PAGE_GUARD) &&
		!(mbi.Protect & PAGE_NOACCESS);

	bool inBounds = ((uintptr_t)ptr + size) <= ((uintptr_t)mbi.BaseAddress + mbi.RegionSize);

	return isCommitted && hasAccess && isNotGuardOrNoAccess && inBounds;
}


template<typename T> bool WPM(int Address, T Value)
{
	T* address = (T*)(Address);
	*address = Value;
	return true;
}

template<typename T> bool vWPM(int address, const T& value, size_t immSize = 4)
{
	DWORD oldProtect;
	if (!VirtualProtect((LPVOID)(address), immSize, PAGE_EXECUTE_READWRITE, &oldProtect))
		return false;

	bool result = WPM<T>(address, value);

	VirtualProtect((LPVOID)(address), immSize, oldProtect, &oldProtect);
	FlushInstructionCache(GetCurrentProcess(), (LPCVOID)(address), immSize);
	return result;
}

bool ApplyItemLimitFix_GameSrv() {

	// --- ITEM LIMIT BYPASS GAME SERVER --- //

	if (!GShell)
		return false;

	DWORD ITEM_LIMIT_GAME_SERVER[] = {
		GShell + 0xC0FE ,
		GShell + 0xC16E ,
		GShell + 0xE40B ,
		GShell + 0xF2B8 ,
		GShell + 0x103E8 ,
		GShell + 0x12271 ,
		GShell + 0x122E3 ,
		GShell + 0x1239C ,
		GShell + 0x12403 ,
		GShell + 0x12479 ,
		GShell + 0x125A9 ,
		GShell + 0x12658 ,
		GShell + 0x1287A ,
		GShell + 0x12979 ,
		GShell + 0x129DB ,
		GShell + 0x12A49 ,
		GShell + 0x407BE ,
		GShell + 0x4AC43 ,
        GShell + 0x4FB6D ,
		GShell + 0x8B0AD ,
		GShell + 0x9D462 ,
		GShell + 0x9D532 ,
		GShell + 0xCAFF3 ,
		GShell + 0xCF626 ,
		GShell + 0x134828 ,
		GShell + 0x139AD2 ,
		GShell + 0x18667E ,
		GShell + 0x1983C5 ,
		GShell + 0x199742 ,
		GShell + 0x1D5233 ,
		GShell + 0x1D55BE ,
		GShell + 0x1DC7D0 ,
		GShell + 0x2323F0 ,
		GShell + 0x254E8C ,
		GShell + 0x2E69FE ,
		GShell + 0x353AB8 ,
		GShell + 0x35428D ,
		GShell + 0x358A35 ,

        GShell + 0x12764 + 0x3,
		GShell + 0x12AC2 ,
		GShell + 0xCF619 + 0x6 ,
		GShell + 0xD1286 ,

	};

	DWORD ITEM_LIMIT_GAME_SERVER_MINUS_ONE[] = {

		GShell + 0x45AC8 ,
		GShell + 0x100D76 ,
		GShell + 0x21AEB2 ,
		GShell + 0x34E823 ,
	};

	for (int i = 0; i < sizeof(ITEM_LIMIT_GAME_SERVER_MINUS_ONE) / sizeof(ITEM_LIMIT_GAME_SERVER_MINUS_ONE[0]); ++i) {
		std::string msg = std::string("minus index : ") + std::to_string(i);
		Log(msg);

		int retries = 50;
		while (retries-- > 0 && !IsAddressValid((void*)ITEM_LIMIT_GAME_SERVER_MINUS_ONE[i])) {
			Sleep(100);
		}

		vWPM<int>(ITEM_LIMIT_GAME_SERVER_MINUS_ONE[i], (ITEMS_COUNT - 1));
	}

	Sleep(1000);

	for (int i = 0; i < sizeof(ITEM_LIMIT_GAME_SERVER) / sizeof(ITEM_LIMIT_GAME_SERVER[0]); ++i) {
		std::string msg = std::string("normal index : ") + std::to_string(i);
		Log(msg);

		int retries = 50;
		while (retries-- > 0 && !IsAddressValid((void*)ITEM_LIMIT_GAME_SERVER[i])) {
			Sleep(100);
		}


		vWPM<int>(ITEM_LIMIT_GAME_SERVER[i], (ITEMS_COUNT));
	}

	return true;
}

char __fastcall initInGameItems_hk(void* This)
{
	Log("[+] initInGameItems was called!\n");
	//ApplyItemLimitFix_GameSrv();

	return oinitInGameItems(This);
}

int __stdcall hooked_itemmem(int a1, int a2, int a3, void(__thiscall* a4)(int))
{

	Log("[-] hooked_itemmem was called!\n");

	const int oldCount = 5120;
	const int newCount = ITEMS_COUNT;

	if (a3 == oldCount) {
		a3 = newCount;
	}

	std::string txt = "[+] Item Memory Created With : " + std::to_string(a3);
	Log(txt);

	return original_itemmem(a1, a2, a3, a4);
}

int __cdecl ItemsArray_hk(int a1)
{
	Log("[-] ItemsArray_hk was called!\n");

	std::string txt = "[+] ItemsArray_hk called With : " + std::to_string(a1);
	Log(txt);


	if (a1 == 17817600)
	{
		a1 = (ITEMS_COUNT * 3480);
	}

	return oItemsArray(a1);
}

WPARAM __fastcall GameSrvStart_hk(void* This, void* /*unused*/, HINSTANCE hInstance, int nCmdShow, int a4, int a5)
{
	Log("[+] Console Patch Started!");


	vWPM<int>((GShell + (0xCAFCA + 0x1)), (ITEMS_COUNT * 3480));

	Sleep(100);

	ApplyItemLimitFix_GameSrv();

	//nCmdShow = 1; // Force show mode
	return oGameSrvStart(This, hInstance, nCmdShow, a4, a5);
}

DWORD WINAPI InitHook(LPVOID)
{
	// Wait until function address is accessible
	/*while (*(BYTE*)(GShell + 0xCF420) == 0x00 || *(BYTE*)(GShell + 0xCF420) == 0xCC)
		Sleep(10);*/

	if (MH_Initialize() != MH_OK)
	{
		Log("[-] MinHook was not initialized!\n");
		return 1;
	}

	if (

		MH_CreateHook(
			(LPVOID)(GShell + 0xCABC0),
			&GameSrvStart_hk,
			reinterpret_cast<LPVOID*>(&oGameSrvStart)) != MH_OK /*&&
		
		MH_CreateHook(
		(LPVOID)(GShell + 0xCF420),
		&initInGameItems_hk,
		reinterpret_cast<LPVOID*>(&oinitInGameItems)) != MH_OK &&

		MH_CreateHook(
			(LPVOID)(GShell + 0x3AF0),
			&hooked_itemmem,
			reinterpret_cast<LPVOID*>(&original_itemmem)) != MH_OK &&

		MH_CreateHook(
			(LPVOID)(GShell + 0x36EC88),
			&ItemsArray_hk,
			reinterpret_cast<LPVOID*>(&oItemsArray)) != MH_OK*/
		
		
		)
	{
		Log("[-] MinHook was not created!\n");
		return 1;
	}



	if (MH_EnableHook((LPVOID)(GShell + 0xCABC0)) != MH_OK /*&& MH_EnableHook((LPVOID)(GShell + 0xCF420)) != MH_OK && MH_EnableHook((LPVOID)(GShell + 0x3AF0)) != MH_OK && MH_EnableHook((LPVOID)(GShell + 0x36EC88)) != MH_OK*/)
	{
		Log("[-] MinHook was not enabled!\n");
		return 1;
	}

	return 0;
}

extern "C" __declspec(dllexport) void HandleGameSrv(LPVOID)
{

	GameSrvExe = (DWORD)GetModuleHandleA("cf_gamesrv.exe");

	if (GameSrvExe)
	{

		Log("[+] Patch Started!");

		GShell = GameSrvExe;

		InitHook(nullptr);
		//ApplyItemLimitFix_GameSrv();

	}
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{

	if (ul_reason_for_call == DLL_PROCESS_ATTACH)
	{
		DisableThreadLibraryCalls(hModule);
		GameSrvHandle = (HANDLE)_beginthread(HandleGameSrv, 0, 0);



	}

	if (ul_reason_for_call == DLL_PROCESS_DETACH)
	{

		if (GameSrvHandle)
		{
			_CloseHandle ACCloseHandle = (_CloseHandle)GetProcAddress(GetModuleHandleA("kernel32.dll"), "CloseHandle");
			if (ACCloseHandle)
				ACCloseHandle(GameSrvHandle);
		}

	}

	return TRUE;



}