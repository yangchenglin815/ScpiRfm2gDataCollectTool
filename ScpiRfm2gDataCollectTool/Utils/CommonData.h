#pragma once

#include "CommEnum.h"

#define MAX_CHANNEL_NUM  28;         //默认28路信号
#define DEF_SENDCYCLE_TIME  200;     //默认发送周期200ms

typedef struct _ChannelInfo
{
	int nId;
	QString sChannelName;
	float lValue;
	bool isChecked;
	_ChannelInfo()
	{
		nId = 0;
		lValue = 0.00;
		isChecked = false;
	}
}ChannelInfo;
Q_DECLARE_METATYPE(_ChannelInfo)
