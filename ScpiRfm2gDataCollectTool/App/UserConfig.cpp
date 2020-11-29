#include <QSettings>
#include <QTextCodec>
#include <QCoreApplication>

#include "UserConfig.h"
#include "FileManager.h"
#include "DataPathManager.h"
#include "SystemChecker.h"

UserConfig::UserConfig()
{
	m_configPath = QString::fromStdString(DataPathManager::getConfigPath()) + "/config.ini";
	m_ConfigIni = new QSettings(m_configPath, QSettings::IniFormat);
	m_ConfigIni->setIniCodec(QTextCodec::codecForName("utf-8"));
}

UserConfig::~UserConfig()
{
	if (m_ConfigIni != NULL)
	{
		delete m_ConfigIni;
		m_ConfigIni = NULL;
	}
}

UserConfig* UserConfig::getInstance()
{
	static UserConfig instance;
	return &instance;
}

QString UserConfig::readSetting(QString group, QString key)
{
	QString value = m_ConfigIni->value(group + "/" + key).toString();
	if (value.isEmpty())
		value = "-1";
	
	return value;
}

bool UserConfig::ReadIntSetting(QString group, QString key, int &value)
{
	bool bRet = false;
	QString _value = m_ConfigIni->value(group + "/" + key).toString();

	if (!_value.isEmpty()) {
		value = _value.toInt();
		bRet = true;
	}
	return bRet;
}

bool UserConfig::readStringSetting(QString group, QString key, QString &value)
{
	bool bRet = false;
	value = m_ConfigIni->value(group + "/" + key).toString();

	if (!value.isEmpty())
		bRet = true;

	return bRet;
}

int UserConfig::writeSetting(QString group, QString key, const QString data)
{
	m_ConfigIni->beginGroup(group);
	m_ConfigIni->setValue(key, data);
	m_ConfigIni->endGroup();

	return 0;
}

void UserConfig::beginGroup(const QString &group)
{
	m_ConfigIni->beginGroup(group);
}

void UserConfig::endGroup()
{
	m_ConfigIni->endGroup();
}

void UserConfig::writeValue(QString key, const QVariant &data)
{
	m_ConfigIni->setValue(key, data);
}

QStringList UserConfig::allkeys()
{
	return m_ConfigIni->allKeys();
}

QStringList UserConfig::childKeys(QString group)
{
	m_ConfigIni->beginGroup(group);
	QStringList childKeysList = m_ConfigIni->childKeys();
	m_ConfigIni->endGroup();

	return childKeysList;
}

QString UserConfig::getVersion()
{
	QString version = "UserConfig V2.0.0.200630";

	return version;
}

int UserConfig::removeKey(QString group, QString key)
{
	m_ConfigIni->beginGroup(group);    // 设置查找节点
	QStringList str2 = m_ConfigIni->allKeys();

	foreach(QString strkey, str2)
	{
		if (key == strkey)    // 判断"键"是否存在
		{
			m_ConfigIni->remove(key);      // 删除此键
		}
		else if ("" == key)
		{
			m_ConfigIni->remove(strkey);
		}

	}
	m_ConfigIni->endGroup();   // 结束掉Group
	return 0;
}

QString UserConfig::readSettingEx(QString group, QString key)
{
	QString value = m_ConfigIni->value(group + "/" + key, "null").toString();
	return value;
}
