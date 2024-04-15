#ifndef SC_DEMUXER_H
#define SC_DEMUXER_H

#include "common.h"

#include <stdbool.h>
#include <stdint.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>

#include "trait/packet_source.h"
#include "trait/packet_sink.h"
#include "util/net.h"
#include "util/thread.h"

#include <assert.h>
#include <libavutil/channel_layout.h>
#include <libavutil/time.h>
#include <windows.h> 
#include "events.h"
#include "packet_merger.h"
#include "util/binary.h"
#include "util/log.h"

#define SC_PACKET_HEADER_SIZE 12

#define SC_PACKET_FLAG_CONFIG    (UINT64_C(1) << 63)
#define SC_PACKET_FLAG_KEY_FRAME (UINT64_C(1) << 62)

#define SC_PACKET_PTS_MASK (SC_PACKET_FLAG_KEY_FRAME - 1)


struct sc_demuxer {
    struct sc_packet_source packet_source; // packet source trait

    const char* name; // must be statically allocated (e.g. a string literal)

    sc_socket socket;
    sc_thread thread;

    const struct sc_demuxer_callbacks* cbs;
    void* cbs_userdata;
};

enum sc_demuxer_status {
    SC_DEMUXER_STATUS_EOS,
    SC_DEMUXER_STATUS_DISABLED,
    SC_DEMUXER_STATUS_ERROR,
};

struct sc_demuxer_callbacks {
    void (*on_ended)(struct sc_demuxer* demuxer, enum sc_demuxer_status,
        void* userdata);
};
// The name must be statically allocated (e.g. a string literal)
void
sc_demuxer_init(struct sc_demuxer* demuxer, const char* name, sc_socket socket);

bool
sc_demuxer_start(struct sc_demuxer* demuxer);

void
sc_demuxer_join(struct sc_demuxer* demuxer);

#endif
