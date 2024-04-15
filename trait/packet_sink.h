#ifndef SC_PACKET_SINK_H
#define SC_PACKET_SINK_H

#include "common.h"

#include <assert.h>
#include <stdbool.h>
#include <libavcodec/avcodec.h>

struct sc_packet_sink;
typedef struct sc_packet_sink sc_packet_sink_t;

struct sc_packet_sink_ops;
typedef struct sc_packet_sink_ops sc_packet_sink_ops_t;

/**
 * Packet sink trait.
 *
 * Component able to receive AVPackets should implement this trait.
 */
typedef bool(*open_func_t)(sc_packet_sink_t* sink, AVCodecContext* ctx);
typedef void(*close_func_t)(sc_packet_sink_t* sink);
typedef bool(*push_func_t)(sc_packet_sink_t* sink, const AVPacket* packet);
typedef void(*disable_func_t)(sc_packet_sink_t* sink);

struct sc_packet_sink {
    const sc_packet_sink_ops_t* ops;
};

struct sc_packet_sink_ops {
    open_func_t open;
    close_func_t close;
    push_func_t push;
    disable_func_t disable;
};


#endif