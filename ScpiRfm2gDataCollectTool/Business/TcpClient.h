#ifndef TCPCLIENT_H
#define TCPCLIENT_H

#include <QObject>
#include <QTcpSocket>
#include "dt_stream_packet.h"

class TcpClient : public QObject
{
	Q_OBJECT

public:
	explicit TcpClient(QObject *parent = 0);
	virtual ~TcpClient();

public:
	void newConnect(const QString &address, quint16 port);
	void sendMessage(QByteArray message);
	void close();

signals:
	void networkStatusChanged(int networkState);
	void receiveMessage(QByteArray message);
	void onDataChanged(int idx, QString value);

private slots:
    void readMessage();
    void displayError(QAbstractSocket::SocketError);
	void onStateChanged(QAbstractSocket::SocketState socketState);

private:
	QTcpSocket *m_pTcpSocket;
};

#endif // TCPCLIENT_H
