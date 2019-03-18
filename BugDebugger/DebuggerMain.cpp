#include <Windows.h>
#include <process.h>
#include <tchar.h>
#include <iostream>
#include <string>

using std::cout;
using std::cin;
using std::endl;
using std::string;

LPCSTR ProcessNameToDebug = "c:\\windows\\system32\\notepad.exe";
CHAR ProcessPath[MAX_PATH] = { 0 };
CHAR LoadedDllPath[MAX_PATH] = { 0 };

int main() {
	HANDLE hDebuggee = NULL;
	BOOLEAN isOk = FALSE;
	STARTUPINFOA si = { 0 };
	PROCESS_INFORMATION pi = { 0 };

	isOk = CreateProcessA(
		ProcessNameToDebug,
		NULL, NULL, NULL, FALSE,
		DEBUG_ONLY_THIS_PROCESS,
		NULL, NULL,
		&si, &pi);
	if (!isOk) {
		cout << "Create debuggee process failed."
			 << "Error code: " << GetLastError() << endl;
		return 1;
	}

	hDebuggee = pi.hProcess;

	DEBUG_EVENT debugEvent = { 0 };
	BOOLEAN isContinueDebug = TRUE;
	DWORD debugStatus = DBG_CONTINUE;
	DWORD length = 0;

	while (isContinueDebug) {
		if (!WaitForDebugEvent(
			&debugEvent,
			INFINITE))
			return 1;	// Get debug event failed, just return

		switch (debugEvent.dwDebugEventCode) {
		case CREATE_PROCESS_DEBUG_EVENT:
			cout << "CREATE_PROCESS_DEBUG_EVENT" << endl;
			length = GetFinalPathNameByHandleA(
				debugEvent.u.CreateProcessInfo.hFile,
				ProcessPath,
				MAX_PATH,
				FILE_NAME_NORMALIZED);
			if (length > MAX_PATH || !length) {
				cout << "Cannot get debuggee's path" << endl;
				continue;	
			}

			cout << "Debugged process is " << ProcessPath << endl;
			break;

		case CREATE_THREAD_DEBUG_EVENT:
			cout << "CREATE_THREAD_DEBUG_EVENT:" << endl;
			cout << "Thread 0x" << debugEvent.u.CreateThread.hThread
				<< " (Id: " << debugEvent.dwThreadId << ")"
				<< " Created at " << debugEvent.u.CreateThread.lpStartAddress << endl;
				 
			break;
				 
		case EXIT_PROCESS_DEBUG_EVENT:
			cout << "EXIT_PROCESS_DEBUG_EVENT:" << endl;
			cout << "Debuggee exited with code " << debugEvent.u.ExitProcess.dwExitCode << endl;
			isContinueDebug = FALSE;

			break;

		case LOAD_DLL_DEBUG_EVENT:
			cout << "LOAD_DLL_DEBUG_EVENT" << endl;

			length = GetFinalPathNameByHandleA(debugEvent.u.LoadDll.hFile, LoadedDllPath, MAX_PATH, FILE_NAME_OPENED);
			if (!length || length > MAX_PATH)	continue;

			cout << "Loaded dll " << LoadedDllPath << "at " << debugEvent.u.LoadDll.lpBaseOfDll << endl;
			break;
		}
		cout << endl;

		ContinueDebugEvent(debugEvent.dwProcessId, debugEvent.dwThreadId, debugStatus);

		debugStatus = DBG_CONTINUE;
	}
}