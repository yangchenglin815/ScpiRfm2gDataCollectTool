#include "TcpClient.h"
#include "Log/GlogManager.h"
#include "CommEnum.h"
#include "ConstData.h"
#include <QDebug>

TcpClient::TcpClient(QObject *parent)
	: QObject(parent)
{
	m_pTcpSocket = new QTcpSocket(this);
	connect(m_pTcpSocket, &QTcpSocket::readyRead, this, &TcpClient::readMessage);
	connect(m_pTcpSocket, &QTcpSocket::stateChanged, this, &TcpClient::onStateChanged);
	connect(m_pTcpSocket, SIGNAL(error(QAbstractSocket::SocketError)),
		this, SLOT(displayError(QAbstractSocket::SocketError)));
}

TcpClient::~TcpClient()
{
	
}

void TcpClient::newConnect(const QString &address, quint16 port)
{
	m_pTcpSocket->abort();
	m_pTcpSocket->connectToHost(address, port);
}

void TcpClient::sendMessage(QByteArray message)
{
	m_pTcpSocket->write(message + ENDF);
	m_pTcpSocket->waitForBytesWritten(500);
	m_pTcpSocket->flush();
	m_pTcpSocket->waitForReadyRead(500);
}

void TcpClient::close()
{
	if (m_pTcpSocket)
	{
		m_pTcpSocket->close();
		m_pTcpSocket->abort();
	}
}

void TcpClient::readMessage()
{
	QByteArray message = m_pTcpSocket->readAll();
	emit receiveMessage(message);

// 		DtStreamPacket packet;
// 		ByteBuffer packet_header_buffer(DT_PACKET_HEADER_SIZE, 0);
// 		memcpy(packet_header_buffer.data(), message.data(), DT_PACKET_HEADER_SIZE);
// 		if (packet.processPacketHeader(packet_header_buffer) < 0)
// 		{
// 			qDebug() << "Could not process packet header";
// 			return;
// 		}
// 		int32_t packet_size = packet.getPacketSize() - DT_PACKET_HEADER_SIZE;
// 		qDebug() << "packet_size: " << packet_size;
// 		ByteBuffer packet_buffer(packet_size, 0);
// 		memcpy(packet_buffer.data(), message.data() + DT_PACKET_HEADER_SIZE, packet_size);
// 		if (packet.processSubPackets(packet_buffer) < 0)
// 		{
// 			qDebug() << "Could not process packet";
// 			return;
// 		}
// 		qDebug()<< QString::fromStdString(packet.getPacketInfo());
// 		QByteArray data;
// 		data.append(QString::fromStdString(packet.getPacketInfo()));
// 		emit receiveMessage(data);
// 		//packet.printChannelSamples();
// 		packet.clearChannels();
}

void TcpClient::displayError(QAbstractSocket::SocketError)
{
	LOG(INFO) << "Error: " << _QLOG(m_pTcpSocket->errorString());
}

void TcpClient::onStateChanged(QAbstractSocket::SocketState socketState)
{
	switch (socketState)
	{
	case QAbstractSocket::ConnectedState:
		emit networkStatusChanged(SOCKET_CONNECTED);
		LOG(INFO) << "Socket Connected.";
		break;
	case QAbstractSocket::ConnectingState:
		emit networkStatusChanged(SOCKET_CONNECTING);
		LOG(INFO) << "Socket Is Connecting...";
		break;
	case QAbstractSocket::UnconnectedState:
		emit networkStatusChanged(SOCKET_DISCONNECT);
		LOG(INFO) << "Socket Unconnected.";
		break;
	default:
		break;
	}
}
