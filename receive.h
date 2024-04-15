#include "demuxer.h"

static bool
sc_demuxer_recv_packet(struct sc_demuxer* demuxer, AVPacket* packet) {
    // The video and audio streams contain a sequence of raw packets (as
    // provided by MediaCodec), each prefixed with a "meta" header.
    //
    // The "meta" header length is 12 bytes:
    // [. . . . . . . .|. . . .]. . . . . . . . . . . . . . . ...
    //  <-------------> <-----> <-----------------------------...
    //        PTS        packet        raw packet
    //                    size
    //
    // It is followed by <packet_size> bytes containing the packet/frame.
    //
    // The most significant bits of the PTS are used for packet flags:
    //
    //  byte 7   byte 6   byte 5   byte 4   byte 3   byte 2   byte 1   byte 0
    // CK...... ........ ........ ........ ........ ........ ........ ........
    // ^^<------------------------------------------------------------------->
    // ||                                PTS
    // | `- key frame
    //  `-- config packet

    uint8_t header[SC_PACKET_HEADER_SIZE];
    ssize_t r = net_recv_all(demuxer->socket, header, SC_PACKET_HEADER_SIZE);
    if (r < SC_PACKET_HEADER_SIZE) {
        return false;
    }

    uint64_t pts_flags = sc_read64be(header);
    uint32_t len = sc_read32be(&header[8]);
    assert(len);

    if (av_new_packet(packet, len)) {
        LOG_OOM();
        return false;
    }

    r = net_recv_all(demuxer->socket, packet->data, len);
    if (r < 0 || ((uint32_t)r) < len) {
        av_packet_unref(packet);
        return false;
    }

    if (pts_flags & SC_PACKET_FLAG_CONFIG) {
        packet->pts = AV_NOPTS_VALUE;
    }
    else {
        packet->pts = pts_flags & SC_PACKET_PTS_MASK;
    }

    if (pts_flags & SC_PACKET_FLAG_KEY_FRAME) {
        packet->flags |= AV_PKT_FLAG_KEY;
    }

    packet->dts = packet->pts;
    return true;
}
