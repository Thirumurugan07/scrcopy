#include "decoder.h"
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/channel_layout.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
#include <libswscale/swscale.h>
#include "events.h"
#include "trait/frame_sink.h"
#include "util/log.h"
/** Downcast packet_sink to decoder */
#define DOWNCAST(SINK) container_of(SINK, struct sc_decoder, packet_sink)
static bool
sc_decoder_open(struct sc_decoder* decoder, AVCodecContext* ctx) {
    decoder->frame = av_frame_alloc();
    if (!decoder->frame) {
        LOG_OOM();
        return false;
    }

    if (!sc_frame_source_sinks_open(&decoder->frame_source, ctx)) {
        av_frame_free(&decoder->frame);
        return false;
    }

    decoder->ctx = ctx;

    return true;
}
//void save_frame_as_bin(const AVFrame* frame, const char* filename) {
//    int width = frame->width;
//    int height = frame->height;
//    int channels = 3; // Assuming BGR format
//    int step = channels * width;
//
//    // Open the file for writing
//    FILE* file = fopen(filename, "wb");
//    if (file == NULL) {
//        printf("Error: Could not open file for writing.\n");
//        return;
//    }
//    uint8_t* data = frame->data[0];
//    for (int i = 0; i < height; i++) {
//        fwrite(data + i * frame->linesize[0], sizeof(uint8_t), step, file);
//    }
//
//    fclose(file);
//}
void save_frame_as_image(const AVFrame* frame, const char* filename) {
    int width = frame->width;
    int height = frame->height;
    int channels = 3; // Assuming RGB format
    int step = channels * width;

    // Open the file for writing
    FILE* file = fopen(filename, "wb");
    if (file == NULL) {
        printf("Error: Could not open file for writing.\n");
        return;
    }

    // Write BMP file header
    uint8_t bmp_header[54] = {
        0x42, 0x4D,             // BM
        0x36, 0x00, 0x00, 0x00, // File size (placeholder)
        0x00, 0x00,             // Reserved
        0x00, 0x00,             // Reserved
        0x36, 0x00, 0x00, 0x00, // Pixel data offset

        0x28, 0x00, 0x00, 0x00, // Header size
        width & 0xFF, (width >> 8) & 0xFF, (width >> 16) & 0xFF, (width >> 24) & 0xFF, // Image width
        height & 0xFF, (height >> 8) & 0xFF, (height >> 16) & 0xFF, (height >> 24) & 0xFF, // Image height
        0x01, 0x00,             // Planes
        0x18, 0x00,             // Bits per pixel (24-bit RGB)
        0x00, 0x00, 0x00, 0x00, // Compression (none)
        0x00, 0x00, 0x00, 0x00, // Image size (placeholder)
        0x00, 0x00, 0x00, 0x00, // X pixels per meter (placeholder)
        0x00, 0x00, 0x00, 0x00, // Y pixels per meter (placeholder)
        0x00, 0x00, 0x00, 0x00, // Total colors (none)
        0x00, 0x00, 0x00, 0x00  // Important colors (none)
    };
    fwrite(bmp_header, sizeof(uint8_t), 54, file);

    // Write pixel data (BGR format)
    uint8_t* data = frame->data[0];
    for (int i = height - 1; i >= 0; i--) {
        fwrite(data + i * frame->linesize[0], sizeof(uint8_t), step, file);
    }

    // Close the file
    fclose(file);
}

static void
sc_decoder_close(struct sc_decoder* decoder) {
    sc_frame_source_sinks_close(&decoder->frame_source);
    av_frame_free(&decoder->frame);
}

#include <libswscale/swscale.h>

static bool sc_decoder_push(struct sc_decoder* decoder, const AVPacket* packet) {
    bool is_config = packet->pts == AV_NOPTS_VALUE;
    if (is_config) {
        // nothing to do
        return true;
    }

    int ret = avcodec_send_packet(decoder->ctx, packet);
    if (ret < 0 && ret != AVERROR(EAGAIN)) {
        LOGE("Decoder '%s': could not send video packet: %d",
            decoder->name, ret);
        return false;
    }
    char filename[256];
    int frame_count = 0;

    // Create a SwsContext for the conversion
    struct SwsContext* swsctx = sws_getContext(1080, 2400, AV_PIX_FMT_YUV420P,
        1080, 2400, AV_PIX_FMT_BGR24,
        SWS_BICUBIC, NULL, NULL, NULL);

    if (!swsctx) {
        LOGE("Failed to allocate SwsContext");
        // Handle the error appropriately, e.g., return false
    }

    for (;;) {
        ret = avcodec_receive_frame(decoder->ctx, decoder->frame);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
            break;
        }

        if (ret) {
            LOGE("Decoder '%s', could not receive video frame: %d",
                decoder->name, ret);
            return false;
        }

        snprintf(filename, sizeof(filename), "C:\\Users\\HIZ1COB\\Developer\\images\\frame_%d.bin", frame_count);
        // Allocate buffer for the BGR image
        int dst_width = decoder->frame->width;
        int dst_height = decoder->frame->height;
        int dst_channels = 3; // BGR format
        int bgr_size = av_image_get_buffer_size(AV_PIX_FMT_BGR24, dst_width, dst_height, 1);
        uint8_t* bgr_buffer = (uint8_t*)av_malloc(bgr_size);
        if (!bgr_buffer) {
            LOGE("Failed to allocate buffer for BGR image");
            // Handle the error appropriately, e.g., return false
        }

        // Create AVFrame for BGR image
        AVFrame* bgr_frame = av_frame_alloc();
        if (!bgr_frame) {
            LOGE("Failed to allocate AVFrame for BGR image");
            av_free(bgr_buffer);
            // Handle the error appropriately, e.g., return false
        }
        av_image_fill_arrays(bgr_frame->data, bgr_frame->linesize, bgr_buffer, AV_PIX_FMT_BGR24, dst_width, dst_height, 1);

        // Perform pixel format conversion
        sws_scale(swsctx, decoder->frame->data, decoder->frame->linesize, 0, decoder->frame->height,
            bgr_frame->data, bgr_frame->linesize);

        // Save the BGR image as a binary file
        FILE* file = fopen(filename, "wb");
        if (!file) {
            LOGE("Failed to open file for writing: %s", filename);
            av_frame_free(&bgr_frame);
            av_free(bgr_buffer);
            // Handle the error appropriately, e.g., return false
        }
        fwrite(bgr_frame->data[0], 1, bgr_size, file);
        fclose(file);

        // Push the frame to the frame source
        bool ok = sc_frame_source_sinks_push(&decoder->frame_source, decoder->frame);
        av_frame_unref(decoder->frame);
        av_frame_free(&bgr_frame);
        av_free(bgr_buffer);
        if (!ok) {
            // Error already logged
            return false;
        }
    }

    // Free the SwsContext
    if (swsctx) {
        sws_freeContext(swsctx);
    }

    return true;
}


static bool
sc_decoder_packet_sink_open(struct sc_packet_sink* sink, AVCodecContext* ctx) {
    struct sc_decoder* decoder = DOWNCAST(sink);
    return sc_decoder_open(decoder, ctx);
}

static void
sc_decoder_packet_sink_close(struct sc_packet_sink* sink) {
    struct sc_decoder* decoder = DOWNCAST(sink);
    sc_decoder_close(decoder);
}

static bool
sc_decoder_packet_sink_push(struct sc_packet_sink* sink,
    const AVPacket* packet) {
    struct sc_decoder* decoder = DOWNCAST(sink);
    return sc_decoder_push(decoder, packet);
}

void
sc_decoder_init(struct sc_decoder* decoder, const char* name) {
    decoder->name = name; 
    sc_frame_source_init(&decoder->frame_source);

    static const struct sc_packet_sink_ops ops = {
        .open = sc_decoder_packet_sink_open,
        .close = sc_decoder_packet_sink_close,
        .push = sc_decoder_packet_sink_push,
    };

    decoder->packet_sink.ops = &ops;
}
