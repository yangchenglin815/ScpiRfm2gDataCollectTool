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

	/*初始化*/
	void init(const char* application_name = "ScpiRfm2gDataCollectTool");  

	/*设置日志在缓存保存时间，0表示立即写入*/
	void setLogbufCacheTime(int time);

	/*设置日志文件最大大小，以兆为单位*/
	void setMaxLogSize(int size); 

	/*设置当磁盘满时是否停止生成*/
	void setStopLoggingIfFullDisk(bool flag);

	/*设置是否输出到标准输入输出*/
	void setLogToStdErr(bool flag);

	/*设置日志文件生成的目录*/
	void setLogFileDir(const char* dir);

	/*设置日志文件的名称扩展*/
	void setLogFilenameExtension(const char* extension);

	/*设置INFO类型日志文件前缀*/
	void setInfoLogPrefix(const char* prefix);

	/*设置Warning级别日志前缀*/
	void setWarningLogPrefix(const char* prefix);

	/*设置Error级别日志前缀*/
	void setErrorLogPrefix(const char* prefix);

	/*关闭日志系统，释放资源*/
	void shutdownLog();

	/*设置进程崩溃时的回调，暂时未用到*/
	void setDumpCallbackFunc(dumpCallBackFunc fn);

private:
	GlogManager();  
	virtual ~GlogManager();
};


#endif