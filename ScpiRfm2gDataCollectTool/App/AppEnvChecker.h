#pragma once

class AppEnvChecker
{
public:
    AppEnvChecker(void);
    ~AppEnvChecker(void);

    bool check();		

private:
	/*�жϽ����Ƿ��Ѿ�����*/
    bool isAppInstanceExisted();
};

