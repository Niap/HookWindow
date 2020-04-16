#include "stdio.h"
#include "conio.h"
#include "windows.h"

#define	DEF_DLL_NAME		"HookDll.dll"
#define	DEF_HOOKSTART		"HookStart"
#define	DEF_HOOKSTOP		"HookStop"

typedef void(*PFN_HOOKSTART)();
typedef void(*PFN_HOOKSTOP)();



void main()
{

	HMODULE			hDll = NULL;
	PFN_HOOKSTART	HookStart = NULL;
	PFN_HOOKSTOP	HookStop = NULL;
	char			ch = 0;

	// 加载KeyHook.dll 
	hDll = LoadLibraryA(DEF_DLL_NAME);
	if (hDll == NULL)
	{
		printf("LoadLibrary(%s) failed!!! [%d]", DEF_DLL_NAME, GetLastError());
		return;
	}

	//获取导出函数地址
	HookStart = (PFN_HOOKSTART)GetProcAddress(hDll, DEF_HOOKSTART);
	HookStop = (PFN_HOOKSTOP)GetProcAddress(hDll, DEF_HOOKSTOP);

	//开始hook
	HookStart();

	//用户输入‘q’停止hook
	printf("press 'q' to quit!\n");
	while (_getch() != 'q');

	// 停止hook
	HookStop();

	//卸载 KeyHook.dll
	FreeLibrary(hDll);
}
