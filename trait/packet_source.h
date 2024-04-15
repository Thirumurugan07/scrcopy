#ifndef SC_PACKET_SOURCE_H
#define SC_PACKET_SOURCE_H

#include "common.h"
#include "packet_sink.h"
#include <libavcodec/avcodec.h> // Assuming you're using FFmpeg for multimedia processing

#define SC_PACKET_SOURCE_MAX_SINKS 3

struct sc_packet_source;
typedef struct sc_packet_source sc_packet_source_t;

struct sc_packet_source {
    struct sc_packet_sink* sinks[SC_PACKET_SOURCE_MAX_SINKS];
    unsigned sink_count;
};

void sc_packet_source_init(struct sc_packet_source* source);
void sc_packet_source_add_sink(struct sc_packet_source* source, struct sc_packet_sink* sink);
bool sc_packet_source_sinks_open(struct sc_packet_source* source, AVCodecContext* ctx);
void sc_packet_source_sinks_close(struct sc_packet_source* source);
bool sc_packet_source_sinks_push(struct sc_packet_source* source, const AVPacket* packet);
void sc_packet_source_sinks_disable(struct sc_packet_source* source);

#endif // SC_PACKET_SOURCE_H
