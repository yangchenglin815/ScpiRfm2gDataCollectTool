#ifndef SCPIRFM2GDATACOLLECTTOOL_H
#define SCPIRFM2GDATACOLLECTTOOL_H

#include <QtWidgets/QMainWindow>
#include <QPointer>
#include <QTimer>
#include "ui_ScpiRfm2gDataCollectTool.h"
#include "TcpClient.h"
#include "CommonData.h"
#include "WorkThread.h"

class ScpiRfm2gDataCollectTool : public QMainWindow
{
	Q_OBJECT

public:
	ScpiRfm2gDataCollectTool(QWidget *parent = 0);
	~ScpiRfm2gDataCollectTool();

protected:
	void closeEvent(QCloseEvent *event);

private slots:
    void OnConnectClicked();
	void OnStartClicked();
	void OnStopClicked();
	void OnExitClicked();
	void OnNetworkStatusChanged(int socketState);
	void OnReceiveMessage(QByteArray message);
	void OnTimeout();

private:
	void InitTable();
	void InitUI();
	void InitConnect();
	void InitNetwork();
	void SaveIni();
	void AppendText(QString text);
	void UpdateChannelInfo(const QList<ChannelInfo>& channelInfoList);

private:
	Ui::ScpiRfm2gDataCollectToolClass ui;
	int m_iTotalChannl;
	QPointer<TcpClient> m_pTcpClient;
	QList<ChannelInfo> m_channelInfoList;
	QTimer *m_pTimer;
	QTimer *m_pUpdateTimer;
	QPointer<WorkThread> m_pWorkThread;

	QString m_address;
	uint    m_SendPort;    //默认10001
	uint    m_Reciveport; //默认5555
	uint    m_defGroupId; //默认2
	uint    m_Rfm2gId;   //默认1
};

#endif // SCPIRFM2GDATACOLLECTTOOL_H
