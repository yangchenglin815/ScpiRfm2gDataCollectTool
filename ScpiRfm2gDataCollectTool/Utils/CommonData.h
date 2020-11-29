#pragma once

#include "CommEnum.h"

#define MAX_CHANNEL_NUM  28;         //Ĭ��28·�ź�
#define DEF_SENDCYCLE_TIME  200;     //Ĭ�Ϸ�������200ms

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
