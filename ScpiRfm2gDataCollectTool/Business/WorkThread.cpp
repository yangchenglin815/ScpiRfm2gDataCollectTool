#include "WorkThread.h"
#include "Log/GlogManager.h"
#include "CommEnum.h"
#include "ConstData.h"
#include "UserConfig.h"

#define MODE2

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
#ifdef MODE2
	m_recvBuffer.clear();
#else
	m_nCounter = 0;
#endif 
	m_pTcpSocket->connectToHost(m_sIpServer, m_nPort);
}

void WorkThread::readMessage()
{
#ifdef MODE2
	m_recvBuffer += m_pTcpSocket->readAll();
	while (m_recvBuffer.contains("OXYGEN<<") && m_recvBuffer.contains(">>OXYGEN"))
	{
		int begin = m_recvBuffer.indexOf("OXYGEN<<");
		int end   = m_recvBuffer.indexOf(">>OXYGEN");
		QByteArray tempBuffer = m_recvBuffer.left(end + 8);
		tempBuffer = tempBuffer.mid(begin);
		//LOG(INFO)<<"tempBuffer: " << tempBuffer.toHex().data();
		//LOG(INFO) << "begin: " << begin << ", end: " << end;
		ByteBuffer packet_header_buffer(DT_PACKET_HEADER_SIZE, 0);
		for (int i = 0; i < DT_PACKET_HEADER_SIZE; ++i)
		{
			packet_header_buffer[i] = tempBuffer[i];
		}
		//LOG(INFO) << "Packet header: " << packet_header_buffer.data();
		if (m_packet->processPacketHeader(packet_header_buffer) < 0)
		{
			LOG(INFO) << "Could not process packet header.";
			continue;
		}
		int32_t packet_size = m_packet->getPacketSize() - DT_PACKET_HEADER_SIZE;
		ByteBuffer packet_buffer(packet_size, 0);
		if (packet_size != tempBuffer.length() - DT_PACKET_HEADER_SIZE)
		{
			LOG(INFO) << "Could not read all packet data.";
			continue;
		}
		for (int i = 0; i < packet_size; ++i)
		{
			packet_buffer[i] = tempBuffer[i + DT_PACKET_HEADER_SIZE];
		}
		if (m_packet->processSubPackets(packet_buffer) < 0)
		{
			LOG(INFO) << "Could not process packet.";
			continue;
		}
		QByteArray data;
		data.append(QString::fromStdString(m_packet->getPacketInfo()));
		int dataDebug = UserConfig::getInstance()->readSetting("RFM2G", "DATADEBUG").toInt();
		if (dataDebug == -1)
			dataDebug = 0;
		if (dataDebug)
			LOG(INFO) << data.data();
		
		m_packet->printChannelSamples();
		m_packet->clearChannels();
		m_recvBuffer = m_recvBuffer.mid(end + 8);

		if (m_packet->isLastPacket())
		{
			m_recvBuffer.clear();
			break;
		}
	}
#else
	if (m_nCounter > 0)
	{
		ByteBuffer packet_header_buffer(DT_PACKET_HEADER_SIZE, 0);
		auto bc = m_pTcpSocket->read(packet_header_buffer.data(), DT_PACKET_HEADER_SIZE);
		if (DT_PACKET_HEADER_SIZE != bc)
		{
			LOG(INFO) << "Could not read header.";
			return;
		}
		QByteArray array(packet_header_buffer.data());
		LOG(INFO) << "Packet header: " << array.toHex().data() << ", size: " << array.length();
		LOG(INFO) << "Packet header: " << packet_header_buffer.data();
		if (m_packet->processPacketHeader(packet_header_buffer) < 0)
		{
			LOG(INFO) << "Could not process packet header.";
			return;
		}
		int32_t packet_size = m_packet->getPacketSize() - DT_PACKET_HEADER_SIZE;
		ByteBuffer packet_buffer(packet_size, 0);
		bc = m_pTcpSocket->read(packet_buffer.data(), packet_size);
		if (packet_size != bc)
		{
			LOG(INFO) << "Could not read all packet data.";
			return;
		}
		QByteArray arrayNew(packet_buffer.data());
		LOG(INFO) << "Packet body: " << arrayNew.toHex().data() << ", size: " << arrayNew.length();
		if (m_packet->processSubPackets(packet_buffer) < 0)
		{
			LOG(INFO) << "Could not process packet.";
			return;
		}
		QByteArray data;
		data.append(QString::fromStdString(m_packet->getPacketInfo()));
		m_packet->printChannelSamples();
		m_packet->clearChannels();
	}
	else
	{
		QByteArray message = m_pTcpSocket->read(30);
		emit receiveMessage(message);
	}
	m_nCounter++;
#endif 
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
