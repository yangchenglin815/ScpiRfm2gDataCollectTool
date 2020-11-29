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
    //清除过期日志
	clearOutdatedLogs();

    //日志系统初始化
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
    //初始化
	initSingleObjects();
	checkDataDirs();
    initLog();
    clearOutdatedLogs();
}

void AppInitializer::checkDataDirs()
{
    //创建相应的文件夹
	QString logsDirPath = QString::fromLocal8Bit(DataPathManager::getLogDirPath().c_str()); 
	QString dumpDirPath = QString::fromLocal8Bit(DataPathManager::getDumpDirPath().c_str());
    FileManager::createDir(logsDirPath);
	FileManager::createDir(dumpDirPath);
}

void AppInitializer::clearOutdatedLogs()
{
    //清除过期的日志信息
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
			{ //删除1个星期前的日志
                if (!QFile::remove(logsDir + "\\" + mfi.fileName()))
				{
					LOG(INFO)<<"移除日志文件失败!";
				}
            }
        }
    }
	else
	{
		LOG(INFO)<<"应用日志目录不存在!";
	}
}

void AppInitializer::initSingleObjects()
{
	//实例化单例
	INSTANCE_USER_CONFIG;
	INSTANCE_GLOG_MANAGER;
}