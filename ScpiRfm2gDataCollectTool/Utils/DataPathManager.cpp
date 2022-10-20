#include <Windows.h>
#include <ShlObj.h>
#include <QApplication>
#include "DataPathManager.h"

string DataPathManager::getUserDir()
{
	//��ȡӦ�ó��������ļ����Ŀ¼
	char personalPath[MAX_PATH] = { 0 };
	setlocale(LC_ALL, "chs");
	bool bRet = SHGetSpecialFolderPathA(NULL, personalPath, CSIDL_PERSONAL, FALSE);

	string dir = personalPath;
	string userDir = dir + "\\ScpiRfm2gDataCollectTool";
	::CreateDirectoryA(userDir.c_str(), NULL);

	return userDir;
}

string DataPathManager::getDevicePath()
{
	//��ȡdevice�ļ�Ŀ¼
	return getUserDir() + "\\device";
}

string DataPathManager::getLogDirPath()
{
	//��ȡ��־�ļ�Ŀ¼
	//return getUserDir() + "\\Log";
	return QString(QApplication::applicationDirPath() + "\\Log").toStdString();
}

string DataPathManager::getImagesDirPath()
{
	//��ȡimages�ļ���Ŀ¼
	return getUserDir() + "\\Images";
}

string DataPathManager::getPdfDirPath()
{
	//��ȡpdf�ļ����·��
	return getUserDir() + "\\Pdf";
}

string DataPathManager::getSmartCabinetDir()
{
	//��ȡSmartCabinetĿ¼
	char personalPath[MAX_PATH] = { 0 };
	setlocale(LC_ALL, "chs");
	bool bRet = SHGetSpecialFolderPathA(NULL, personalPath, CSIDL_PERSONAL, FALSE);

	string dir = personalPath;
	string smartCabinetDir = dir + "\\ScpiRfm2gDataCollectTool";

	return smartCabinetDir;
}

string DataPathManager::getDumpDirPath()
{
	//��ȡDump�ļ����Ŀ¼
	return getUserDir() + "\\Dump";
}

string DataPathManager::getConfigPath()
{
	//��ȡConfig�ļ����Ŀ¼
	return getUserDir() + "\\Config";
}
