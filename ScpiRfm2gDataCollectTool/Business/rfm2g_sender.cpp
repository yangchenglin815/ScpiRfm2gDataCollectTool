#include "rfm2g_sender.h"
#include "UserConfig.h"
#include "Log/GlogManager.h"

CRfm2gSender::CRfm2gSender()
{
	m_CRfm2gHandle = 0;
	m_iDeviceNodeId = 1;
	m_isDebug = UserConfig::getInstance()->readSetting("RFM2G", "DEBUG").toInt();
	LOG(INFO) << "m_isDebug: " << m_isDebug;
	bool ok;
	m_iOffset = UserConfig::getInstance()->readSetting("OTHRCFG", "OFFSET").toLong(&ok, 16);
	LOG(INFO) << "m_iOffset: " << m_iOffset;
}

CRfm2gSender::~CRfm2gSender()
{
	if (m_CRfm2gHandle != 0)
	{
		RFM2gClose(&m_CRfm2gHandle);
	}
}

CRfm2gSender* CRfm2gSender::getInstance()
{
	static CRfm2gSender instance;
	return &instance;
}

RFM2GHANDLE CRfm2gSender::Rfm2gOpen()
{
	m_iDeviceNodeId = INSTANCE_USER_CONFIG->readSetting("RFM2G", "NODEID").toInt();
	LOG(INFO) << "m_iDeviceNodeId: " << m_iDeviceNodeId;

	char   device[64];          /* Name of PCI RFM2G device to use   */
	RFM2G_STATUS   result;                 /* Return codes from RFM2g API calls */

	sscanf(device, "%d", &m_iDeviceNodeId);
	/* if sscanf fails, then numDevice will stay 0 */
	sprintf(device, "%s%d", DEVICE_PREFIX, m_iDeviceNodeId);
	LOG(INFO) << "device " << DEVICE_PREFIX << m_iDeviceNodeId;
	QString dev = QString("%1%2").arg(DEVICE_PREFIX).arg(m_iDeviceNodeId);
	LOG(INFO) << "device " << _QLOG(dev);

	/* Open the Reflective Memory device */
	result = RFM2gOpen(dev.toLocal8Bit().data(), &m_CRfm2gHandle);
	if (result != RFM2G_SUCCESS)
	{
		LOG(INFO) << "ERROR: RFM2gOpen() failed.";
		LOG(INFO) << "Error: " << RFM2gErrorMsg(result);
		return 0;
	}
	return m_CRfm2gHandle;
}

bool CRfm2gSender::Rfm2g_Write(const QList<ChannelInfo> &channelInfoList)
{
	RFM2G_STATUS   result;  /* Return codes from RFM2g API calls */
	if (m_isDebug)
	   LOG(INFO) << "PCI RFM2g Write.";
	QByteArray header;
	header.resize(4);
	header[0] = (char)0x0E;
	header[1] = (char)0x01;
	header[2] = (char)0x00;
	header[3] = (char)0x01;
	if (m_isDebug)
	   LOG(INFO) << "Packet Header: " << _QLOG(QString(header.toHex()));

	QByteArray body;
	body.resize(4 * channelInfoList.size() + 1);
	body[0] = (char)0x01;
	int index = 0;
	for (auto &channelInfo : channelInfoList)
	{
		if (m_isDebug)
		   LOG(INFO) << "Rfm2g_Write: " << channelInfo.lValue;
		unsigned char *hex = (unsigned char*)&channelInfo.lValue;
		for (int i = 0; i < 4; ++i)
		{
			body[(index * 4) + (i + 1)] = hex[i];
		}
		index++;
	}

	QByteArray packet = header + body;
	if (m_isDebug)
	    LOG(INFO) << "Write Packet: " << packet.toHex().data();
	result = RFM2gWrite(m_CRfm2gHandle, m_iOffset, (void *)packet.data(), 4 * 29 + 1);
	if (result == RFM2G_SUCCESS)
	{
		if (m_isDebug)
		   LOG(INFO) << "The data was written to Reflective Memory.";
	}
	else
	{
		LOG(INFO) << "ERROR: Could not write data to Reflective Memory.";
		LOG(INFO) << "Error: " << RFM2gErrorMsg(result);
		return false;
	}
	return true;
       


	RFM2G_UINT32   outbuffer[BUFFER_SIZE]; /* Data written to another node      */
	RFM2G_UINT32   inbuffer[BUFFER_SIZE];  /* Data read from another node       */

	/* Initialize the data buffers */
	for (int i = 0; i < BUFFER_SIZE; i++)
	{
		outbuffer[i] = 0xa5a50000 + i;  /* Any fake data will do */
		inbuffer[i] = 0;
	}

	/* Display contents of the outbuffer */
	printf("\nThis buffer will be written to Reflective Memory starting"
		" at offset 0x%X:\n", OFFSET_1);
	LOG(INFO) << "This buffer will be written to Reflective Memory starting at offset " << OFFSET_1;
	for (int i = 0; i < BUFFER_SIZE; i++)
	{
		printf("Offset: 0x%X\tData: 0x%08X\n", OFFSET_1 + i * 4, outbuffer[i]);
		LOG(INFO) << "Offset: " << OFFSET_1 + i * 4 << ", Data: " << outbuffer[i];
	}

	/* Write outbuffer into Reflective Memory starting at OFFSET_1 */
	result = RFM2gWrite(m_CRfm2gHandle, OFFSET_1, (void *)outbuffer, BUFFER_SIZE * 4);
	if (result == RFM2G_SUCCESS)
	{
		LOG(INFO) << "The data was written to Reflective Memory.";
	}
	else
	{
		LOG(INFO) << "ERROR: Could not write data to Reflective Memory.";
		//RFM2gClose(&m_CRfm2gHandle);
		return false;
	}
	return true;
}


bool CRfm2gSender::Rfm2g_Read(QByteArray &data)
{
	LOG(INFO) << "Rfm2g_Read...";
	RFM2G_STATUS   result;                 /* Return codes from RFM2g API calls */
	result = RFM2gRead(m_CRfm2gHandle, m_iOffset, (void *)data.data(), 9);
	if (result != RFM2G_SUCCESS)
	{
		LOG(INFO) << "ERROR: Could not read data from Reflective Memory.";
		//RFM2gClose(&m_CRfm2gHandle);
		return false;
	}

	LOG(INFO) << "Rfm2g_Read: " << data.toHex().data();
	return true;


	RFM2G_UINT32   outbuffer[BUFFER_SIZE]; /* Data written to another node      */
	RFM2G_UINT32   inbuffer[BUFFER_SIZE];  /* Data read from another node       */

	//RFM2G_NODE     otherNodeId;            /* Node ID of the other RFM board    */
	//RFM2G_CHAR     string[40];             /* User input                        */
	//RFM2GEVENTINFO EventInfo;              /* Info about received interrupts    */
	//char     device[40];                  /* Name of PCI RFM2G device to use   */
    //RFM2G_INT32    numDevice = 0;
	//RFM2G_CHAR     selection[10];

	LOG(INFO) << "PCI RFM2g Read.";

	
	/* Initialize the data buffers */
	for (int i = 0; i < BUFFER_SIZE; i++)
	{
		outbuffer[i] = 0xa5a50000 + i;  /* Any fake data will do */
		inbuffer[i] = 0;
	}

	/* Display contents of the outbuffer */
	LOG(INFO) << "This buffer will be written to Reflective Memory starting at offset " << OFFSET_1;
	printf("\nThis buffer will be written to Reflective Memory starting"
		" at offset 0x%X:\n", OFFSET_1);
	for (int i = 0; i < BUFFER_SIZE; i++)
	{
		printf("Offset: 0x%X\tData: 0x%08X\n", OFFSET_1 + i * 4, outbuffer[i]);
		LOG(INFO) << "Offset: " << OFFSET_1 + i * 4 << ", Data: " << outbuffer[i];
	}

	/* Got the interrupt, now read data from the other board from OFFSET_2 */
	result = RFM2gRead(m_CRfm2gHandle, OFFSET_2, (void *)inbuffer, BUFFER_SIZE * 4);
	if (result != RFM2G_SUCCESS)
	{
		LOG(INFO) << "ERROR: Could not read data from Reflective Memory.";
		//RFM2gClose(&m_CRfm2gHandle);
		return false;
	}

	/* Display contents of the inbuffer */
	printf("\nThis buffer was read from Reflective Memory starting"
		" at offset 0x%X:\n", OFFSET_2);
	LOG(INFO) << "This buffer was read from Reflective Memory starting at offset " << OFFSET_2;;
	for (int i = 0; i < BUFFER_SIZE; i++)
	{
		printf("Offset: 0x%X\tData: 0x%08X\n", OFFSET_2 + i * 4, inbuffer[i]);
		LOG(INFO) << "Offset: " << OFFSET_2 + i * 4 << ", Data: " << inbuffer[i];
	}

	LOG(INFO) << "Success!";
	return true;
}

bool CRfm2gSender::Rfm2g_sender()
{   
	bool bRet = false;

	RFM2G_STATUS   result;                 /* Return codes from RFM2g API calls */
	RFM2G_UINT32   outbuffer[BUFFER_SIZE]; /* Data written to another node      */
	RFM2G_UINT32   inbuffer[BUFFER_SIZE];  /* Data read from another node       */
	//RFM2G_INT32    i;                      /* Loop variable                     */
	RFM2G_NODE     otherNodeId;            /* Node ID of the other RFM board    */
	RFM2G_CHAR     string[40];             /* User input                        */
	RFM2GEVENTINFO EventInfo;              /* Info about received interrupts    */
	char     device[40];                  /* Name of PCI RFM2G device to use   */
	RFM2G_INT32    numDevice = 0;
	RFM2G_CHAR     selection[10];

	//RFM2GHANDLE    Handle = 0;

	LOG(INFO) << "PCI RFM2g Sender.";

	/*
	printf("Please enter device number: ");
	while ((fgets(device, sizeof(device), stdin) == (char *)NULL) || (strlen(device) < 2))
	{
	}
	*/
	// 
	sscanf(device, "%d", &numDevice);
	/* if sscanf fails, then numDevice will stay 0 */
	sprintf(device, "%s%d", DEVICE_PREFIX, numDevice);

	/* Open the Reflective Memory device */
	result = RFM2gOpen(device, &m_CRfm2gHandle);
	if (result != RFM2G_SUCCESS)
	{
		LOG(INFO) << "ERROR: RFM2gOpen() failed.";
		LOG(INFO) << "Error: " << RFM2gErrorMsg(result);
		return false;
	}

	/* Get the other Reflective Memory board's node ID */
	/*
	printf("\nWhat is the Reflective Memory Node ID of the computer running\n"
		"the \"RFM2G_receiver\" program?  ");
	if (fgets(string, sizeof(string), stdin) == (char *)NULL)
	{
		printf("ERROR: Could not get user input.\n");
		RFM2gClose(&Handle);
		return(-1);
	}
#if (defined(WIN32))
	otherNodeId = (RFM2G_INT16)strtol(string, 0, 0);
#else
	otherNodeId = strtol(string, 0, 0);
#endif
	*/
	/* Prompt user to start the RFM2G_receiver program */
	/*
	printf("\nStart the \"RFM2G_receiver\" program on the other computer.\n");
	printf("  Press RETURN to continue ...  ");
	getchar();
	printf("\n");

	result = RFM2gEnableEvent(Handle, RFM2GEVENT_INTR2);
	if (result != RFM2G_SUCCESS)
	{
		printf("Error: %s\n", RFM2gErrorMsg(result));
		RFM2gClose(&Handle);
		return(result);
	}
	*/

	/* Initialize the data buffers */
	for (int i = 0; i < BUFFER_SIZE; i++)
	{
		outbuffer[i] = 0xa5a50000 + i;  /* Any fake data will do */
		inbuffer[i] = 0;
	}

		
	/* Display contents of the outbuffer */
	printf("\nThis buffer will be written to Reflective Memory starting"
				" at offset 0x%X:\n", OFFSET_1);
	LOG(INFO) << "This buffer will be written to Reflective Memory starting at offset " << OFFSET_1;
	for (int i = 0; i<BUFFER_SIZE; i++)
	{
		printf("Offset: 0x%X\tData: 0x%08X\n", OFFSET_1 + i * 4, outbuffer[i]);
		LOG(INFO) << "Offset: " << OFFSET_1 + i * 4 << ", Data: " << outbuffer[i];
	}
		
	/* Write outbuffer into Reflective Memory starting at OFFSET_1 */
	result = RFM2gWrite(m_CRfm2gHandle, OFFSET_1, (void *)outbuffer, BUFFER_SIZE * 4);
	if (result == RFM2G_SUCCESS)
	{
		LOG(INFO) << "The data was written to Reflective Memory.";
	}
	else
	{
		LOG(INFO) << "ERROR: Could not write data to Reflective Memory.";
		RFM2gClose(&m_CRfm2gHandle);
		return false;
	}

		/* Send an interrupt to the other Reflective Memory board */
	   /*
		result = RFM2gSendEvent(Handle, otherNodeId, RFM2GEVENT_INTR1, 0);
		if (result == RFM2G_SUCCESS)
		{
			printf("An interrupt was sent to Node %d.\n", otherNodeId);
		}
		else
		{
			printf("Error: %s\n", RFM2gErrorMsg(result));
			RFM2gClose(&Handle);
			return(-1);
		}
		*/
		/* Now wait on an interrupt from the other Reflective Memory board */
	  /*
	printf("\nWaiting %d seconds for an interrupt from Node %d ...  ",
			TIMEOUT / 1000, otherNodeId);
	fflush(stdout);
	EventInfo.Event = RFM2GEVENT_INTR2;  // We'll wait on this interrupt 
	EventInfo.Timeout = TIMEOUT;       // We'll wait this many milliseconds 

	result = RFM2gWaitForEvent(Handle, &EventInfo);
	if (result == RFM2G_SUCCESS)
	{
		printf("Received the interrupt from Node %d.\n", EventInfo.NodeId);
	}
	else
	{
		printf("\nTimed out waiting for the interrupt.\n");
		RFM2gClose(&Handle);
		return(-1);
	}

	/* Got the interrupt, now read data from the other board from OFFSET_2 */
	result = RFM2gRead(m_CRfm2gHandle, OFFSET_2, (void *)inbuffer, BUFFER_SIZE * 4);
	if (result != RFM2G_SUCCESS)
	{
		LOG(INFO) << "ERROR: Could not read data from Reflective Memory.";
		RFM2gClose(&m_CRfm2gHandle);
		return false;
	}

	
	/* Display contents of the inbuffer */
	printf("\nThis buffer was read from Reflective Memory starting"
				" at offset 0x%X:\n", OFFSET_2);
	for (int i = 0; i<BUFFER_SIZE; i++)
	{
		printf("Offset: 0x%X\tData: 0x%08X\n", OFFSET_2 + i * 4, inbuffer[i]);
	}
	
	LOG(INFO) << "Success!";

	/* Close the Reflective Memory device */
	RFM2gClose(&m_CRfm2gHandle);

	return bRet;
}

bool CRfm2gSender::Rfm2g_close()
{
	RFM2G_STATUS result = RFM2gClose(&m_CRfm2gHandle);
	if (result != RFM2G_SUCCESS)
	{
		LOG(INFO) << "ERROR: RFM2gClose() failed.";
		LOG(INFO) << "Error: " << RFM2gErrorMsg(result);
		return false;
	}
	LOG(INFO) << "RFM2gClose() success.";
	return true;
}
