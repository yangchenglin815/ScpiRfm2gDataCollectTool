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
	//��ʼ��Dump
	EnableAutoDump();
#if (QT_VERSION >= QT_VERSION_CHECK(5, 6, 0))
	QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif
	QApplication a(argc, argv);

	//��������
	QFont font;
	font.setFamily(QString::fromLocal8Bit("΢���ź�"));
	a.setFont(font);

	//���ñ����ʽ
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

	//��黷�������Ƿ����ȣ�
	AppEnvChecker envChecker;
	if (!envChecker.check()) {
		return -1;
	}

	//��ʼ��
	AppInitializer  initializer;
	initializer.init();
	LOG(INFO) << "App ����.";

	ScpiRfm2gDataCollectTool w;
	w.show();

	int ret = a.exec();
	LOG(INFO) << "App �˳�.";

	//����
	AppCleaner cleaner;
	cleaner.clean();

	return ret;
}
