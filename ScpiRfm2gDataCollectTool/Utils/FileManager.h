#pragma once

#include <QString>
#include <QStringList>
#include <string>

using std::string;

class FileManager
{
public:
	/*获取App当前路径*/
    static string getAppDir();

	/*移除指定目录*/
    static void removeDir( QString dirPath);

	/*移除指定目录下的内容*/
    static void removeDirContent(QString dirPath);

	/*移除指定文件*/
    static void removeFile(QString filePath);

	/*创建目录*/
    static QString createDir(QString dirPath);

	/*重命名目录*/
    static bool renameDir(QString oldDirPath, QString newDirName);

	/*移除后缀*/
    static QString removeSuffix(QString path);

	/*获取目录下的文件*/
    static QStringList getFilePathsOfDir(QString dirPath, QString suffix, bool needSort = true);
};

