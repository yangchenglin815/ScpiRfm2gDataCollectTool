#include "ScpiRfm2gDataCollectTool.h"
#include <QtWidgets/QApplication>
#include <QTextCodec>
#include <QFont>
#include <windows.h>
#include "WindowsDump.h"
#include "AppEnvChecker.h"
#include "AppInitializer.h"
#include "AppCleaner.h"
#include "Log/GlogManager.h"
#include "FileManager.h"

int main(int argc, char *argv[])
{
	//初始化Dump
	EnableAutoDump();
#if (QT_VERSION >= QT_VERSION_CHECK(5, 6, 0))
	QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif
	QApplication a(argc, argv);

	//设置字体
	QFont font;
	font.setFamily(QString::fromLocal8Bit("微软雅黑"));
	a.setFont(font);

	//设置编码格式
	QTextCodec *codec = QTextCodec::codecForName("GB2312");
	QTextCodec::setCodecForLocale(codec);

	//
	QString configDirPath = QString::fromLocal8Bit(DataPathManager::getConfigPath().c_str());
	FileManager::createDir(configDirPath);
	QString desPath = configDirPath + "/config.ini";
	QString srcPath = QApplication::applicationDirPath() + "/config.ini";
	QFile file(desPath);
	if (!file.exists())
	{
		QFile::copy(srcPath, desPath);
	}

	//检查环境（如是否单例等）
	AppEnvChecker envChecker;
	if (!envChecker.check()) {
		return -1;
	}

	//初始化
	AppInitializer  initializer;
	initializer.init();
	LOG(INFO) << "App 启动.";

	ScpiRfm2gDataCollectTool w;
	w.show();

	int ret = a.exec();
	LOG(INFO) << "App 退出.";

	//清理
	AppCleaner cleaner;
	cleaner.clean();

	return ret;
}
