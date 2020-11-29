#include "CommonFunc.h"
#include <QApplication>
#include <QDesktopWidget>
#include <windows.h>
#include <QFile>

void CommonFunc::shutDownSystem()
{
#if defined Q_OS_WIN32

	HANDLE hToken;
	TOKEN_PRIVILEGES tkp;

	//获取进程标志
	if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
		return;

	//获取关机特权的LUID
	LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, &tkp.Privileges[0].Luid);
	tkp.PrivilegeCount = 1;
	tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

	//获取这个进程的关机特权
	AdjustTokenPrivileges(hToken, false, &tkp, 0, (PTOKEN_PRIVILEGES)NULL, 0);
	if (GetLastError() != ERROR_SUCCESS)
		return;

	// 强制关闭计算机
	ExitWindowsEx(EWX_SHUTDOWN | EWX_FORCE, 0);
	Sleep(500);
#endif
}