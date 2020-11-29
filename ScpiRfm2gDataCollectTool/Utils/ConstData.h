#pragma once

#include <QString>

const QByteArray SCPI_QUERY_VERSON = "*IDN?";
const QByteArray SCPI_RESET_CMD = ":DST:RESET";
const QByteArray SCPI_INIT_CMD = ":DST:INIT";
const QByteArray SCPI_START_CMD = ":DST:START";
const QByteArray SCPI_STOP_CMD = ":DST:STOP";
const QByteArray SCPI_DELETE_CMD = ":DST:DELETE";

const QByteArray SCPI_SET_PORT = ":DST:PORT";    //:DST:PORT 5555
const QByteArray SCPI_SET_CHANNEL = ":DST:ITEMs"; //:DST:ITEMs2 "AI 1/1", "AI 1/2"
const QByteArray SCPI_DSP_STATE = ":DST:STATe"; //:DST:STATe2？
const QByteArray DEF_GROUP_ID = "2";  //默认分组为2

const QByteArray ENDF = "\r\n";  //结束符