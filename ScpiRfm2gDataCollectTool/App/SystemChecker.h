#pragma once

#include <QString>
#include <tchar.h>

class SystemChecker
{
public:
	/*��ȡMAC��ַ*/
    static QString getCurrentMacAddr();

	/*�ж�App�����Ƿ����*/
    static bool isAppInstanceExisted();

	/*�ж������������Ƿ����*/
	static bool isLauncherInstanceExisted();

	/*�ж�����������*/
	static void shutdownLauncherInstance();

	/*��������������*/
	static bool restartLauncherInstance();

	/*�ж�ϵͳĬ�ϼ��̽����Ƿ����*/
	static bool isSystemDefKeyBoardInstanceExisted();

	/*�ж�ϵͳĬ�ϼ��̽���*/
	static void shutdownSystemDefKeyBoardInstance();

	/*����ϵͳĬ�ϼ��̽���*/
	static bool restartSystemDefKeyBoardInstance();
 
private:
	/*��ȡ����ID*/
    static bool getProcessIdByName(const wchar_t* szProcessname, unsigned long* lpPID);
};

