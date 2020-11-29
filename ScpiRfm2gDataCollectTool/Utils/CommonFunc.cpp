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

	//��ȡ���̱�־
	if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
		return;

	//��ȡ�ػ���Ȩ��LUID
	LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, &tkp.Privileges[0].Luid);
	tkp.PrivilegeCount = 1;
	tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

	//��ȡ������̵Ĺػ���Ȩ
	AdjustTokenPrivileges(hToken, false, &tkp, 0, (PTOKEN_PRIVILEGES)NULL, 0);
	if (GetLastError() != ERROR_SUCCESS)
		return;

	// ǿ�ƹرռ����
	ExitWindowsEx(EWX_SHUTDOWN | EWX_FORCE, 0);
	Sleep(500);
#endif
}