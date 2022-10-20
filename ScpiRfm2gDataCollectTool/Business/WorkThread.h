#ifndef WORKTHREAD_H
#define WORKTHREAD_H

#include <QObject>
#include <QThread>
#include <QTcpSocket>
#include <QMutex>
#include "dt_stream_packet.h"
#include "CommonData.h"

class WorkThread : public QThread
{
	Q_OBJECT

public:
	explicit WorkThread(QList<ChannelInfo> channelInfoList, QString ip, 
		uint port, QObject *parent = 0);
	virtual ~WorkThread();
	QList<ChannelInfo> GetChannelInfo();
	void setChannelInfo(const QList<ChannelInfo> &channelInfoList);

protected:
	void run();

private:
	void startTcpConn();

signals:
	void networkStatusChanged(int networkState);
	void receiveMessage(QByteArray message);

private slots:
	void readMessage();
	void displayError(QAbstractSocket::SocketError);
	void onStateChanged(QAbstractSocket::SocketState socketState);
	void OnDataChanged(int idx, QString value);

private:
	QTcpSocket *m_pTcpSocket;
	DtStreamPacket *m_packet;
	int m_nCounter;
	QString m_sIpServer;
	uint m_nPort;
	QList<ChannelInfo> m_channelInfoList;
	QMutex m_mutex;
	QByteArray m_recvBuffer;
};

#endif // WORKTHREAD_H
