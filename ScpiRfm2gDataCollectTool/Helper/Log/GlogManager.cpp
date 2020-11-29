#include "GlogManager.h"
#include <string>

using std::string;

#ifdef _DEBUG
	#pragma comment(lib,"glog_d.lib")
#else
	#pragma comment(lib,"glog.lib")
#endif

using namespace google; 

GlogManager::GlogManager()
{
}

GlogManager::~GlogManager()
{
}

GlogManager* GlogManager::getInstance()
{
	static GlogManager instance;
	return &instance;
}

void GlogManager::init(const char* application_name/*= "ScpiRfm2gDataCollectTool"*/)
{
	google::InitGoogleLogging(application_name);
}

void GlogManager::setLogbufCacheTime(int time)
{
	FLAGS_logbufsecs = time;
}

void GlogManager::setMaxLogSize(int size)
{
	FLAGS_max_log_size = size;
}

void GlogManager::setStopLoggingIfFullDisk(bool flag)
{
	FLAGS_stop_logging_if_full_disk = flag;
}

void GlogManager::setLogToStdErr(bool flag)
{
	FLAGS_logtostderr = flag;
}

void GlogManager::setLogFileDir(const char* dir)
{
	FLAGS_log_dir = dir;
}

void GlogManager::setLogFilenameExtension(const char* extension)
{
	google::SetLogFilenameExtension(extension);
}

void GlogManager::setInfoLogPrefix(const char* prefix)
{
    string strDest = FLAGS_log_dir;
    string strPrefix = prefix; 
	strDest += "\\";
	strDest += strPrefix;
	google::SetLogDestination(google::GLOG_INFO,strDest.c_str());  
}

void GlogManager::setWarningLogPrefix(const char* prefix)
{
	string strDest = FLAGS_log_dir;
	string strPrefix = prefix; 
	strDest += "\\";
	strDest += strPrefix;
	google::SetLogDestination(google::GLOG_WARNING,strDest.c_str()); 
}

void GlogManager::setErrorLogPrefix(const char* prefix)
{
	string strDest = FLAGS_log_dir;
	string strPrefix = prefix; 
	strDest += "\\";
	strDest += strPrefix;
	google::SetLogDestination(google::GLOG_ERROR,strDest.c_str()); 
}

void GlogManager::shutdownLog()
{
	google::ShutdownGoogleLogging();
}

void GlogManager::setDumpCallbackFunc(dumpCallBackFunc fn)
{
	//google::Install
	//google::InstallFailureWriter(fn);
}