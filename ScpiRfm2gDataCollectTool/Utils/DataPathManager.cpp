#include <Windows.h>
#include <ShlObj.h>
#include <QApplication>
#include "DataPathManager.h"

string DataPathManager::getUserDir()
{
	//获取应用程序数据文件存放目录
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
	//获取device文件目录
	return getUserDir() + "\\device";
}

string DataPathManager::getLogDirPath()
{
	//获取日志文件目录
	//return getUserDir() + "\\Log";
	return QString(QApplication::applicationDirPath() + "\\Log").toStdString();
}

string DataPathManager::getImagesDirPath()
{
	//获取images文件夹目录
	return getUserDir() + "\\Images";
}

string DataPathManager::getPdfDirPath()
{
	//获取pdf文件存放路径
	return getUserDir() + "\\Pdf";
}

string DataPathManager::getSmartCabinetDir()
{
	//获取SmartCabinet目录
	char personalPath[MAX_PATH] = { 0 };
	setlocale(LC_ALL, "chs");
	bool bRet = SHGetSpecialFolderPathA(NULL, personalPath, CSIDL_PERSONAL, FALSE);

	string dir = personalPath;
	string smartCabinetDir = dir + "\\ScpiRfm2gDataCollectTool";

	return smartCabinetDir;
}

string DataPathManager::getDumpDirPath()
{
	//获取Dump文件存放目录
	return getUserDir() + "\\Dump";
}

string DataPathManager::getConfigPath()
{
	//获取Config文件存放目录
	return getUserDir() + "\\Config";
}
