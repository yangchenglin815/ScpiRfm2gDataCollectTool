#pragma once

#include <QString>
#include <QStringList>
#include <string>

using std::string;

class FileManager
{
public:
	/*��ȡApp��ǰ·��*/
    static string getAppDir();

	/*�Ƴ�ָ��Ŀ¼*/
    static void removeDir( QString dirPath);

	/*�Ƴ�ָ��Ŀ¼�µ�����*/
    static void removeDirContent(QString dirPath);

	/*�Ƴ�ָ���ļ�*/
    static void removeFile(QString filePath);

	/*����Ŀ¼*/
    static QString createDir(QString dirPath);

	/*������Ŀ¼*/
    static bool renameDir(QString oldDirPath, QString newDirName);

	/*�Ƴ���׺*/
    static QString removeSuffix(QString path);

	/*��ȡĿ¼�µ��ļ�*/
    static QStringList getFilePathsOfDir(QString dirPath, QString suffix, bool needSort = true);
};

