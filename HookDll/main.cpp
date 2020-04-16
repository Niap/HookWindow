#include <windows.h>
#include <iostream>
#include <Psapi.h>
#include <string>

char * g_filters = "notepad";

HINSTANCE g_hInstance = NULL;
HHOOK g_hHook = NULL;
HWND g_hWnd = NULL;

INT g_screenPhysicsWidth = 0;
INT g_screenPhysicsHeight = 0;


char * ReadConfig(TCHAR *filePath)
{
	HANDLE pFile;
	DWORD fileSize;
	char *tmpBuf, *buffer;
	DWORD dwBytesRead, dwBytesToRead;

	pFile = CreateFile(filePath, GENERIC_READ,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,        //打开已存在的文件 
		FILE_ATTRIBUTE_NORMAL,
		NULL);

	if (pFile == INVALID_HANDLE_VALUE)
	{
		printf("open file error!\n");
		CloseHandle(pFile);
		return FALSE;
	}

	fileSize = GetFileSize(pFile, NULL);          //得到文件的大小
	buffer = (char *)malloc(fileSize + 1);
	ZeroMemory(buffer, fileSize + 1);

	dwBytesToRead = fileSize;
	dwBytesRead = 0;
	tmpBuf = buffer;

	do{                                       //循环读文件，确保读出完整的文件    

		ReadFile(pFile, tmpBuf, dwBytesToRead, &dwBytesRead, NULL);

		if (dwBytesRead == 0)
			break;

		dwBytesToRead -= dwBytesRead;
		tmpBuf += dwBytesRead;

	} while (dwBytesToRead > 0);

	//  TODO 处理读到的数据 buffer

	CloseHandle(pFile);

	return buffer;
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD dwReason, LPVOID lpvReserved)
{
	HDC hdcScreen = NULL;
	switch (dwReason)
	{
	case DLL_PROCESS_ATTACH:
		g_filters = ReadConfig(TEXT("c:\\config.ini"));
		g_screenPhysicsWidth = GetSystemMetrics(SM_CXSCREEN);
		g_screenPhysicsHeight = GetSystemMetrics(SM_CYSCREEN);
		g_hInstance = hinstDLL;
		break;

	case DLL_PROCESS_DETACH:
		break;
	}

	return TRUE;
}

TCHAR * CharToTchar(const char * _char)
{
	int iLength;
	TCHAR * newStr;

	iLength = MultiByteToWideChar(CP_ACP, 0, _char, strlen(_char) + 1, NULL, 0);
	newStr = (TCHAR *)malloc(iLength * 2);
	MultiByteToWideChar(CP_ACP, 0, _char, strlen(_char) + 1, newStr, iLength);
	return newStr;
}



BOOL isTargetProcess(HWND   hWnd)
{
	DWORD processID = NULL;
	GetWindowThreadProcessId(hWnd, &processID);
	HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processID);

	//LPSTR procName;
	TCHAR procName[MAX_PATH] = { 0 };
	GetProcessImageFileName(hProcess, procName, MAX_PATH);
	TCHAR * source = procName;

	char * copyofg_filter = (char *)malloc(strlen(g_filters) + 1);
	strcpy_s(copyofg_filter, strlen(g_filters)+1 , g_filters);

	char *token = NULL;
	char *next_token = NULL;
	char seps[] = ",";
	token = strtok_s(copyofg_filter, seps, &next_token);

	if (token != NULL && wcsstr(source, CharToTchar(token)) != NULL){
		return true;
	}
	
	while (token != NULL)
	{
		// Get next token:
		if (token != NULL)
		{
			token = strtok_s(NULL, seps, &next_token);
			if (token != NULL && wcsstr(source, CharToTchar(token)) != NULL){ 
				return true;
			}
		}
	}
	return false;

}

LRESULT CALLBACK HookProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	if (nCode<0)
		return CallNextHookEx(g_hHook, nCode, wParam, lParam);
	
	if (nCode >= 0)
	{
		switch (nCode)
		{
		case HCBT_ACTIVATE:
		{
			HWND currentHwnd = (HWND)wParam;

			if (isTargetProcess(currentHwnd))
			{
				SetWindowPos(currentHwnd, HWND_TOPMOST, 0, 0, g_screenPhysicsWidth, g_screenPhysicsHeight,  SWP_SHOWWINDOW );
				SetWindowPos(currentHwnd, HWND_TOPMOST, 0, 0, g_screenPhysicsWidth, g_screenPhysicsHeight, SWP_SHOWWINDOW|SWP_NOMOVE);
				//ShowWindow(currentHwnd, SW_MAXIMIZE);
				LONG lStyle = GetWindowLong(currentHwnd, GWL_STYLE);
				lStyle &= ~( WS_MINIMIZEBOX | WS_MAXIMIZEBOX);
				SetWindowLong(currentHwnd, GWL_STYLE, lStyle);
			}

			break;
		}

		case HCBT_MOVESIZE:
		{
			HWND currentHwnd = (HWND)wParam;
			if (isTargetProcess(currentHwnd)){
				return 1;
			}
			break;
		}

		case HCBT_MINMAX:
		{
			HWND currentHwnd = (HWND)wParam;
			if (LOWORD(lParam) == SW_MINIMIZE || LOWORD(lParam) == SW_RESTORE) {
				if (isTargetProcess(currentHwnd)){
					return 1;
				}
			}
			break;
		}
		default:
			break;
		}
	}

	// 如果不是notepad.exe，则调用CallNextHookEx函数，将消息传递给应用程序
	return CallNextHookEx(g_hHook, nCode, wParam, lParam);
}




#ifdef __cplusplus
extern "C" {
#endif
	__declspec(dllexport) void HookStart()
	{
		g_hHook = SetWindowsHookEx(WH_CBT, HookProc, g_hInstance, 0);
	}

	__declspec(dllexport) void HookStop()
	{
		if (g_hHook)
		{
			UnhookWindowsHookEx(g_hHook);
			g_hHook = NULL;
		}
	}
#ifdef __cplusplus
}
#endif