// Copyright DEWETRON GmbH 2018
#pragma once

#include <cinttypes>
#include <map>
#include <memory>
#include <string>
#include <vector>
#include <QObject>
#include <QMutex>

/**
 * Protocol 1.5 defines
 */
#define DT_PROTOCOL_VERSION         0x01050000
#define DT_WELCOME_MSG_SIZE         64

#define DT_START_TOKEN              "OXYGEN<<"
#define DT_END_TOKEN                ">>OXYGEN"
#define DT_START_TOKEN_SIZE          8
#define DT_END_TOKEN_SIZE            8

// Fix size packets:
#define DT_PACKET_BASE_SIZE          8
#define DT_PACKET_HEADER_SIZE        12
#define DT_PACKET_INFO_SIZE          32
#define DT_PACKET_ACQ_INFO_SIZE      16
#define DT_PACKET_FOOTER_SIZE        20

#define DT_PACKET_SYNC_HEADER_SIZE   36
#define DT_PACKET_ASYNC_HEADER_SIZE  28

using ByteBuffer = std::vector<char>;


/**
 * Sub Packet types valid for version 1.5
 */
enum SubPacketType
{
    SBT_PACKET_INFO         = 0x00000001,
    SBT_XML_CONFIG          = 0x00000002,
    SBT_SYNC_FIXED          = 0x00000003,
    SBT_SYNC_VARIABLE       = 0x00000004,
    SBT_ASYNC_FIXED         = 0x00000005,
    SBT_ASYNC_VARIABLE      = 0x00000006,
    SBT_PACKET_FOOTER       = 0x00000007,
};

/**
 * Mark the dedication of the stream
 */
enum StreamStatus
{
    ST_FIRST_PACKET         = 0x00000001,
    ST_LAST_PACKET          = 0x00000002,
    ST_NORMAL_PACKET        = 0x00000000,
    ST_ERROR_PACKET         = 0x10000000,
};

/**
 * Header packet for all versions
 */
struct DtPacketHeader
{
    uint32_t packet_size;
};

/**
 * Subpacket base class
 */
struct SubPacketBase
{
    uint32_t sub_packet_size;
    uint32_t sub_packet_type;
};

template<typename P1, typename P2>
void subPacketInit(P1& dest, const P2& source)
{
    dest.sub_packet_size = source.sub_packet_size;
    dest.sub_packet_type = source.sub_packet_type;
}

using SubPacketBasePtr = std::shared_ptr<SubPacketBase>;


/**
 * Packet info subpacket V1.5
 * "SBT_PACKET_INFO"
 */
struct DtPacketInfo : public SubPacketBase
{
    uint32_t protocol_version;
    uint32_t stream_id;
    uint32_t sequence_number;
    uint32_t stream_status;
    uint32_t seed;
    uint32_t number_of_subpackets;
};

/**
 * Generic XML subpacket V1.5
 * "SBT_XML_CONFIG"
 */
struct DtXmlSubPacket : public SubPacketBase
{
    std::string xml_content;
};

/**
 * Sync channel fixed sample size V1.5
 * "SBT_SYNC_FIXED"
 */
struct DtChannelSyncFixed : public SubPacketBase
{
    uint32_t channel_data_type;     // fixed size samples (not dt_binary, dt_string)
    uint32_t channel_dimension;
    uint32_t number_samples;
    uint64_t timestamp;
    double   timebase_frequency;
    ByteBuffer sample_data;
};

/**
 * Sync channel variable sample size V1.5
 * "SBT_SYNC_VARIABLE"
 */
struct DtChannelSyncVariable : public SubPacketBase
{
    uint32_t channel_data_type;     // variable size: dt_binary, dt_string
    uint32_t channel_dimension;
    uint32_t number_samples;
    uint64_t timestamp;
    double   timebase_frequency;
    ByteBuffer sample_data;
};

/**
 * Async channel fixed sample size V1.5
 * "SBT_ASYNC_FIXED"
 */
struct DtChannelAsyncFixed : public SubPacketBase
{
    uint32_t channel_data_type;     // fixed size samples (not dt_binary, dt_string)
    uint32_t channel_dimension;
    uint32_t number_samples;
    double   timebase_frequency;
    ByteBuffer sample_data;
};

/**
 * Async channel variable sample size V1.5
 * "SBT_ASYNC_VARIABLE"
 */
struct DtChannelAsyncVariable : public SubPacketBase
{
    uint32_t channel_data_type;     // variable size: dt_binary, dt_string
    uint32_t channel_dimension;
    uint32_t number_samples;
    double   timebase_frequency;
    ByteBuffer sample_data;
};

/**
 * "SBT_PACKET_FOOTER"
 */
struct DtPacketFooter : public SubPacketBase
{
    uint32_t checksum;
};


/**
 * Sample data types
 */
enum DtDataType
{
    dt_sint8 = 0,          // 0
    dt_uint8,              // 1
    dt_sint16,             // 2
    dt_uint16,             // 3
    dt_sint24,             // 4
    dt_uint24,             // 5
    dt_sint32,             // 6
    dt_uint32,             // 7
    dt_sint64,             // 8
    dt_uint64,             // 9
    dt_float,              // 10
    dt_double,             // 11
    dt_complex_float,      // 12
    dt_complex_double,     // 13
    dt_string,             // 14
    dt_binary,             // 15
    dt_CAN,                // 16
};

std::size_t getDtDataTypeSize(int32_t dt);

std::string toSampleValue(ByteBuffer::const_iterator& sample_it, int32_t dt);

std::string toSampleValueHex(ByteBuffer::const_iterator& sample_it, uint32_t data_size);

uint32_t getSampleSizeFromSampleIterator(ByteBuffer::const_iterator& sample_it);

uint64_t getTimestampFromSampleIterator(ByteBuffer::const_iterator& sample_it);


struct LinearScaling
{
    double factor;
    double offset;
	LinearScaling()
	{
		factor = 0.00;
		offset = 0.00;
	}
};


/**
 * OXYGEN DATA TRANSFER Protocol 1.5
 */
class DtStreamPacket : public QObject
{
	Q_OBJECT
public:
    using iterator = ByteBuffer::iterator;
    using const_iterator = ByteBuffer::const_iterator;

	DtStreamPacket(QObject *parent = 0);
    ~DtStreamPacket();

    /**
     * Process the fix default packet header
     */
    int32_t processPacketHeader(const ByteBuffer& header_buffer);

    /**
     * After the packet header is processed, it is possible to
     * get the size of the complete package (including the header)
     */
    int32_t getPacketSize() const;

    /**
     * Process all sub packets
     */
    int32_t processSubPackets(const ByteBuffer& channels_buffer);

    /**
     * Print information about the received packet.
     */
    std::string getPacketInfo() const;

    /**
     *
     */
    void printChannelSamples();

    void clearChannels();

    bool isLastPacket() const;
	void OnClear(){ m_channel_scalings.clear(); };

    iterator readData(const ByteBuffer& byte_buf, iterator pos, void* target, std::size_t target_size);
    const_iterator readData(const ByteBuffer& byte_buf, const_iterator pos, void* target, std::size_t target_size);

signals:
	void onDataChanged(int idx, QString value);

private:
    SubPacketBasePtr processPacketDummy(const ByteBuffer& packets_buffer, const_iterator pos, const SubPacketBase& packet_base);
    SubPacketBasePtr processPacketInfo(const ByteBuffer& packets_buffer, const_iterator pos, const SubPacketBase& packet_base);
    SubPacketBasePtr processXmlConfig(const ByteBuffer& packets_buffer, const_iterator pos, const SubPacketBase& packet_base);
    SubPacketBasePtr processSyncFixed(const ByteBuffer& packets_buffer, const_iterator pos, const SubPacketBase& packet_base);
    SubPacketBasePtr processSyncVariable(const ByteBuffer& packets_buffer, const_iterator pos, const SubPacketBase& packet_base);
    SubPacketBasePtr processAsyncFixed(const ByteBuffer& packets_buffer, const_iterator pos, const SubPacketBase& packet_base);
    SubPacketBasePtr processAsyncVariable(const ByteBuffer& packets_buffer, const_iterator pos, const SubPacketBase& packet_base);
    SubPacketBasePtr processFooter(const ByteBuffer& packets_buffer, const_iterator pos, const SubPacketBase& packet_base);

    void printXmlConfig(SubPacketBasePtr pkt, std::ostream& o) const;
    void printSyncFixed(SubPacketBasePtr pkt, std::ostream& o) const;
    void printSyncVariable(SubPacketBasePtr pkt, std::ostream& o) const;
    void printAsyncFixed(SubPacketBasePtr pkt, std::ostream& o) const;
    void printAsyncVariable(SubPacketBasePtr pkt, std::ostream& o) const;

    void printSamplesSyncFixed(int32_t chan_idx, std::shared_ptr<DtChannelSyncFixed> channel);
    void printSamplesSyncVariable(int32_t chan_idx, std::shared_ptr<DtChannelSyncVariable> channel);
    void printSamplesAsyncFixed(int32_t chan_idx, std::shared_ptr<DtChannelAsyncFixed> channel);
    void printSamplesAsyncVariable(int32_t chan_idx, std::shared_ptr<DtChannelAsyncVariable> channel);
    void printCANSamples(int32_t chan_idx, std::shared_ptr<DtChannelAsyncFixed> channel);

    template<typename DtChannelSync>
    SubPacketBasePtr processSync(const ByteBuffer& packets_buffer, ByteBuffer::const_iterator pos, const SubPacketBase& packet_base)
    {
		QMutexLocker locker(&m_pMutex);
        auto sub_packet = std::make_shared<DtChannelSync>();
        subPacketInit(*sub_packet, packet_base);

        pos = readData(packets_buffer, pos, &sub_packet->channel_data_type, sizeof(sub_packet->channel_data_type));
        pos = readData(packets_buffer, pos, &sub_packet->channel_dimension, sizeof(sub_packet->channel_dimension));
        pos = readData(packets_buffer, pos, &sub_packet->number_samples, sizeof(sub_packet->number_samples));
        pos = readData(packets_buffer, pos, &sub_packet->timestamp, sizeof(sub_packet->timestamp));
        pos = readData(packets_buffer, pos, &sub_packet->timebase_frequency, sizeof(sub_packet->timebase_frequency));

        sub_packet->sample_data.resize(sub_packet->sub_packet_size - DT_PACKET_SYNC_HEADER_SIZE);
        pos = readData(packets_buffer, pos, sub_packet->sample_data.data(), sub_packet->sample_data.size());

        return sub_packet;
    }

    template<typename DtChannelSync>
    SubPacketBasePtr processAsync(const ByteBuffer& packets_buffer, ByteBuffer::const_iterator pos, const SubPacketBase& packet_base)
    {
		QMutexLocker locker(&m_pMutex);
        auto sub_packet = std::make_shared<DtChannelSync>();
        subPacketInit(*sub_packet, packet_base);

        pos = readData(packets_buffer, pos, &sub_packet->channel_data_type, sizeof(sub_packet->channel_data_type));
        pos = readData(packets_buffer, pos, &sub_packet->channel_dimension, sizeof(sub_packet->channel_dimension));
        pos = readData(packets_buffer, pos, &sub_packet->number_samples, sizeof(sub_packet->number_samples));
        pos = readData(packets_buffer, pos, &sub_packet->timebase_frequency, sizeof(sub_packet->timebase_frequency));

        sub_packet->sample_data.resize(sub_packet->sub_packet_size - DT_PACKET_ASYNC_HEADER_SIZE);
        pos = readData(packets_buffer, pos, sub_packet->sample_data.data(), sub_packet->sample_data.size());

        return sub_packet;
    }

private:
    DtPacketHeader  m_header;
    DtPacketInfo    m_info;
    DtPacketFooter  m_footer;

    ByteBuffer m_header_buffer;
    std::vector<SubPacketBasePtr> m_sub_packets;

    std::map<uint32_t, uint32_t> m_sequence_map;
    uint32_t* m_crc32_table;

    std::vector<LinearScaling> m_channel_scalings;
	QMutex m_pMutex;
};
