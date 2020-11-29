#ifndef _RFM2G_SENDER_H_
#define _RFM2G_SENDER_H_

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <QObject>
#include "rfm2g_api.h"
#include "rfm2g_types.h"
#include "CommonData.h"

#if (defined(RFM2G_LINUX))

#ifdef CONFIG_DEVFS_FS
#define DEVICE_PREFIX   "/dev/rfm2g/"
#else
#define DEVICE_PREFIX   "/dev/rfm2g"
#endif

#define PROCFILE         "/proc/rfm2g"

#elif defined(RFM2G_VXWORKS)

#define DEVICE_PREFIX   "RFM2G_"

#elif defined(SOLARIS)

#define DEVICE_PREFIX   "/dev/rfm2g"

#elif defined(WIN32)
#define DEVICE_PREFIX   "\\\\.\\rfm2g"
#else
#error Please define DEVICE_PREFIX for your driver
#endif

#define BUFFER_SIZE     256
#define OFFSET_1        0x00000000
#define OFFSET_2        0x2000

#define TIMEOUT         60000

#define INSTANCE_CRFM2G_SENDER CRfm2gSender::getInstance()

class CRfm2gSender : public QObject
{
public:
	static CRfm2gSender* getInstance();
	RFM2GHANDLE Rfm2gOpen();
	bool        Rfm2g_Write(const QList<ChannelInfo> &channelInfoList);
	bool        Rfm2g_Read(QByteArray &data);
	bool        Rfm2g_sender();
	bool        Rfm2g_close();

private:
	CRfm2gSender();
	virtual ~CRfm2gSender();

private:
	RFM2GHANDLE    m_CRfm2gHandle;
	RFM2G_INT32    m_iDeviceNodeId;
	RFM2G_INT32    m_iOffset;
	bool m_isDebug = false;
};

#endif