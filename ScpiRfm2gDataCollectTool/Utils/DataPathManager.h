#pragma once

#include <QString>
#include <string>
using std::string;

class DataPathManager
{
public:
	/*获取用户目录*/
	static string getUserDir();

	/*获取设备码配置文件路径*/
	static string getDevicePath();

	/*获取日志文件路径*/
	static string getLogDirPath();

	/*获取图片目录路径*/
	static string getImagesDirPath();

	/*获取pdf文件路径*/
	static string getPdfDirPath();

	/*获取SmartCabinet路径*/
	static string getSmartCabinetDir();

	/*获取Dump路径*/
	static string getDumpDirPath();

	/*获取Config路径*/
	static string getConfigPath();
};