// keepAlive.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

std::string sectionKeepAlive = "KEEPALIVE";
std::string keyProcessName = "ProcessName";
std::string keyProcessPath = "ProcessPath";
std::string keyProcessArgs = "ProcessArgs";

int main(){

	char buff[4096], cmd[4096];
	bool args = 0;
	std::string processName("");
	std::string processPath("");
	std::string processArgs("");
	std::string processFullPath("");

	processName = (std::string)iniReadStr((char*)sectionKeepAlive.c_str(), (char*)keyProcessName.c_str(), "");
	processPath = (std::string)iniReadStr((char*)sectionKeepAlive.c_str(), (char*)keyProcessPath.c_str(), "");
	processArgs = (std::string)iniReadStr((char*)sectionKeepAlive.c_str(), (char*)keyProcessArgs.c_str(), "");

	if (
		(!processName.empty() && processName.length() > 1)
		// && (!processPath.empty() && processPath.length() > 1)
		){

		processFullPath = processPath + "\\" + processName;
		processArgs.empty() ? args = 0 : args = 1;

		sprintf_s(buff, "[Info]: ProcessName = %s\n", processName.c_str()); printf(buff);
		sprintf_s(buff, "[Info]: ProcessPath = %s\n", processPath.c_str()); printf(buff);
		sprintf_s(buff, "[Info]: ProcessArgs = %s\n", processArgs.c_str()); printf(buff);

		sprintf_s(buff, "[Info]: Success. KeepAlive is now running for '%s'\n\n", processFullPath.c_str());
		printf(buff);

		while (1){

			if (!isProcessRunning(processName.c_str())){

				if (args) sprintf_s(cmd, "cmd.exe /C \"%s\" %s", processFullPath.c_str(), processArgs.c_str());
				else sprintf_s(cmd, "cmd.exe /C \"%s\"", processFullPath.c_str());
				WinExec(cmd, 0);
			}

			Sleep(500);
		}
	}

	else{

		if (processName.empty()) sprintf_s(buff, "[Error]: 'ProcessName' is not specified in the KeepAlive.ini. Terminating.\n\n");
		printf(buff);
		return 1;
	}

    return 0;
}

