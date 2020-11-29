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
    //�������Ƿ����
	if (SystemChecker::isAppInstanceExisted()) {
        return true;
    }

    return false;
}
