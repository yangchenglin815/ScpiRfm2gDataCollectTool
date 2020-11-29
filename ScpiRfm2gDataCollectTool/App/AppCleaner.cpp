#include "AppCleaner.h"
#include "Log/GlogManager.h"
#include "SystemChecker.h"

AppCleaner::AppCleaner(void)
{
}


AppCleaner::~AppCleaner(void)
{
}

void AppCleaner::clean()
{
    //系统退出时的一些清理操作
	shutdownLog();
}

void AppCleaner::shutdownLog()
{
    //关闭日志系统
	INSTANCE_GLOG_MANAGER->shutdownLog();
}
