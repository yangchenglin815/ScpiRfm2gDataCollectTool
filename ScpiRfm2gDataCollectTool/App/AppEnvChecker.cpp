#include "AppEnvChecker.h"
#include "DataPathManager.h"
#include "SystemChecker.h"
#include <QtWidgets/QMessageBox>

AppEnvChecker::AppEnvChecker(void)
{
}


AppEnvChecker::~AppEnvChecker(void)
{
}

bool AppEnvChecker::check()
{
    return !isAppInstanceExisted();
}

bool AppEnvChecker::isAppInstanceExisted()
{
    //检测进程是否存在
	if (SystemChecker::isAppInstanceExisted()) {
        return true;
    }

    return false;
}
