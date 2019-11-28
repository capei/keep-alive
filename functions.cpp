#include "stdafx.h"


#define INI_Name "./KeepAlive.ini"

/*
	SAMPLE INI
	------------------------------------



*/

// STRING
char* iniReadStr(char* Section, char* Key, const char* DefaultString){

	char* Result = new char[255];
	//char Result[512];
	memset(Result, 0x00, 255);
	GetPrivateProfileStringA(Section, Key, DefaultString, Result, 255, INI_Name);
	return Result;
}

void iniWriteStr(char* Section, char* Key, const char* ValueString){

	WritePrivateProfileStringA(Section, Key, ValueString, INI_Name);
}

// INT
int iniReadInt(char* Section, char* Key, int DefaultInt){

	int Result = GetPrivateProfileIntA(Section, Key, DefaultInt, INI_Name);
	return Result;
}

void iniWriteInt(char* Section, char* Key, int ValueInt, bool DecHex){

	char tmpValue[255];
	if (DecHex == 0) { sprintf_s(tmpValue, 255, "%d", ValueInt); } // %d = DEC
	if (DecHex == 1) { sprintf_s(tmpValue, 255, "%x", ValueInt); } // %x = HEX
	WritePrivateProfileStringA(Section, Key, tmpValue, INI_Name);
}

// FLOAT
float iniReadFloat(char* szSection, char* szKey, float DefaultFloat){

	char szResult[255];
	char szDefault[255];
	double dResult;
	float fResult;
	sprintf_s(szDefault, "%f", DefaultFloat);
	GetPrivateProfileStringA(szSection, szKey, szDefault, szResult, 255, INI_Name);
	dResult = atof(szResult);
	fResult = (float)dResult;
	return fResult;
}

void iniWriteFloat(char* szSection, char* szKey, float ValueFloat, int decimal){

	char szValue[255];
	if (decimal <= 1) sprintf_s(szValue, "%.1f", ValueFloat);
	else if (decimal == 2) sprintf_s(szValue, "%.2f", ValueFloat);
	else if (decimal == 3) sprintf_s(szValue, "%.3f", ValueFloat);
	else if (decimal == 4) sprintf_s(szValue, "%.4f", ValueFloat);
	else if (decimal == 5) sprintf_s(szValue, "%.5f", ValueFloat);
	else if (decimal == 6) sprintf_s(szValue, "%.6f", ValueFloat);
	else if (decimal >= 7) sprintf_s(szValue, "%.7f", ValueFloat);
	WritePrivateProfileStringA(szSection, szKey, szValue, INI_Name);
}

///	<summary>
///	Searches for the given process name.
///	</summary>
///	<returns>
///	Returns 1 if process is running, 606 if the system version cannot be determined, 607 if unsupported OS,
///	605 if the process list could not be enumerated or if some modules (i.e.: PSAPI.DLL, etc) failed to load.
///	</returns>
int isProcessRunning(const char *szToFind){

	BOOL bResult, bResultm;
	DWORD aiPID[1000], iCb = 1000, iNumProc, iV2000 = 0;
	DWORD iCbneeded, i;
	char szName[MAX_PATH], szToFindUpper[MAX_PATH];
	HANDLE hProc, hSnapShot, hSnapShotm;
	OSVERSIONINFO osvi;
	HINSTANCE hInstLib;
	int iLen, iLenP, indx;
	HMODULE hMod;
	PROCESSENTRY32 procentry;
	MODULEENTRY32 modentry;

	// PSAPI Function Pointers.
	BOOL(WINAPI *lpfEnumProcesses)(DWORD *, DWORD cb, DWORD *);
	BOOL(WINAPI *lpfEnumProcessModules)(HANDLE, HMODULE *,
		DWORD, LPDWORD);
	DWORD(WINAPI *lpfGetModuleBaseName)(HANDLE, HMODULE,
		LPTSTR, DWORD);

	// ToolHelp Function Pointers.
	HANDLE(WINAPI *lpfCreateToolhelp32Snapshot)(DWORD, DWORD);
	BOOL(WINAPI *lpfProcess32First)(HANDLE, LPPROCESSENTRY32);
	BOOL(WINAPI *lpfProcess32Next)(HANDLE, LPPROCESSENTRY32);
	BOOL(WINAPI *lpfModule32First)(HANDLE, LPMODULEENTRY32);
	BOOL(WINAPI *lpfModule32Next)(HANDLE, LPMODULEENTRY32);

	// Transfer Process name into "szToFindUpper" and
	// convert it to upper case
	iLenP = strlen(szToFind);
	if (iLenP<1 || iLenP>MAX_PATH) return 632;
	for (indx = 0; indx<iLenP; indx++)
		szToFindUpper[indx] = toupper(szToFind[indx]);
	szToFindUpper[iLenP] = 0;

	// First check what version of Windows we're in
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	bResult = GetVersionEx(&osvi);
	if (!bResult)     // Unable to identify system version
		return 606;

	// At Present we only support Win/NT/2000 or Win/9x/ME
	if ((osvi.dwPlatformId != VER_PLATFORM_WIN32_NT) &&
		(osvi.dwPlatformId != VER_PLATFORM_WIN32_WINDOWS))
		return 607;

	if (osvi.dwPlatformId == VER_PLATFORM_WIN32_NT)
	{
		// Win/NT or 2000 or XP

		// Load library and get the procedures explicitly. We do
		// this so that we don't have to worry about modules using
		// this code failing to load under Windows 95, because
		// it can't resolve references to the PSAPI.DLL.
		hInstLib = LoadLibraryA("PSAPI.DLL");
		if (hInstLib == NULL)
			return 605;

		// Get procedure addresses.
		lpfEnumProcesses = (BOOL(WINAPI *)(DWORD *, DWORD, DWORD*))
			GetProcAddress(hInstLib, "EnumProcesses");
		lpfEnumProcessModules = (BOOL(WINAPI *)(HANDLE, HMODULE *,
			DWORD, LPDWORD)) GetProcAddress(hInstLib,
				"EnumProcessModules");
		lpfGetModuleBaseName = (DWORD(WINAPI *)(HANDLE, HMODULE,
			LPTSTR, DWORD)) GetProcAddress(hInstLib,
				"GetModuleBaseNameA");

		if (lpfEnumProcesses == NULL ||
			lpfEnumProcessModules == NULL ||
			lpfGetModuleBaseName == NULL)
		{
			FreeLibrary(hInstLib);
			return 605;
		}

		bResult = lpfEnumProcesses(aiPID, iCb, &iCbneeded);
		if (!bResult)
		{
			// Unable to get process list, EnumProcesses failed
			FreeLibrary(hInstLib);
			return 605;
		}

		// How many processes are there?
		iNumProc = iCbneeded / sizeof(DWORD);

		// Get and match the name of each process
		for (i = 0; i<iNumProc; i++)
		{
			// Get the (module) name for this process

			strcpy(szName, "Unknown");
			// First, get a handle to the process
			hProc = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE,
				aiPID[i]);
			// Now, get the process name
			if (hProc)
			{
				if (lpfEnumProcessModules(hProc, &hMod, sizeof(hMod), &iCbneeded))
				{
					iLen = lpfGetModuleBaseName(hProc, hMod, (LPTSTR)szName, MAX_PATH);
				}
			}
			CloseHandle(hProc);
			// Match regardless of lower or upper case
			if (strcmp(_strupr(szName), szToFindUpper) == 0)
			{
				// Process found
				FreeLibrary(hInstLib);
				return 1;
			}
		}
	}

	if (osvi.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS)
	{
		// Win/95 or 98 or ME

		hInstLib = LoadLibraryA("Kernel32.DLL");
		if (hInstLib == NULL)
			return FALSE;

		// Get procedure addresses.
		// We are linking to these functions of Kernel32
		// explicitly, because otherwise a module using
		// this code would fail to load under Windows NT,
		// which does not have the Toolhelp32
		// functions in the Kernel 32.
		lpfCreateToolhelp32Snapshot =
			(HANDLE(WINAPI *)(DWORD, DWORD))
			GetProcAddress(hInstLib,
				"CreateToolhelp32Snapshot");
		lpfProcess32First =
			(BOOL(WINAPI *)(HANDLE, LPPROCESSENTRY32))
			GetProcAddress(hInstLib, "Process32First");
		lpfProcess32Next =
			(BOOL(WINAPI *)(HANDLE, LPPROCESSENTRY32))
			GetProcAddress(hInstLib, "Process32Next");
		lpfModule32First =
			(BOOL(WINAPI *)(HANDLE, LPMODULEENTRY32))
			GetProcAddress(hInstLib, "Module32First");
		lpfModule32Next =
			(BOOL(WINAPI *)(HANDLE, LPMODULEENTRY32))
			GetProcAddress(hInstLib, "Module32Next");
		if (lpfProcess32Next == NULL ||
			lpfProcess32First == NULL ||
			lpfModule32Next == NULL ||
			lpfModule32First == NULL ||
			lpfCreateToolhelp32Snapshot == NULL)
		{
			FreeLibrary(hInstLib);
			return 605;
		}

		// The Process32.. and Module32.. routines return names in all uppercase
		// Get a handle to a Toolhelp snapshot of all the systems processes.

		hSnapShot = lpfCreateToolhelp32Snapshot(
			TH32CS_SNAPPROCESS, 0);
		if (hSnapShot == INVALID_HANDLE_VALUE)
		{
			FreeLibrary(hInstLib);
			return 605;
		}

		// Get the first process' information.
		procentry.dwSize = sizeof(PROCESSENTRY32);
		bResult = lpfProcess32First(hSnapShot, &procentry);

		// While there are processes, keep looping and checking.
		while (bResult)
		{
			// Get a handle to a Toolhelp snapshot of this process.
			hSnapShotm = lpfCreateToolhelp32Snapshot(
				TH32CS_SNAPMODULE, procentry.th32ProcessID);
			if (hSnapShotm == INVALID_HANDLE_VALUE)
			{
				CloseHandle(hSnapShot);
				FreeLibrary(hInstLib);
				return 605;
			}
			// Get the module list for this process
			modentry.dwSize = sizeof(MODULEENTRY32);
			bResultm = lpfModule32First(hSnapShotm, &modentry);

			// While there are modules, keep looping and checking
			while (bResultm)
			{
				if (strcmp((const char*)modentry.szModule, szToFindUpper) == 0)
				{
					// Process found
					CloseHandle(hSnapShotm);
					CloseHandle(hSnapShot);
					FreeLibrary(hInstLib);
					return 1;
				}
				else
				{  // Look for next modules for this process
					modentry.dwSize = sizeof(MODULEENTRY32);
					bResultm = lpfModule32Next(hSnapShotm, &modentry);
				}
			}

			//Keep looking
			CloseHandle(hSnapShotm);
			procentry.dwSize = sizeof(PROCESSENTRY32);
			bResult = lpfProcess32Next(hSnapShot, &procentry);
		}
		CloseHandle(hSnapShot);
	}
	FreeLibrary(hInstLib);
	return 0;

}



