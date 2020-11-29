#pragma once

#include <QString>
#include <tchar.h>

class SystemChecker
{
public:
	/*获取MAC地址*/
    static QString getCurrentMacAddr();

	/*判断App进程是否存在*/
    static bool isAppInstanceExisted();

	/*判断启动器进程是否存在*/
	static bool isLauncherInstanceExisted();

	/*中断启动器进程*/
	static void shutdownLauncherInstance();

	/*重启启动器进程*/
	static bool restartLauncherInstance();

	/*判断系统默认键盘进程是否存在*/
	static bool isSystemDefKeyBoardInstanceExisted();

	/*中断系统默认键盘进程*/
	static void shutdownSystemDefKeyBoardInstance();

	/*重启系统默认键盘进程*/
	static bool restartSystemDefKeyBoardInstance();
 
private:
	/*获取进程ID*/
    static bool getProcessIdByName(const wchar_t* szProcessname, unsigned long* lpPID);
};

