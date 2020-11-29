#pragma once

#include <tchar.h>
#include <DbgHelp.h>
#include <QDir>
#include <QString>
#include "DataPathManager.h"

#pragma comment(lib, "dbghelp.lib")

//解决无法解析的外部符号 __imp_wsprintfW
#pragma comment(lib, "user32.lib")

#ifdef UNICODE
#define TSprintf	wsprintf
#else
#define TSprintf	sprintf
#endif

// 启动自动生成dump文件的话，只需要在main函数开始处
// 调用该函数（EnableAutoDump）即可
void EnableAutoDump();

// 其它函数
LONG ApplicationCrashHandler(EXCEPTION_POINTERS *pException);
void CreateDumpFile(LPCWSTR lpstrDumpFilePathName, EXCEPTION_POINTERS *pException);

LONG ApplicationCrashHandler(EXCEPTION_POINTERS *pException)
{


	TCHAR szDumpDir[MAX_PATH] = { 0 };
	TCHAR szDumpFile[MAX_PATH] = { 0 };
	TCHAR szMsg[MAX_PATH] = { 0 };
	SYSTEMTIME	stTime = { 0 };

	// 构建dump文件路径
	GetLocalTime(&stTime);
	QString path = QString::fromLocal8Bit(DataPathManager::getDumpDirPath().c_str());
	path.toWCharArray(szDumpDir);
	//::GetCurrentDirectory(MAX_PATH, szDumpDir);
	TSprintf(szDumpFile, (L"%s\\%04d%02d%02d_%02d%02d%02d.dmp"), szDumpDir,
		stTime.wYear, stTime.wMonth, stTime.wDay,
		stTime.wHour, stTime.wMinute, stTime.wSecond);
	// 创建dump文件
	CreateDumpFile(szDumpFile, pException);



	// 这里弹出一个错误对话框并退出程序
	TSprintf(szMsg, (L"Sorry, ScpiRfm2gDataCollectTool has crashed, please contact to us for help! \r\ndump file : %s"), szDumpFile);
	FatalAppExit(-1, szMsg);

	return EXCEPTION_EXECUTE_HANDLER;
}

void EnableAutoDump()
{
	SetUnhandledExceptionFilter((LPTOP_LEVEL_EXCEPTION_FILTER)ApplicationCrashHandler);
}

// 创建Dump文件
void CreateDumpFile(LPCWSTR lpstrDumpFilePathName, EXCEPTION_POINTERS *pException)
{
	// 创建Dump文件
	HANDLE hDumpFile = CreateFile(lpstrDumpFilePathName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	// Dump信息
	MINIDUMP_EXCEPTION_INFORMATION dumpInfo;
	dumpInfo.ExceptionPointers = pException;
	dumpInfo.ThreadId = GetCurrentThreadId();
	dumpInfo.ClientPointers = TRUE;

	// 写入Dump文件内容
	//MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hDumpFile, MiniDumpWithPrivateReadWriteMemory, &dumpInfo, NULL, NULL);
	MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hDumpFile, MiniDumpNormal, &dumpInfo, NULL, NULL);
	CloseHandle(hDumpFile);
}