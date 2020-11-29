#pragma once

class AppCleaner
{
public:
    AppCleaner(void);
    ~AppCleaner(void);

    void clean();				

private:
	/*关闭日志系统*/
    void shutdownLog();	
};

