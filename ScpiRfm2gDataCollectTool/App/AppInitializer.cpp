#include "AppInitializer.h"
#include <QMetaType>
#include <string>
#include <QDateTime>
#include <QDir>
#include <QApplication>
#include <QTimer>
#include "UserConfig.h"
#include "Log/GlogManager.h"
#include "DataPathManager.h"
#include "FileManager.h"
#include "SystemChecker.h"

AppInitializer::AppInitializer(QObject *parent) : QObject(parent)
{

}


AppInitializer::~AppInitializer(void)
{

}

void AppInitializer::initLog()
{
    //���������־
	clearOutdatedLogs();

    //��־ϵͳ��ʼ��
	INSTANCE_GLOG_MANAGER->init();
    INSTANCE_GLOG_MANAGER->setLogFileDir(DataPathManager::getLogDirPath().c_str());
    INSTANCE_GLOG_MANAGER->setInfoLogPrefix("Info");
    INSTANCE_GLOG_MANAGER->setWarningLogPrefix("Warnig");
    INSTANCE_GLOG_MANAGER->setErrorLogPrefix("Error");
    INSTANCE_GLOG_MANAGER->setMaxLogSize(100);
    INSTANCE_GLOG_MANAGER->setStopLoggingIfFullDisk(true);
    INSTANCE_GLOG_MANAGER->setLogbufCacheTime(0);
}

void AppInitializer::init()
{
    //��ʼ��
	initSingleObjects();
	checkDataDirs();
    initLog();
    clearOutdatedLogs();
}

void AppInitializer::checkDataDirs()
{
    //������Ӧ���ļ���
	QString logsDirPath = QString::fromLocal8Bit(DataPathManager::getLogDirPath().c_str()); 
	QString dumpDirPath = QString::fromLocal8Bit(DataPathManager::getDumpDirPath().c_str());
    FileManager::createDir(logsDirPath);
	FileManager::createDir(dumpDirPath);
}

void AppInitializer::clearOutdatedLogs()
{
    //������ڵ���־��Ϣ
	QString logsDir = QString::fromLocal8Bit(DataPathManager::getLogDirPath().c_str()); 
    QDir dir(logsDir);
    if (dir.exists())
	{
        dir.setFilter(QDir::Files); 
        int currentDate = QDateTime::currentDateTime().toString("yyyyMMdd").toInt(); 
        foreach (QFileInfo mfi ,dir.entryInfoList())
		{
            int logDate = mfi.fileName().mid(4, 8).toInt(); 
            if (currentDate - logDate > 7) 
			{ //ɾ��1������ǰ����־
                if (!QFile::remove(logsDir + "\\" + mfi.fileName()))
				{
					LOG(INFO)<<"�Ƴ���־�ļ�ʧ��!";
				}
            }
        }
    }
	else
	{
		LOG(INFO)<<"Ӧ����־Ŀ¼������!";
	}
}

void AppInitializer::initSingleObjects()
{
	//ʵ��������
	INSTANCE_USER_CONFIG;
	INSTANCE_GLOG_MANAGER;
}