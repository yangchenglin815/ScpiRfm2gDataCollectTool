#ifndef __GLOG_MANAGER_H__
#define __GLOG_MANAGER_H__


#define GLOG_NO_ABBREVIATED_SEVERITIES
#define GOOGLE_GLOG_DLL_DECL
#define _QLOG(s) s.toLocal8Bit().data()  

#include "logging.h"  

typedef void (*dumpCallBackFunc)(const char *data, int size);

#define INSTANCE_GLOG_MANAGER GlogManager::getInstance()

class GlogManager
{
public:
	static GlogManager* getInstance();

	/*��ʼ��*/
	void init(const char* application_name = "ScpiRfm2gDataCollectTool");  

	/*������־�ڻ��汣��ʱ�䣬0��ʾ����д��*/
	void setLogbufCacheTime(int time);

	/*������־�ļ�����С������Ϊ��λ*/
	void setMaxLogSize(int size); 

	/*���õ�������ʱ�Ƿ�ֹͣ����*/
	void setStopLoggingIfFullDisk(bool flag);

	/*�����Ƿ��������׼�������*/
	void setLogToStdErr(bool flag);

	/*������־�ļ����ɵ�Ŀ¼*/
	void setLogFileDir(const char* dir);

	/*������־�ļ���������չ*/
	void setLogFilenameExtension(const char* extension);

	/*����INFO������־�ļ�ǰ׺*/
	void setInfoLogPrefix(const char* prefix);

	/*����Warning������־ǰ׺*/
	void setWarningLogPrefix(const char* prefix);

	/*����Error������־ǰ׺*/
	void setErrorLogPrefix(const char* prefix);

	/*�ر���־ϵͳ���ͷ���Դ*/
	void shutdownLog();

	/*���ý��̱���ʱ�Ļص�����ʱδ�õ�*/
	void setDumpCallbackFunc(dumpCallBackFunc fn);

private:
	GlogManager();  
	virtual ~GlogManager();
};


#endif