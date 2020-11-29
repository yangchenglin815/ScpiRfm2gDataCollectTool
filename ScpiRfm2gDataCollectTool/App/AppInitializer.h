#pragma once

#include <QStringList>
#include <QObject>

class AppInitializer : public QObject
{
	Q_OBJECT
public:
    AppInitializer(QObject *parent = 0);
    ~AppInitializer(void);

	/*Ӧ�ó����ʼ��*/
    void init();			

private:
	/*������ʼ��*/
	void initSingleObjects();

	/*��ʼ����־��*/
    void initLog();	

	/*���Ӧ�ó�����ʱ�ļ����Ŀ¼*/
    void checkDataDirs();

	/*���������־*/
    void clearOutdatedLogs();
};

