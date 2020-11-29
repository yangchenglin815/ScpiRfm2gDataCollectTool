#ifndef  __USERCONFIG_H__
#define  __USERCONFIG_H__

#include <QString>
#include <QSettings>

#define INSTANCE_USER_CONFIG UserConfig::getInstance()

class UserConfig
{
public:
	static UserConfig* getInstance();
	QString readSetting(QString group, QString key);
	bool ReadIntSetting(QString group, QString key, int &value);
	bool readStringSetting(QString group, QString key, QString &value);
	int writeSetting(QString group, QString key, const QString data);
	void beginGroup(const QString &group);
	void endGroup();
	void writeValue(QString key, const QVariant &data);
	QStringList allkeys();
	QStringList childKeys(QString group);
	QString getVersion();
	int removeKey(QString group, QString key);
	QString readSettingEx(QString group, QString key);

private:
	UserConfig();
	virtual ~UserConfig();
	
private:
	QString m_configPath;
	QSettings* m_ConfigIni;
};

#endif