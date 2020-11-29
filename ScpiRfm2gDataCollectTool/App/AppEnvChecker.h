#pragma once

class AppEnvChecker
{
public:
    AppEnvChecker(void);
    ~AppEnvChecker(void);

    bool check();		

private:
	/*判断进程是否已经存在*/
    bool isAppInstanceExisted();
};

