#pragma once

#include <QStringList>
#include <QObject>

class AppInitializer : public QObject
{
	Q_OBJECT
public:
    AppInitializer(QObject *parent = 0);
    ~AppInitializer(void);

	/*应用程序初始化*/
    void init();			

private:
	/*单例初始化*/
	void initSingleObjects();

	/*初始化日志库*/
    void initLog();	

	/*监测应用程序临时文件存放目录*/
    void checkDataDirs();

	/*清除过期日志*/
    void clearOutdatedLogs();
};

