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
    //ϵͳ�˳�ʱ��һЩ�������
	shutdownLog();
}

void AppCleaner::shutdownLog()
{
    //�ر���־ϵͳ
	INSTANCE_GLOG_MANAGER->shutdownLog();
}
