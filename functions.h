#pragma once

// STRING
extern char* iniReadStr(char* Section, char* Key, const char* DefaultString);

extern void iniWriteStr(char* Section, char* Key, const char* ValueString);

// INT
extern int iniReadInt(char* Section, char* Key, int DefaultInt);

extern void iniWriteInt(char* Section, char* Key, int ValueInt, bool DecHex);

// FLOAT
extern float iniReadFloat(char* szSection, char* szKey, float DefaultFloat);

extern void iniWriteFloat(char* szSection, char* szKey, float ValueFloat, int decimal);

///	<summary>
///	Searches for the given process name.
///	</summary>
///	<returns>
///	Returns 1 if process is running, 606 if the system version cannot be determined, 607 if unsupported OS,
///	605 if the process list could not be enumerated or if some modules (i.e.: PSAPI.DLL, etc) failed to load.
///	</returns>
extern int isProcessRunning(const char *szToFind);
