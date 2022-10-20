#include "ScpiRfm2gDataCollectTool.h"
#include "CommonData.h"
#include "UserConfig.h"
#include "CommEnum.h"
#include "ConstData.h"
#include "Log/GlogManager.h"
#include "rfm2g_sender.h"
#include <QCheckBox>
#include <QDateTime>
#include <QDebug>
#include <QtConcurrent>
#include <QMessageBox>
#include <QFlags>

ScpiRfm2gDataCollectTool::ScpiRfm2gDataCollectTool(QWidget *parent)
	: QMainWindow(parent)
	, m_iTotalChannl(0)
	, m_address("127.0.0.1")
	, m_Reciveport(5555)
	, m_SendPort(10001)
	, m_defGroupId(2)
	, m_Rfm2gId(1)
	, m_pWorkThread(Q_NULLPTR)
{
	ui.setupUi(this);
	setWindowIcon(QIcon(QString(":/ScpiRfm2gDataCollectTool/Resources/desktop.png")));
	setWindowTitle(QStringLiteral("试验数据采集传输终端"));

	InitTable();
	InitUI();
	InitNetwork();
	InitConnect();
	AppendText(QStringLiteral("初始化完成..."));
}

ScpiRfm2gDataCollectTool::~ScpiRfm2gDataCollectTool()
{

}

void ScpiRfm2gDataCollectTool::closeEvent(QCloseEvent *event)
{
	if (m_pTimer->isActive())
	{
		m_pTimer->stop();
	}
	if (m_pUpdateTimer->isActive())
	{
		m_pUpdateTimer->stop();
	}
	if (m_pWorkThread)
	{
		m_pWorkThread->quit();
		m_pWorkThread->wait();
	}
	//INSTANCE_CRFM2G_SENDER->getInstance()->Rfm2g_close();
	m_pTcpClient->close();
	m_pUdpClient->Udp_close();
	SaveIni();
}

void ScpiRfm2gDataCollectTool::OnConnectClicked()
{
	if (ui.comboBox_bindIp->currentIndex() == -1)
	{
		AppendText(QStringLiteral("请先绑定网卡及IP..."));
		QMessageBox::about(this, QStringLiteral("提示"), QStringLiteral("请先绑定网卡及IP"));
		return;
	}

	if (ui.lineEdit_timeInterval->text().isEmpty())
	{
		AppendText(QStringLiteral("请先设置发送周期..."));
		QMessageBox::about(this, QStringLiteral("提示"), QStringLiteral("请先设置发送周期"));
		return;
	}

	int rows = ui.tableWidget->rowCount();
	for (int row = 0; row < rows; row++)
	{
		QTableWidgetItem *item = ui.tableWidget->item(row, 0);
		if (item->isSelected())
		{
			m_channelInfoList[row].isChecked = true;
		}
		else
		{
			m_channelInfoList[row].isChecked = false;
		}
	}

	QStringList items;
	for (auto &channelInfo : m_channelInfoList)
	{
		QString channelName = QStringLiteral("\"%1\"").arg(channelInfo.sChannelName);
		if (channelInfo.isChecked)
		{
			items.append(channelName);
		}
	}
	if (items.isEmpty())
	{
		AppendText(QStringLiteral("未找到通道..."));
		AppendText(QStringLiteral("配置失败..."));
		QMessageBox::about(this, QStringLiteral("提示"), QStringLiteral("请选择通道"));
		ui.label_status->setStyleSheet("background: gray; border-radius: 12px;");
		return;
	}

	OnStopClicked();
	if (m_pTimer->isActive())
	{
		m_pTimer->stop();
		m_pTimer->setInterval(ui.lineEdit_timeInterval->text().toInt());
	}
	m_pTcpClient->close();
	m_pUdpClient->Udp_close();

	AppendText(QStringLiteral("正在配置Udp管道..."));
	m_pUdpClient->Udp_start(ui.comboBox_bindIp->currentText());

	AppendText(QStringLiteral("正在配置网络..."));
	m_pTcpClient->newConnect(m_address, m_SendPort);
	AppendText(QStringLiteral("获取插件版本号..."));
	m_pTcpClient->sendMessage(SCPI_QUERY_VERSON);
	AppendText(QStringLiteral("重置插件..."));
	m_pTcpClient->sendMessage(SCPI_RESET_CMD);

	QString group = INSTANCE_USER_CONFIG->readSetting("COMMUNCTION", "DEFGROUP");
	QString subPort = INSTANCE_USER_CONFIG->readSetting("COMMUNCTION", "RECVPORT");
	QString str = QStringLiteral("%1 %2").arg(group).arg(subPort);
	QByteArray cmd;
	cmd.append(str);
	AppendText(QStringLiteral("设置通信端口..."));
	m_pTcpClient->sendMessage(SCPI_SET_PORT + cmd);
	str = QStringLiteral("%1?").arg(group);
	cmd.clear();
	cmd.append(str);
	m_pTcpClient->sendMessage(SCPI_SET_PORT + cmd);

	str = QStringLiteral("%1 %2").arg(group).arg(items.join(","));
	cmd.clear();
	cmd.append(str);
	AppendText(QStringLiteral("设置通道分组..."));
	m_pTcpClient->sendMessage(SCPI_SET_CHANNEL + cmd);
	str = QStringLiteral("%1?").arg(group);
	cmd.clear();
	cmd.append(str);
	m_pTcpClient->sendMessage(SCPI_SET_CHANNEL + cmd);

	str = QStringLiteral(" %1").arg(group);
	cmd.clear();
	cmd.append(str);
	AppendText(QStringLiteral("初始化分组..."));
	m_pTcpClient->sendMessage(SCPI_INIT_CMD + cmd);
	str = QStringLiteral("%1?").arg(group);
	cmd.clear();
	cmd.append(str);
	m_pTcpClient->sendMessage(SCPI_DSP_STATE + cmd);

	str = QStringLiteral(" %1").arg(group);
	cmd.clear();
	cmd.append(str);
	AppendText(QStringLiteral("打开分组..."));
	m_pTcpClient->sendMessage(SCPI_START_CMD + cmd);
	str = QStringLiteral("%1?").arg(group);
	cmd.clear();
	cmd.append(str);
	m_pTcpClient->sendMessage(SCPI_DSP_STATE + cmd);
	AppendText(QStringLiteral("配置成功..."));
	ui.label_status->setStyleSheet("background: yellow; border-radius: 12px;");

	if (m_pWorkThread)
	{
		m_pWorkThread->setChannelInfo(m_channelInfoList);
	}
}

void ScpiRfm2gDataCollectTool::OnStartClicked()
{
	if (ui.comboBox_bindIp->currentIndex() == -1)
	{
		AppendText(QStringLiteral("请先绑定网卡及IP..."));
		QMessageBox::about(this, QStringLiteral("提示"), QStringLiteral("请先绑定网卡及IP"));
		return;
	}

	if (ui.lineEdit_timeInterval->text().isEmpty())
	{
		AppendText(QStringLiteral("请先设置发送周期..."));
		QMessageBox::about(this, QStringLiteral("提示"), QStringLiteral("请先设置发送周期"));
		return;
	}

	bool isHave = false;
	for (auto &channelInfo : m_channelInfoList)
	{
		if (channelInfo.isChecked)
		{
			isHave = true;
			break;
		}
	}
	if (!isHave)
	{
		AppendText(QStringLiteral("未找到通道..."));
		AppendText(QStringLiteral("请选择通道..."));
		QMessageBox::about(this, QStringLiteral("提示"), QStringLiteral("请选择通道"));
		return;
	}

	//INSTANCE_CRFM2G_SENDER->getInstance()->Rfm2gOpen();
	m_pUpdateTimer->start();
	m_pTimer->start();
	if (m_pWorkThread == Q_NULLPTR)
	{
		m_pWorkThread = new WorkThread(m_channelInfoList, m_address, m_Reciveport, this);
		connect(m_pWorkThread, &WorkThread::networkStatusChanged, this, &ScpiRfm2gDataCollectTool::OnNetworkStatusChanged);
		connect(m_pWorkThread, &WorkThread::receiveMessage, this, &ScpiRfm2gDataCollectTool::OnReceiveMessage);
		//connect(m_pWorkThread, &WorkThread::onDataChanged, this, &ScpiRfm2gDataCollectTool::OnDataChanged);
	}
	m_pWorkThread->start();
	return;
}

void ScpiRfm2gDataCollectTool::OnStopClicked()
{
	if (m_pTimer->isActive())
	{
		m_pTimer->stop();
	}
	if (m_pUpdateTimer->isActive())
	{
		m_pUpdateTimer->stop();
	}
	if (m_pWorkThread)
	{
		m_pWorkThread->quit();
	}
	//INSTANCE_CRFM2G_SENDER->getInstance()->Rfm2g_close();
}

void ScpiRfm2gDataCollectTool::OnExitClicked()
{
	this->close();
}

void ScpiRfm2gDataCollectTool::OnNetworkStatusChanged(int socketState)
{
	switch (socketState)
	{
	case SOCKET_CONNECTED:
		AppendText(QStringLiteral("网络连接成功..."));
		ui.label_status->setStyleSheet("background: green; border-radius: 12px;");
		break;
	case SOCKET_CONNECTING:
		ui.label_status->setStyleSheet("background: yellow; border-radius: 12px;");
		break;
	default:
		AppendText(QStringLiteral("网络已断开..."));
		ui.label_status->setStyleSheet("background: red; border-radius: 12px;");
		break;
	}
}

void ScpiRfm2gDataCollectTool::OnReceiveMessage(QByteArray message)
{
	AppendText(QString(message).simplified());
}

void ScpiRfm2gDataCollectTool::OnTimeout()
{
	QtConcurrent::run([&]() {
		if (m_pWorkThread)
		{
			QList<ChannelInfo> channalInfoList = m_pWorkThread->GetChannelInfo();
			//INSTANCE_CRFM2G_SENDER->getInstance()->Rfm2g_Write(channalInfoList);
			m_pUdpClient->Udp_write(channalInfoList);
		}
	});
}

void ScpiRfm2gDataCollectTool::InitTable()
{
	//设置表头内容
	QStringList header;
	ui.tableWidget->setColumnCount(3); //设置列数
	header << QStringLiteral("序号") << QStringLiteral("通道名称") << QStringLiteral("通道值");
	ui.tableWidget->setHorizontalHeaderLabels(header);
	ui.tableWidget->horizontalHeader()->setHighlightSections(false);
	ui.tableWidget->verticalHeader()->setHighlightSections(false);

	ui.tableWidget->horizontalHeader()->setStretchLastSection(true);//设置表格是否充满，即行末不留空
	//QHeaderView* headerView = ui.tableWidget->horizontalHeader();
	//headerView->setSectionResizeMode(QHeaderView::Stretch);//设置tablewidget等宽

	//设置表头文字显示格式
	ui.tableWidget->horizontalHeader()->setDefaultAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

	ui.tableWidget->verticalHeader()->setDefaultSectionSize(30); //设置行高
	ui.tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);  //整行选中的方式
	ui.tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);   //禁止修改
	ui.tableWidget->setSelectionMode(QAbstractItemView::MultiSelection);

	ui.tableWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff); //去掉水平滚动条
	ui.tableWidget->setVerticalScrollMode(QAbstractItemView::ScrollPerItem);//垂直滚动条按项移动

	ui.tableWidget->setFrameShape(QFrame::NoFrame);
	ui.tableWidget->verticalHeader()->setVisible(false);
	ui.tableWidget->setFocusPolicy(Qt::NoFocus);

	ui.tableWidget->setColumnWidth(0, 100);
	ui.tableWidget->setColumnWidth(1, 300);
	ui.tableWidget->setColumnWidth(2, 100);
}

void ScpiRfm2gDataCollectTool::InitUI()
{
	ui.lineEdit_timeInterval->setValidator(new QRegExpValidator(QRegExp("[0-9]+$")));

	int iDefautChlNum = MAX_CHANNEL_NUM;
	m_iTotalChannl = INSTANCE_USER_CONFIG->readSetting("ALLCHLINFO", "COUNT").toInt();
	for (int index = 0; index < m_iTotalChannl; ++index)
	{
		ChannelInfo channelInfo;
		QString channlName = INSTANCE_USER_CONFIG->readSetting("ALLCHLINFO", QString("NAME%1").arg(index));
		channelInfo.nId = index;
		channelInfo.sChannelName = channlName;
		m_channelInfoList.append(channelInfo);

		ui.tableWidget->insertRow(index);
		QTableWidgetItem *itemNo = new QTableWidgetItem;
		itemNo->setTextAlignment(Qt::AlignCenter);
		itemNo->setText(QString::number(index + 1));
		itemNo->setCheckState(Qt::Unchecked);
		ui.tableWidget->setItem(index, 0, itemNo);

		QTableWidgetItem *itemName = new QTableWidgetItem;
		itemName->setTextAlignment(Qt::AlignCenter);
		itemName->setText(QStringLiteral("%1").arg(channlName));
		ui.tableWidget->setItem(index, 1, itemName);

		QTableWidgetItem *itemVal = new QTableWidgetItem;
		itemVal->setTextAlignment(Qt::AlignCenter);
		itemVal->setText("0.00");
		ui.tableWidget->setItem(index, 2, itemVal);
	}

	QString iSendCycTime = INSTANCE_USER_CONFIG->readSetting("OTHRCFG", "SNDCYCLE");
	if (iSendCycTime == "-1")
	{
		iSendCycTime = "200";
	}
	ui.lineEdit_timeInterval->setText(iSendCycTime);
	m_address = INSTANCE_USER_CONFIG->readSetting("COMMUNCTION", "RSERVERIP");
	m_SendPort = INSTANCE_USER_CONFIG->readSetting("COMMUNCTION", "SENDPORT").toInt();
	m_Reciveport = INSTANCE_USER_CONFIG->readSetting("COMMUNCTION", "RECVPORT").toInt();
	m_defGroupId = INSTANCE_USER_CONFIG->readSetting("COMMUNCTION", "DEFGROUP").toInt();
	m_Rfm2gId = INSTANCE_USER_CONFIG->readSetting("COMMUNCTION", "NODEID").toInt();

	QString currSelectedItem = INSTANCE_USER_CONFIG->readSetting("CURCHLINFO", "ITEM");
	if (currSelectedItem != "-1")
	{
		QStringList list = currSelectedItem.split("|");
		if (list.size() > 0)
		{
			for (auto &itemIndex : list)
			{
				int index = itemIndex.toInt();
				ui.tableWidget->selectRow(index);
				QTableWidgetItem *item = ui.tableWidget->item(index, 0);
				item->setCheckState(Qt::Checked);
				m_channelInfoList[index].isChecked = true;
			}
		}
	}
}

void ScpiRfm2gDataCollectTool::InitConnect()
{
	connect(ui.button_connect, &QPushButton::clicked, this, &ScpiRfm2gDataCollectTool::OnConnectClicked);
	connect(ui.button_start, &QPushButton::clicked, this, &ScpiRfm2gDataCollectTool::OnStartClicked);
	connect(ui.button_stop, &QPushButton::clicked, this, &ScpiRfm2gDataCollectTool::OnStopClicked);
	connect(ui.button_exit, &QPushButton::clicked, this, &ScpiRfm2gDataCollectTool::OnExitClicked);
	connect(ui.checkBox, &QCheckBox::clicked, this, [&](bool checked)
	{
		QList<QTableWidgetItem*> itemList = ui.tableWidget->selectedItems();
		for (auto &item : itemList)
		{
			item->setSelected(!checked);
		}

		int rows = ui.tableWidget->rowCount();
		for (int row = 0; row < rows; ++row)
		{
			ui.tableWidget->selectRow(row);
			QTableWidgetItem *item = ui.tableWidget->item(row, 0);
			item->setCheckState(ui.checkBox->checkState());
		}
	});

	connect(ui.tableWidget, &QTableWidget::cellClicked, this, [&](int row, int column)
	{
		QTableWidgetItem *item = ui.tableWidget->item(row, 0);
		int checkState = item->checkState();
		if (checkState == Qt::Unchecked)
		{
			item->setCheckState(Qt::Checked);
		}
		else
		{
			item->setCheckState(Qt::Unchecked);
		}
	});
}

void ScpiRfm2gDataCollectTool::InitNetwork()
{
	m_pTcpClient = new TcpClient(this);
	connect(m_pTcpClient, &TcpClient::receiveMessage, this, &ScpiRfm2gDataCollectTool::OnReceiveMessage);

	m_pUpdateTimer = new QTimer(this);
	int timeout = INSTANCE_USER_CONFIG->readSetting("OTHRCFG", "UPDATEUI").toInt();
	if (timeout == -1)
	{
		timeout = 500;
	}
	m_pUpdateTimer->setInterval(timeout);
	connect(m_pUpdateTimer, &QTimer::timeout, this, [&](){
		if (m_pWorkThread)
		{
			QList<ChannelInfo> channalInfoList = m_pWorkThread->GetChannelInfo();
			UpdateChannelInfo(channalInfoList);
		}
	});

	m_pTimer = new QTimer(this);
	m_pTimer->setInterval(ui.lineEdit_timeInterval->text().toInt());
	connect(m_pTimer, &QTimer::timeout, this, &ScpiRfm2gDataCollectTool::OnTimeout);

	m_pUdpClient = new UdpClient(this);
	QList<QNetworkInterface> hosts = m_pUdpClient->GetAllNetworkInterface();
	QStringList ipList;
	for (auto &host : hosts)
	{
		QFlags<QNetworkInterface::InterfaceFlag> flags = host.flags();
		if (flags.testFlag(QNetworkInterface::IsLoopBack))
			continue;
		if (flags.testFlag(QNetworkInterface::IsUp) && flags.testFlag(QNetworkInterface::IsRunning))
		{
			QList<QNetworkAddressEntry> entrys = host.addressEntries();
			for (auto &entry : entrys)
			{
				QHostAddress address = entry.ip();
				if (address.protocol() == QAbstractSocket::IPv4Protocol)
					ipList.append(address.toString());
			}
		}
	}
	if (!ipList.isEmpty())
	   ui.comboBox_bindIp->addItems(ipList);

	QString boardcast = INSTANCE_USER_CONFIG->readSetting("COMMUNCTION", "BOARDCAST");
	int index = ui.comboBox_bindIp->findText(boardcast);
	ui.comboBox_bindIp->setCurrentIndex(index);
}

void ScpiRfm2gDataCollectTool::SaveIni()
{
	QString timeInterval = ui.lineEdit_timeInterval->text();
	INSTANCE_USER_CONFIG->writeSetting("OTHRCFG", "SNDCYCLE", timeInterval);

	QString selectedItem;
	QStringList selectedList;
	QList<QTableWidgetItem*> itemList = ui.tableWidget->selectedItems();
	for (auto &item : itemList)
	{
		QString row = QString::number(ui.tableWidget->row(item));
		if (selectedList.indexOf(row) == -1)
		{
			selectedList.append(row);
		}
	}

	if (!selectedList.isEmpty())
	{
		selectedItem = selectedList.join("|");
	}
	INSTANCE_USER_CONFIG->writeSetting("CURCHLINFO", "ITEM", selectedItem);

	if (ui.comboBox_bindIp->currentIndex() != -1)
	{
		QString boardcast = ui.comboBox_bindIp->currentText();
		INSTANCE_USER_CONFIG->writeSetting("COMMUNCTION", "BOARDCAST", boardcast);
	}
}

void ScpiRfm2gDataCollectTool::AppendText(QString text)
{
	QString dateTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
	ui.textEdit->append(QStringLiteral("[%1]:  %2").arg(dateTime).arg(text));
	LOG(INFO) << _QLOG(QStringLiteral("%1").arg(text));
}

void ScpiRfm2gDataCollectTool::UpdateChannelInfo(const QList<ChannelInfo>& channelInfoList)
{
	for (auto &channelInfo : channelInfoList)
	{
		QTableWidgetItem *item = ui.tableWidget->item(channelInfo.nId, 2);
		if (item)
		{
			item->setText(QString::number(channelInfo.lValue));
		}
	}
}
