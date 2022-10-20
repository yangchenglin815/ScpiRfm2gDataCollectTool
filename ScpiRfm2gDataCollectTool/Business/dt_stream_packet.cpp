// Copyright DEWETRON GmbH 2018
#include "dt_stream_packet.h"
#include "crc32.h"
#include "pugixml.hpp"
#include "Log/GlogManager.h"
#include "UserConfig.h"
#include <cassert>
#include <algorithm>
#include <cstddef>
#include <cstring>
#include <sstream>
#include <iomanip>
#include <iostream>

std::string toString(int dt)
{
    switch(dt)
    {
    case dt_sint8:  return "dt_sint8";
    case dt_uint8:  return "dt_uint8";
    case dt_sint16: return "dt_sint16";
    case dt_uint16: return "dt_uint16";
    case dt_sint24: return "dt_sint24";
    case dt_uint24: return "dt_uint24";
    case dt_sint32: return "dt_sint32";
    case dt_uint32: return "dt_uint32";
    case dt_sint64: return "dt_sint64";
    case dt_uint64: return "dt_uint64";
    case dt_float:  return "dt_float";
    case dt_double: return "dt_double";
    case dt_complex_float:  return "dt_complex_float";
    case dt_complex_double: return "dt_complex_double";
    case dt_string: return "dt_string";
    case dt_binary: return "dt_binary";
    case dt_CAN:    return "dt_CAN";
    }

    return "dt_unknown_type";
}

std::size_t getDtDataTypeSize(int dt)
{
    switch(dt)
    {
    case dt_sint8:  return 1;
    case dt_uint8:  return 1;
    case dt_sint16: return 2;
    case dt_uint16: return 2;
    case dt_sint24: return 3;
    case dt_uint24: return 3;
    case dt_sint32: return 4;
    case dt_uint32: return 4;
    case dt_sint64: return 8;
    case dt_uint64: return 8;
    case dt_float:  return 4;
    case dt_double: return 8;
    case dt_complex_float:  return 8;
    case dt_complex_double: return 16;
    case dt_string: return -1;
    case dt_binary: return -1;
    case dt_CAN:    return 8;
    }

    return 0;
}


std::string toSampleValue(ByteBuffer::const_iterator& sample_it, int dt, const LinearScaling& sc)
{
    switch(dt)
    {
    case dt_sint8:  return std::to_string(static_cast<int8_t>(*sample_it) * sc.factor + sc.offset);
    case dt_uint8:  return std::to_string(static_cast<uint8_t>(*sample_it) * sc.factor + sc.offset);
    case dt_sint16: return std::to_string(*reinterpret_cast<const int16_t*>(&(*sample_it)) * sc.factor + sc.offset);
    case dt_uint16: return std::to_string(*reinterpret_cast<const uint16_t*>(&(*sample_it)) * sc.factor + sc.offset);
    case dt_sint24: {
                        // 24bit to 32bit conversion
                        int32_t v = 0;
                        memcpy(&v, &(*sample_it), 3);
                        if (v & 0x800000)
                        {
                            // sign extension
                            v |= 0xff000000;
                        }

                        return std::to_string(v * sc.factor + sc.offset);
                    }
    case dt_uint24: {
                        // 24bit to 32bit conversion
                        uint32_t v = 0;
                        memcpy(&v, &(*sample_it), 3);
                        return std::to_string(v * sc.factor + sc.offset);
                    }
    case dt_sint32: return std::to_string(*reinterpret_cast<const int32_t*>(&(*sample_it)) * sc.factor + sc.offset);
    case dt_uint32: return std::to_string(*reinterpret_cast<const uint32_t*>(&(*sample_it)) * sc.factor + sc.offset);
    case dt_sint64: return std::to_string(*reinterpret_cast<const int64_t*>(&(*sample_it)) * sc.factor + sc.offset);
    case dt_uint64: return std::to_string(*reinterpret_cast<const uint64_t*>(&(*sample_it)) * sc.factor + sc.offset);
    case dt_float:  return std::to_string(*reinterpret_cast<const float*>(&(*sample_it)) * sc.factor + sc.offset);
    case dt_double: return std::to_string(*reinterpret_cast<const double*>(&(*sample_it)) * sc.factor + sc.offset);
    case dt_complex_float:  return "";
    case dt_complex_double: return "";
    case dt_string: return "";
    case dt_binary: return "";
    case dt_CAN:    return "";
    }

    return 0;
}

std::string toSampleValueHex(ByteBuffer::const_iterator& sample_it, uint32_t data_size)
{
    const uint32_t max_bytes_to_show = 16;
    std::stringstream value;

    auto it = sample_it;
    for (uint32_t i = 0; i < std::min(max_bytes_to_show, data_size); ++i)
    {
        value << std::hex << *it << " ";
        ++it;
    }

    return value.str();
}

uint32_t getSampleSizeFromSampleIterator(ByteBuffer::const_iterator& sample_it)
{
    auto size = *reinterpret_cast<const uint32_t*>(&(*sample_it));
    assert(size != 0);
    return size;
}

uint64_t getTimestampFromSampleIterator(ByteBuffer::const_iterator& sample_it)
{
    auto ts = *reinterpret_cast<const uint64_t*>(&(*sample_it));
    return ts;
}


DtStreamPacket::DtStreamPacket(QObject *parent) : QObject(parent)
    , m_header()
    , m_info()
    , m_footer()
    , m_header_buffer()
    , m_sub_packets()
    , m_sequence_map()
    , m_crc32_table(nullptr)
    , m_channel_scalings()
{
    memset(&m_header, 0, sizeof(m_header));
    memset(&m_footer, 0, sizeof(m_footer));

    if (!m_crc32_table)
    {
        m_crc32_table = new uint32_t[256];
        crc32::generate_table(m_crc32_table);
    }
	m_isDebug = UserConfig::getInstance()->readSetting("RFM2G", "DATADEBUG").toInt();
}

DtStreamPacket::~DtStreamPacket()
{
    delete [] m_crc32_table;
}

int32_t DtStreamPacket::processPacketHeader(const ByteBuffer& header_buffer)
{
    assert(DT_PACKET_HEADER_SIZE == header_buffer.size());

    if (DT_PACKET_HEADER_SIZE == header_buffer.size())
    {
        // start token
        char start_token[DT_START_TOKEN_SIZE] = {0};
        auto pos = readData(header_buffer, header_buffer.begin(), start_token, DT_START_TOKEN_SIZE);
        if (0 != std::strncmp(DT_START_TOKEN, start_token, DT_START_TOKEN_SIZE))
        {
			if (m_isDebug)
				LOG(INFO) << "Invalid packet start token.";
           // DTLOG_ERROR("Invalid packet start token");
            return -1;
        }

        // read the complete packet size
        pos = readData(header_buffer, pos, &m_header.packet_size, sizeof(m_header.packet_size));

        // remember for checksum recalculation
        m_header_buffer = header_buffer;

        return 0;
    }

    return -1;
}


int32_t DtStreamPacket::processSubPackets(const ByteBuffer& packets_buffer)
{
    auto pos = packets_buffer.begin();
    bool hit_footer = false;

    while ((pos < packets_buffer.end()) && (!hit_footer))
    {
        auto sub_packet_start = pos;

        // Read generic subpacket info
        SubPacketBase packet_base;

        pos = readData(packets_buffer, pos, &packet_base.sub_packet_size, sizeof(packet_base.sub_packet_size));
        assert(packet_base.sub_packet_size >= 8);
        pos = readData(packets_buffer, pos, &packet_base.sub_packet_type, sizeof(packet_base.sub_packet_type));

        SubPacketBasePtr sub_packet;

        switch(packet_base.sub_packet_type)
        {
        case SBT_PACKET_INFO:       sub_packet = processPacketInfo(packets_buffer, pos, packet_base);  break;
        case SBT_XML_CONFIG:        sub_packet = processXmlConfig(packets_buffer, pos, packet_base);  break;
        case SBT_SYNC_FIXED:        sub_packet = processSyncFixed(packets_buffer, pos, packet_base);  break;
        case SBT_SYNC_VARIABLE:     sub_packet = processSyncVariable(packets_buffer, pos, packet_base);  break;
        case SBT_ASYNC_FIXED:       sub_packet = processAsyncFixed(packets_buffer, pos, packet_base);  break;
        case SBT_ASYNC_VARIABLE:    sub_packet = processAsyncVariable(packets_buffer, pos, packet_base);  break;
        case SBT_PACKET_FOOTER:
            sub_packet = processFooter(packets_buffer, pos, packet_base);
            hit_footer = true; // ensure to end the loop after footer processing
            break;
        default:
           // DTLOG_ERROR("Unsupported SubPacketType: " << packet_base.sub_packet_type);
            break;
        }

        if (sub_packet)
        {
            m_sub_packets.push_back(sub_packet);
        }

        // iterate to next subpacket
        pos = sub_packet_start + packet_base.sub_packet_size;
    }

    // calculate crc32
    uint32_t packet_crc = crc32::update(m_crc32_table, 0, m_header_buffer.data(), m_header_buffer.size());
    packet_crc = crc32::update(m_crc32_table, packet_crc, packets_buffer.data(), packets_buffer.size() - DT_PACKET_FOOTER_SIZE);

    if (m_footer.checksum != packet_crc)
    {
		if (m_isDebug)
			LOG(INFO) << "Packet checksum error " << std::hex << m_footer.checksum << " != " << packet_crc << std::dec;
        //DTLOG_ERROR("Packet checksum error " << std::hex << m_footer.checksum << " != " << packet_crc << std::dec);
    }
    return 0;
}

int32_t DtStreamPacket::getPacketSize() const
{
    return m_header.packet_size;
}

std::string DtStreamPacket::getPacketInfo() const
{
    std::stringstream s;
    std::stringstream v;

    s << "Packet size:  " << m_header.packet_size << "\n";
    v << "Version:      " << std::hex << std::setfill('0') << std::setw(8) << m_info.protocol_version << std::dec << "\n";
    s << v.str();
    s << "Stream ID:    " << m_info.stream_id << "\n";
    s << "Seq Number:   " << m_info.sequence_number << "\n";
    s << "Stream status:" << std::hex << m_info.stream_status << std::dec << "\n";
    s << "Stream seed:  " << std::hex << m_info.seed << std::dec << "\n";
    s << "Num sub pkt:  " << m_info.number_of_subpackets << "\n";
    s << "CRC32:        " << std::hex << m_footer.checksum  << std::dec << "\n";

	int32_t chan_idx = 0;
    for (std::size_t i = 0; i < m_sub_packets.size(); ++i)
    {
        const auto& pkt = m_sub_packets.at(i);

        switch(pkt->sub_packet_type)
        {
        case SBT_PACKET_INFO:
            // already printed ->m_info
            break;
        case SBT_XML_CONFIG:
            printXmlConfig(pkt, s);
            break;
        case SBT_SYNC_FIXED:
            printSyncFixed(pkt, s);
			++chan_idx;
            break;
        case SBT_SYNC_VARIABLE:
            printSyncVariable(pkt, s);
			++chan_idx;
            break;
        case SBT_ASYNC_FIXED:
            printAsyncFixed(pkt, s);
			++chan_idx;
            break;
        case SBT_ASYNC_VARIABLE:
            printAsyncVariable(pkt, s);
			++chan_idx;
            break;
        case SBT_PACKET_FOOTER:
            // already printed ->m_footer
            break;
        default:
            //DTLOG_ERROR("Unsupported SubPacketType: " << pkt->sub_packet_type);
            break;
        }

    }

    return s.str();
}

void DtStreamPacket::printChannelSamples()
{
    //DTLOG_INFO(2, "printChannelSamples");
	if (m_isDebug)
	   LOG(INFO) << "printChannelSamples.";
    int32_t chan_idx = 0;
    for (const auto& pkt : m_sub_packets)
    {
		if (m_isDebug)
		  LOG(INFO) << "sub_packet_type: " << pkt->sub_packet_type;
        switch(pkt->sub_packet_type)
        {
        case SBT_PACKET_INFO:
        case SBT_XML_CONFIG:
        case SBT_PACKET_FOOTER:
            // skip
            break;
        case SBT_SYNC_FIXED:
            printSamplesSyncFixed(chan_idx, std::static_pointer_cast<DtChannelSyncFixed>(pkt));
            ++chan_idx;
            break;
        case SBT_SYNC_VARIABLE:
            printSamplesSyncVariable(chan_idx, std::static_pointer_cast<DtChannelSyncVariable>(pkt));
            ++chan_idx;
            break;
        case SBT_ASYNC_FIXED:
        {
            auto channel = std::static_pointer_cast<DtChannelAsyncFixed>(pkt);
            if (channel->channel_data_type == dt_CAN)
            {
                printCANSamples(chan_idx, channel);
            }
            else
            {
                printSamplesAsyncFixed(chan_idx, channel);

            }
            ++chan_idx;
        }   break;
        case SBT_ASYNC_VARIABLE:
            printSamplesAsyncVariable(chan_idx, std::static_pointer_cast<DtChannelAsyncVariable>(pkt));
            ++chan_idx;
            break;
        default:
			if (m_isDebug)
				LOG(INFO) << "Unsupported SubPacketType: " << pkt->sub_packet_type;
            //DTLOG_ERROR("Unsupported SubPacketType: " << pkt->sub_packet_type);
            break;
        }
    }
}

void DtStreamPacket::clearChannels()
{
	if (m_isDebug)
		LOG(INFO) << "clearChannels.";
    m_sub_packets.clear();
}

bool DtStreamPacket::isLastPacket() const
{
    return (m_info.stream_status & ST_LAST_PACKET) != 0;
}

DtStreamPacket::iterator DtStreamPacket::readData(const ByteBuffer& byte_buf, DtStreamPacket::iterator pos, void* target, std::size_t target_size)
{
    memcpy(target, &(*pos), target_size);
    pos += target_size;
    assert(pos <= byte_buf.end());
    return pos;
}

DtStreamPacket::const_iterator DtStreamPacket::readData(const ByteBuffer& byte_buf, DtStreamPacket::const_iterator pos, void* target, std::size_t target_size)
{
    memcpy(target, &(*pos), target_size);
    pos += target_size;
    assert(pos <= byte_buf.end());
    return pos;
}

SubPacketBasePtr DtStreamPacket::processPacketDummy(const ByteBuffer& packets_buffer, const_iterator pos, const SubPacketBase& packet_base)
{
    return {};
}


SubPacketBasePtr DtStreamPacket::processPacketInfo(const ByteBuffer& packets_buffer, const_iterator pos, const SubPacketBase& packet_base)
{
/*
struct DtPacketInfo : public SubPacketBase
{
    uint32_t protocol_version;
    uint32_t stream_id;
    uint32_t sequence_number;
    uint32_t stream_status;
    uint32_t number_of_subpackets;
};
*/
    auto sub_packet = std::make_shared<DtPacketInfo>();
    subPacketInit(*sub_packet, packet_base);

    // protocol version
    pos = readData(packets_buffer, pos, &sub_packet->protocol_version, sizeof(sub_packet->protocol_version));
    if (sub_packet->protocol_version != DT_PROTOCOL_VERSION)
    {
        //DTLOG_WARN("Unsupported protocol version: " << std::hex << sub_packet->protocol_version << std::dec);
    }

    pos = readData(packets_buffer, pos, &sub_packet->stream_id, sizeof(sub_packet->stream_id));
    pos = readData(packets_buffer, pos, &sub_packet->sequence_number, sizeof(sub_packet->sequence_number));
    pos = readData(packets_buffer, pos, &sub_packet->stream_status, sizeof(sub_packet->stream_status));
    pos = readData(packets_buffer, pos, &sub_packet->seed, sizeof(sub_packet->seed));
    pos = readData(packets_buffer, pos, &sub_packet->number_of_subpackets, sizeof(sub_packet->number_of_subpackets));

    m_sub_packets.reserve(sub_packet->number_of_subpackets);

    if (m_sequence_map.find(sub_packet->stream_id) == m_sequence_map.end())
    {
        m_sequence_map[sub_packet->stream_id] = sub_packet->sequence_number;
    }

    if (sub_packet->sequence_number != m_sequence_map[sub_packet->stream_id]++)
    {
       // DTLOG_ERROR("Unexpected sequence number: " << sub_packet->sequence_number << " != " << m_sequence_map[sub_packet->stream_id]);
        return {};
    }

    if ((sub_packet->stream_status & ST_LAST_PACKET) != 0)
    {
        //DTLOG_INFO(1, "Reset sequence count for stream " << sub_packet->stream_id);
        m_sequence_map[sub_packet->stream_id] = 0;
    }

    m_info = *sub_packet;

    return sub_packet;
}


SubPacketBasePtr DtStreamPacket::processXmlConfig(const ByteBuffer& packets_buffer, const_iterator pos, const SubPacketBase& packet_base)
{
/*
struct DtXmlSubPacket : public SubPacketBase
{
    std::string xml_content;
};
*/
    auto sub_packet = std::make_shared<DtXmlSubPacket>();
    subPacketInit(*sub_packet, packet_base);

    const int32_t content_size = sub_packet->sub_packet_size - DT_PACKET_BASE_SIZE;

    assert(content_size >= 0);

    if (content_size > 0)
    {
        char* sbuff = new char[content_size];

        pos = readData(packets_buffer, pos, sbuff, content_size);

        sub_packet->xml_content = std::string(sbuff, content_size);

        // read channel linear scalings
        pugi::xml_document doc;
        auto status = doc.load_string(sbuff, content_size);
        if (status.status == pugi::status_ok)
        {
            auto root_name = doc.document_element().name();
            if (0 == std::strcmp("ChannelInfo", root_name))
            {
                auto channel_nodes = doc.document_element().select_nodes("Channel");
                for (const auto channel_node : channel_nodes)
                {
                    auto lin_sc_node = channel_node.node().select_node("LinearScaling");

                    LinearScaling ls;
                    ls.factor = lin_sc_node.node().attribute("factor").as_double();
                    ls.offset = lin_sc_node.node().attribute("offset").as_double();
                    m_channel_scalings.push_back(ls);
					//LOG(INFO) << "push_back m_channel_scalings: " << ls.factor << "<<"<<ls.offset;
                }
            }
        }

        delete [] sbuff;
    }

    return sub_packet;
}

SubPacketBasePtr DtStreamPacket::processSyncFixed(const ByteBuffer& packets_buffer, const_iterator pos, const SubPacketBase& packet_base)
{
/*
struct DtChannelSyncFixed : public SubPacketBase
{
    uint32_t channel_data_type;     // fixed size samples (not dt_binary, dt_string)
    uint32_t channel_dimension;
    uint32_t number_samples;
    uint64_t timestamp;
    double   timebase_frequency;
    ByteBuffer sample_data;
};
*/
    return processSync<DtChannelSyncFixed>(packets_buffer, pos, packet_base);
}

SubPacketBasePtr DtStreamPacket::processSyncVariable(const ByteBuffer& packets_buffer, const_iterator pos, const SubPacketBase& packet_base)
{
/*
struct DtChannelSyncVariable : public SubPacketBase
{
    uint32_t channel_data_type;     // variable size: dt_binary, dt_string
    uint32_t channel_dimension;
    uint32_t number_samples;
    uint64_t timestamp;
    double   timebase_frequency;
    ByteBuffer sample_data;
};
*/
    return processSync<DtChannelSyncVariable>(packets_buffer, pos, packet_base);
}

SubPacketBasePtr DtStreamPacket::processAsyncFixed(const ByteBuffer& packets_buffer, const_iterator pos, const SubPacketBase& packet_base)
{
/*
struct DtChannelAsyncFixed : public SubPacketBase
{
    uint32_t channel_data_type;     // fixed size samples (not dt_binary, dt_string)
    uint32_t channel_dimension;
    uint32_t number_samples;
    double   timebase_frequency;
    ByteBuffer sample_data;
};
*/
    return processAsync<DtChannelAsyncFixed>(packets_buffer, pos, packet_base);
}

SubPacketBasePtr DtStreamPacket::processAsyncVariable(const ByteBuffer& packets_buffer, const_iterator pos, const SubPacketBase& packet_base)
{
/*
struct DtChannelAsyncVariable : public SubPacketBase
{
    uint32_t channel_data_type;     // variable size: dt_binary, dt_string
    uint32_t channel_dimension;
    uint32_t number_samples;
    double   timebase_frequency;
    ByteBuffer sample_data;
};
*/
    return processAsync<DtChannelAsyncVariable>(packets_buffer, pos, packet_base);
}

SubPacketBasePtr DtStreamPacket::processFooter(const ByteBuffer& packets_buffer, const_iterator pos, const SubPacketBase& packet_base)
{
/*
struct DtPacketFooter : public SubPacketBase
{
    uint32_t checksum;
};
*/
    auto sub_packet = std::make_shared<DtPacketFooter>();
    subPacketInit(*sub_packet, packet_base);

    pos = readData(packets_buffer, pos, &sub_packet->checksum, sizeof(sub_packet->checksum));

    // end token
    char end_token[DT_END_TOKEN_SIZE] = {0};
    pos = readData(packets_buffer, pos, end_token, DT_END_TOKEN_SIZE);
    if (0 != std::strncmp(DT_END_TOKEN, end_token, DT_END_TOKEN_SIZE))
    {
       // DTLOG_ERROR("Invalid packet end token");
    }

    m_footer = *sub_packet;

    return sub_packet;
}

void DtStreamPacket::printXmlConfig(SubPacketBasePtr pkt, std::ostream& o) const
{
    auto xml_config = std::static_pointer_cast<DtXmlSubPacket>(pkt);
    o << "  XmlPacket:\n";
    o << "  xml_content:        " << xml_config->xml_content << "\n";
}

void DtStreamPacket::printSyncFixed(SubPacketBasePtr pkt, std::ostream& o) const
{
    auto channel = std::static_pointer_cast<DtChannelSyncFixed>(pkt);
    o << "  DtChannelSyncFixed:\n";
    o << "  channel_data_type:  " << toString(channel->channel_data_type) << "\n";
    o << "  channel_dimension:  " << channel->channel_dimension << "\n";
    o << "  number_samples:     " << channel->number_samples << "\n";
    o << "  timestamp:          " << channel->timestamp << "\n";
    o << "  timebase_frequency: " << channel->timebase_frequency << "\n";
}

void DtStreamPacket::printSyncVariable(SubPacketBasePtr pkt, std::ostream& o) const
{
    auto channel = std::static_pointer_cast<DtChannelSyncVariable>(pkt);
    o << "  DtChannelSyncVariable:\n";
    o << "  channel_data_type:  " << toString(channel->channel_data_type) << "\n";
    o << "  channel_dimension:  " << channel->channel_dimension << "\n";
    o << "  number_samples:     " << channel->number_samples << "\n";
    o << "  timestamp:          " << channel->timestamp << "\n";
    o << "  timebase_frequency: " << channel->timebase_frequency << "\n";
}

void DtStreamPacket::printAsyncFixed(SubPacketBasePtr pkt, std::ostream& o) const
{
    auto channel = std::static_pointer_cast<DtChannelAsyncFixed>(pkt);
    o << "  DtChannelAsyncFixed:\n";
    o << "  channel_data_type:  " << toString(channel->channel_data_type) << "\n";
    o << "  channel_dimension:  " << channel->channel_dimension << "\n";
    o << "  number_samples:     " << channel->number_samples << "\n";
    o << "  timebase_frequency: " << channel->timebase_frequency << "\n";
}

void DtStreamPacket::printAsyncVariable(SubPacketBasePtr pkt, std::ostream& o) const
{
    auto channel = std::static_pointer_cast<DtChannelAsyncVariable>(pkt);
    o << "  DtChannelAsyncVariable:\n";
    o << "  channel_data_type:  " << toString(channel->channel_data_type) << "\n";
    o << "  channel_dimension:  " << channel->channel_dimension << "\n";
    o << "  number_samples:     " << channel->number_samples << "\n";
    o << "  timebase_frequency: " << channel->timebase_frequency << "\n";
}

void DtStreamPacket::printSamplesSyncFixed(int32_t chan_idx, std::shared_ptr<DtChannelSyncFixed> channel)
{
    std::size_t sample_size = getDtDataTypeSize(channel->channel_data_type);

    //std::cout << '{';
	if (m_isDebug)
	   LOG(INFO) << "printSamplesSyncFixed <<";
    for (auto sample_it = channel->sample_data.cbegin();
         sample_it < channel->sample_data.cend(); sample_it = sample_it + sample_size)
    {
		if (&(*sample_it) == nullptr)
		{
			break;
		}
		if (m_channel_scalings.size() <= chan_idx)
		{
			break;
		}
		if (m_isDebug)
		  LOG(INFO) << "m_channel_scalings SIZE:" << m_channel_scalings.size() << ",IDX: " << chan_idx;
		std::string value = toSampleValue(sample_it, channel->channel_data_type, m_channel_scalings.at(chan_idx));
		emit onDataChanged(chan_idx, QString::fromStdString(value));
		//std::cout << '{' << value << "}, ";
		if (m_isDebug)
		   LOG(INFO) << "{" << value << "}, ";
    }

    //std::cout << "},\n";
}

void DtStreamPacket::printSamplesSyncVariable(int32_t chan_idx, std::shared_ptr<DtChannelSyncVariable> channel)
{
	if (m_isDebug)
	   LOG(INFO) << "printSamplesSyncVariable <<";
    auto sample_it = channel->sample_data.cbegin();
    //std::cout << '{';
    do
    {
		if (&(*sample_it) == nullptr)
		{
			break;
		}
        uint32_t sample_size = getSampleSizeFromSampleIterator(sample_it);
        sample_it += 4;
		std::string value = toSampleValueHex(sample_it, sample_size);
		emit onDataChanged(chan_idx, QString::fromStdString(value));
        //std::cout << '{' << sample_size << ',' << value << "}, ";
		if (m_isDebug)
		  LOG(INFO) << "{" << sample_size << "," << value << "}, ";
        sample_it += sample_size;
    }
    while (sample_it < channel->sample_data.cend());

   // std::cout << "},\n";
}

void DtStreamPacket::printSamplesAsyncFixed(int32_t chan_idx, std::shared_ptr<DtChannelAsyncFixed> channel)
{
	if (m_isDebug)
	  LOG(INFO) << "printSamplesAsyncFixed <<";
    // Note: 8 byte timestamp + size_of(channel_data_type)
    std::size_t sample_size = getDtDataTypeSize(channel->channel_data_type);
    auto sample_it = channel->sample_data.cbegin();

    //std::cout << '{';

    do
    {
		if (&(*sample_it) == nullptr)
		{
			break;
		}
		if (m_channel_scalings.size() <= chan_idx)
		{
			break;
		}
        uint64_t ts = getTimestampFromSampleIterator(sample_it);
        sample_it += 8;
		if (m_isDebug)
		  LOG(INFO) << "m_channel_scalings SIZE:" << m_channel_scalings.size() << ",IDX: " << chan_idx;
		std::string value = toSampleValue(sample_it, channel->channel_data_type, m_channel_scalings.at(chan_idx));
		emit onDataChanged(chan_idx, QString::fromStdString(value));
		//std::cout << '{' << ts << ", " << value << "}, ";
		if (m_isDebug)
		  LOG(INFO) << "{" << ts << ", " << value << "}, ";
        sample_it += sample_size;
    }
    while (sample_it < channel->sample_data.cend());

   // std::cout << "},\n";
}

void DtStreamPacket::printSamplesAsyncVariable(int32_t chan_idx, std::shared_ptr<DtChannelAsyncVariable> channel)
{
	if (m_isDebug)
	  LOG(INFO) << "printSamplesAsyncVariable <<";
    // Note: 8 byte timestamp + 4 byte sample size + data_sample
    auto sample_it = channel->sample_data.cbegin();

    //std::cout << '{';

    do
    {
		if (&(*sample_it) == nullptr)
		{
			break;
		}
        uint64_t ts = getTimestampFromSampleIterator(sample_it);
        sample_it += 8;
        uint32_t sample_size = getSampleSizeFromSampleIterator(sample_it);
        sample_it += 4;
		std::string value = toSampleValueHex(sample_it, sample_size);
        //std::cout << '{' << ts << ", " << sample_size << ", " << value << "}, ";
		if (m_isDebug)
		  LOG(INFO) << "{" << ts << ", " << sample_size << ", " << value << "}, ";
		emit onDataChanged(chan_idx, QString::fromStdString(value));
        sample_it += sample_size;
    }
    while (sample_it < channel->sample_data.cend());

   // std::cout << "},\n";
}

void DtStreamPacket::printCANSamples(int32_t chan_idx, std::shared_ptr<DtChannelAsyncFixed> channel)
{
	if (m_isDebug)
	  LOG(INFO) << "printCANSamples <<";
    std::size_t sample_size = getDtDataTypeSize(channel->channel_data_type);

    auto sample_it = channel->sample_data.cbegin();
   // std::cout << '{';
    do
    {
		if (&(*sample_it) == nullptr)
		{
			break;
		}
        uint64_t ts = getTimestampFromSampleIterator(sample_it);
        sample_it += 8;

        int64_t id = 0;
        memcpy(&id, &(*sample_it), 5);
        sample_it += 5;

        std::stringstream id_str;
        id_str << "0x" << std::hex << id;

        std::stringstream data_stream;

        data_stream << std::hex << std::setfill('0');

        auto it = sample_it;
        for (uint32_t i = 0; i < 8; ++i)
        {
            int32_t v = 0;
            memcpy(&v, &(*it), 1);
            data_stream << std::setw(2) << v << ' ';
            ++it;
        }

       // std::cout << '{' << ts << ", " << id_str.str() << ", " << data_stream.str() << "}, ";
		if (m_isDebug)
		   LOG(INFO) << "{" << ts << ", " << id_str.str() << ", " << data_stream.str() << "}, ";
		emit onDataChanged(chan_idx, QString::fromStdString(data_stream.str()));
        sample_it += sample_size;
    }
    while (sample_it < channel->sample_data.cend());

    //std::cout << "},\n";

}

