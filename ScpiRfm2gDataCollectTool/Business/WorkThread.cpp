#include "WorkThread.h"
#include "Log/GlogManager.h"
#include "CommEnum.h"
#include "ConstData.h"

WorkThread::WorkThread(QList<ChannelInfo> channelInfoList, QString ip, uint port, QObject *parent)
	: QThread(parent)
	, m_sIpServer(ip)
	, m_nPort(port)
	, m_nCounter(0)
{
	m_pTcpSocket = Q_NULLPTR;
	m_channelInfoList = channelInfoList;
}

WorkThread::~WorkThread()
{
	m_pTcpSocket->abort();
	m_pTcpSocket->close();
	m_pTcpSocket->deleteLater();
}

QList<ChannelInfo> WorkThread::GetChannelInfo()
{
	QMutexLocker locker(&m_mutex);
	return m_channelInfoList;
}

void WorkThread::setChannelInfo(const QList<ChannelInfo> &channelInfoList)
{
	QMutexLocker locker(&m_mutex);
	m_channelInfoList = channelInfoList;
	if (m_packet)
	{
		m_packet->OnClear();
	}
}

void WorkThread::run()
{
	LOG(INFO) << "Thread run.";
	this->startTcpConn();
	this->exec();
 	m_pTcpSocket->abort();
 	m_pTcpSocket->close();
}

void WorkThread::startTcpConn()
{
	if (m_pTcpSocket == Q_NULLPTR)
	{
		m_pTcpSocket = new QTcpSocket;
		connect(m_pTcpSocket, &QTcpSocket::readyRead, this, &WorkThread::readMessage, Qt::DirectConnection);
		connect(m_pTcpSocket, &QTcpSocket::stateChanged, this, &WorkThread::onStateChanged, Qt::DirectConnection);
		connect(m_pTcpSocket, SIGNAL(error(QAbstractSocket::SocketError)),
			this, SLOT(displayError(QAbstractSocket::SocketError)), Qt::DirectConnection);

		m_packet = new DtStreamPacket;
		connect(m_packet, &DtStreamPacket::onDataChanged, this, &WorkThread::OnDataChanged, Qt::DirectConnection);
	}
	m_pTcpSocket->connectToHost(m_sIpServer, m_nPort);
}

void WorkThread::readMessage()
{
	//QMutexLocker locker(&m_dataMutex);
	if (m_nCounter > 0)
	{
		while (true)
		{
			ByteBuffer packet_header_buffer(DT_PACKET_HEADER_SIZE, 0);
			auto bc = m_pTcpSocket->read(packet_header_buffer.data(), DT_PACKET_HEADER_SIZE);
			if (DT_PACKET_HEADER_SIZE != bc)
			{
				//qDebug() << ("Could not read header");
				break;
			}
			if (m_packet->processPacketHeader(packet_header_buffer) < 0)
			{
				LOG(INFO) << "Could not process packet header";
				continue;
			}
			int32_t packet_size = m_packet->getPacketSize() - DT_PACKET_HEADER_SIZE;
			ByteBuffer packet_buffer(packet_size, 0);
			bc = m_pTcpSocket->read(packet_buffer.data(), packet_size);
			if (packet_size != bc)
			{
				//qDebug() << ("Could not read all packet data");
				break;
			}
			if (m_packet->processSubPackets(packet_buffer) < 0)
			{
				LOG(INFO) << "Could not process packet";
				continue;
			}
			QByteArray data;
			data.append(QString::fromStdString(m_packet->getPacketInfo()));
			m_packet->printChannelSamples();
			m_packet->clearChannels();
			if (m_packet->isLastPacket()) break;
		}
	}
	else
	{
		QByteArray message = m_pTcpSocket->read(30);
		emit receiveMessage(message);
	}
	m_nCounter++;
}

void WorkThread::displayError(QAbstractSocket::SocketError)
{
	LOG(INFO) << "Error: " << _QLOG(m_pTcpSocket->errorString());
}

void WorkThread::onStateChanged(QAbstractSocket::SocketState socketState)
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

void WorkThread::OnDataChanged(int idx, QString value)
{
	QMutexLocker locker(&m_mutex);
	int index = 0;
	for (auto &channelInfo : m_channelInfoList)
	{
		if (channelInfo.isChecked)
		{
			if (index == idx)
			{
				channelInfo.lValue = value.toFloat();
			}
			index++;
		}
		else
		{
			channelInfo.lValue = 0.0f;
		}
	}
}
