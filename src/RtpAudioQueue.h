#pragma once

#include "Video.h"

#include "rs.h"

#define RTPQ_DEFAULT_QUEUE_TIME 40

#define RTPA_DATA_SHARDS 4
#define RTPA_FEC_SHARDS 2
#define RTPA_TOTAL_SHARDS (RTPA_DATA_SHARDS + RTPA_FEC_SHARDS)

typedef struct _AUDIO_FEC_HEADER {
    uint8_t fecShardIndex;
    uint8_t payloadType;
    uint16_t baseSequenceNumber;
    uint32_t baseTimestamp;
    uint32_t ssrc;
} AUDIO_FEC_HEADER, *PAUDIO_FEC_HEADER;

typedef struct _RTPA_FEC_BLOCK {
    struct _RTPA_FEC_BLOCK* prev;
    struct _RTPA_FEC_BLOCK* next;

    PRTP_PACKET dataPackets[RTPA_DATA_SHARDS];
    uint8_t* fecPackets[RTPA_FEC_SHARDS];
    uint8_t marks[RTPA_TOTAL_SHARDS];

    AUDIO_FEC_HEADER fecHeader;

    uint64_t queueTimeMs;
    uint8_t dataShardsReceived;
    uint8_t fecShardsReceived;
    bool fullyReassembled;

    // Used when dequeuing data from FEC blocks for the caller
    uint8_t nextDataPacketIndex;

    uint16_t blockSize;

    // Data for shards comes here
} RTPA_FEC_BLOCK, *PRTPA_FEC_BLOCK;

typedef struct _RTP_AUDIO_QUEUE {
    PRTPA_FEC_BLOCK blockHead;
    PRTPA_FEC_BLOCK blockTail;

    reed_solomon* rs;

    uint32_t maxQueueTimeMs;

    uint16_t nextRtpSequenceNumber;
    uint16_t oldestRtpBaseSequenceNumber;
} RTP_AUDIO_QUEUE, *PRTP_AUDIO_QUEUE;

#define RTPQ_RET_PACKET_CONSUMED 0x1
#define RTPQ_RET_PACKET_READY    0x2
#define RTPQ_RET_HANDLE_NOW      0x4

#define RTPQ_PACKET_CONSUMED(x) ((x) & RTPQ_RET_PACKET_CONSUMED)
#define RTPQ_PACKET_READY(x)    ((x) & RTPQ_RET_PACKET_READY)
#define RTPQ_HANDLE_NOW(x)      ((x) == RTPQ_RET_HANDLE_NOW)

void RtpaInitializeQueue(PRTP_AUDIO_QUEUE queue);
void RtpaCleanupQueue(PRTP_AUDIO_QUEUE queue);
int RtpaAddPacket(PRTP_AUDIO_QUEUE queue, PRTP_PACKET packet, uint16_t length);
PRTP_PACKET RtpaGetQueuedPacket(PRTP_AUDIO_QUEUE queue, uint16_t customHeaderLength, uint16_t* length);