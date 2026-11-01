#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <iostream>
#include <process.h>
#include <winhttp.h>
#include <wininet.h>
#include "xor.h"
#pragma comment(lib, "winhttp.lib")
#pragma comment(lib, "wininet.lib")

HANDLE objHandle = NULL;
HANDLE objHandle2 = NULL;
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

void OpenConsole()
{
	AllocConsole();
	FILE* pConsole;
	freopen_s(&pConsole, "CONIN$", "r", stdin);
	freopen_s(&pConsole, "CONOUT$", "w", stdout);
	freopen_s(&pConsole, "CONOUT$", "w", stderr);
	printf("DLL HAS BEEN LOADED SUCCESSFULLY\n");
}

DWORD ObjDll = 0x0;
DWORD hObject = 0x0;

#define ITEMS_COUNT 5120 * 2
#define ITEMS_SIZE 0xEA8
#define WEAPONS_COUNT 0x1000 * 2
#define WEAPONS_SIZE 0x479C

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

bool ApplyItemLimitFix_Obj() {
	
	if (!hObject)
		return false;

	// --- ITEM LIMIT ARRAY --- //

	DWORD ITEM_LIMIT_BOT_MATCH_ARRAY[] = {
	hObject + 0x6D0C5 + 0x1,
	hObject + 0x6B1D3 + 0x1,
	hObject + 0x6B1E6 + 0x1,
	hObject + 0x6CF2D + 0x4,
	hObject + 0x6D8C0 + 0x4,
	hObject + 0xC52EF + 0x2,
	};

	// --- ITEM LIMIT INDEX --- //

	DWORD ITEM_LIMIT_BOT_MATCH[] = {
	hObject + 0x6D0E8 + 0x1,
	hObject + 0xC5372 + 0x2,
	hObject + 0xF4F2D + 0x1,
	hObject + 0xF51BA + 0x1,
	hObject + 0x571EDB + 0x1,
	hObject + 0x571FB3 + 0x1,

	hObject + 0x56838B + 0x2,
	hObject + 0x568561 + 0x3,
	hObject + 0x568579 + 0x3,
	hObject + 0x568586 + 0x3,
	hObject + 0x5685E7 + 0x3,
	hObject + 0x568657 + 0x3,
	};

	// --- ITEM LIMIT INDEX MINUS ONE --- //

	DWORD ITEM_LIMIT_BOT_MATCH_MINUS_ONE[] = {
	hObject + 0x6D2D8 + 0x1,
	hObject + 0x571E74 + 0x2,

	};


	// --- WEAPON LIMIT INDEX --- //

	DWORD WEAPON_BOT_MATCH_LIMIT[] = {
		hObject + 0x1d6c7e,
		hObject + 0x1d61fd,
		hObject + 0x1d4bb9,
		hObject + 0x1d24c3,
		hObject + 0x1c34b2,
		hObject + 0x8c756,
		hObject + 0x7a421,
	};


	// --- WEAPON LIMIT INDEX MINUS ONE --- //

	DWORD WEAPON_LIMIT_BOT_MATCH_MINUS_ONE[] = {
		hObject + 0x7a45e,
		hObject + 0x7a49b,
		hObject + 0x7a52d,
		hObject + 0x832a3,
		hObject + 0x84001,
		hObject + 0x1c1d2d,
		hObject + 0x1c5307,
	};

	// --- PATCHING ITEM.CFT --- //

	for (int i = 0; i < sizeof(ITEM_LIMIT_BOT_MATCH_ARRAY) / sizeof(ITEM_LIMIT_BOT_MATCH_ARRAY[0]); ++i) {
		vWPM<int>(ITEM_LIMIT_BOT_MATCH_ARRAY[i], (ITEMS_COUNT * ITEMS_SIZE));
	}

	for (int i = 0; i < sizeof(ITEM_LIMIT_BOT_MATCH) / sizeof(ITEM_LIMIT_BOT_MATCH[0]); ++i) {
		vWPM<int>(ITEM_LIMIT_BOT_MATCH[i], (ITEMS_COUNT));
	}

	for (int i = 0; i < sizeof(ITEM_LIMIT_BOT_MATCH_MINUS_ONE) / sizeof(ITEM_LIMIT_BOT_MATCH_MINUS_ONE[0]); ++i) {
		vWPM<int>(ITEM_LIMIT_BOT_MATCH_MINUS_ONE[i], (ITEMS_COUNT - 1));
	}

	vWPM<short>((hObject + (0x6B137 + 0x3)), (ITEMS_COUNT - 1), 2);


	// --- PATCHING BF005.LTC --- //

	vWPM<int>((hObject + 0x7a3b2), (WEAPONS_COUNT * WEAPONS_SIZE));



	vWPM<short>((hObject + 0x1f1397), (WEAPONS_COUNT));

	for (int i = 0; i < sizeof(WEAPON_BOT_MATCH_LIMIT) / sizeof(WEAPON_BOT_MATCH_LIMIT[0]); ++i) {
		vWPM<int>(WEAPON_BOT_MATCH_LIMIT[i], WEAPONS_COUNT);
	}


	for (int i = 0; i < sizeof(WEAPON_LIMIT_BOT_MATCH_MINUS_ONE) / sizeof(WEAPON_LIMIT_BOT_MATCH_MINUS_ONE[0]); ++i) {
		vWPM<short>(WEAPON_LIMIT_BOT_MATCH_MINUS_ONE[i], (WEAPONS_COUNT - 1));
	}

	return true;
}

DWORD WINAPI WaitForObjectDll(LPVOID lpParam)
{
	HMODULE hMod = NULL;

	// Loop until GetModuleHandle returns a valid handle (not NULL)
	while (true)
	{
		hMod = GetModuleHandleA("Object.dll");

		if (hMod != NULL)
		{
			// DLL Found! Break the loop.
			break;
		}

		// Sleep for 100ms to prevent high CPU usage while waiting
		Sleep(10);
	}

	// 2. Setup variables
	ObjDll = (DWORD)hMod;
	hObject = ObjDll;

	// 3. Apply the fix exactly one time
	ApplyItemLimitFix_Obj();


	return 0;
}

extern "C" __declspec(dllexport) void HandleObject(LPVOID)
{
	// Create a thread to handle the waiting logic. 
	// This allows the main program to continue without freezing.
	HANDLE hThread = CreateThread(
		NULL,                   // default security attributes
		0,                      // default stack size
		(LPTHREAD_START_ROUTINE)WaitForObjectDll, // thread function
		nullptr,                // parameter to thread function
		0,                      // default creation flags
		NULL                    // optionally receive thread ID
	);

	

	if (hThread)
	{
		CloseHandle(hThread); 
	}

}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{

	if (ul_reason_for_call == DLL_PROCESS_ATTACH)
	{
		DisableThreadLibraryCalls(hModule);
		objHandle = (HANDLE)_beginthread(HandleObject, 0, 0);




	}

	if (ul_reason_for_call == DLL_PROCESS_DETACH)
	{

		if (objHandle)
		{
			_CloseHandle ACCloseHandle = (_CloseHandle)GetProcAddress(GetModuleHandleA("kernel32.dll"), "CloseHandle");
			if (ACCloseHandle)
				ACCloseHandle(objHandle);
		}

	}

	return TRUE;



}