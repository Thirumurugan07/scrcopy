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
#include <time.h>
/** Downcast packet_sink to decoder */
#define DOWNCAST(SINK) container_of(SINK, struct sc_decoder, packet_sink)
static bool
sc_decoder_open(struct sc_decoder* decoder, AVCodecContext* ctx) {
    decoder->frame = av_frame_alloc();
    if (!decoder->frame) {
        LOG_OOM();
        return false;
    }

    /*if (!sc_frame_source_sinks_open(&decoder->frame_source, ctx)) {
        av_frame_free(&decoder->frame);
        return false;
    }*/

    decoder->ctx = ctx;

    return true;
}
time_t start;


static void
sc_decoder_close(struct sc_decoder* decoder) {
    sc_frame_source_sinks_close(&decoder->frame_source);
    av_frame_free(&decoder->frame);
}

#include <libswscale/swscale.h>
int frame_count = 0;

static bool sc_decoder_push(struct sc_decoder* decoder, const AVPacket* packet) {
    bool is_config = packet->pts == AV_NOPTS_VALUE;
    if (is_config) {
        // nothing to do
        return true;
    }

    int ret = avcodec_send_packet(decoder->ctx, packet);
    if (ret < 0 && ret != AVERROR(EAGAIN)) {
        LOGE("Decoder '%s': could not send video packet: %d", decoder->name, ret);
        return false;
    }

    for (;;) {
        ret = avcodec_receive_frame(decoder->ctx, decoder->frame);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
            break;
        }
        if (ret < 0) {
            LOGE("Decoder '%s', could not receive video frame: %d", decoder->name, ret);
            return false;
        }
        time_t curr = time(NULL);
        if (difftime(curr, start) > 1) {
            LOGI("More than 1 sec");
            exit(1);
        }
        frame_count++;
        printf("Frame forming %d\n", frame_count);

        char filename[256];
        snprintf(filename, sizeof(filename), "C:\\Users\\HIZ1COB\\Developer\\images\\frame_%d.bin", frame_count);

        /// Allocate buffer for the RGB image (modify bgr_size calculation)
        int dst_width = decoder->frame->width;
        int dst_height = decoder->frame->height;
        int rgb_size = av_image_get_buffer_size(AV_PIX_FMT_RGB24, dst_width, dst_height, 1);
        uint8_t* rgb_buffer = (uint8_t*)av_malloc(rgb_size);
        if (!rgb_buffer) {
            LOGE("Failed to allocate buffer for RGB image");
            return false;
        }

        // Create AVFrame for RGB image
        AVFrame* rgb_frame = av_frame_alloc();
        if (!rgb_frame) {
            LOGE("Failed to allocate AVFrame for RGB image");
            av_free(rgb_buffer);
            return false;
        }
        av_image_fill_arrays(rgb_frame->data, rgb_frame->linesize, rgb_buffer, AV_PIX_FMT_RGB24, dst_width, dst_height, 1);

        // Create SwsContext for the conversion (modify format)
        struct SwsContext* swsctx = sws_getContext(decoder->frame->width, decoder->frame->height, decoder->frame->format,
            dst_width, dst_height, AV_PIX_FMT_RGB24,
            SWS_BICUBIC, NULL, NULL, NULL);

        if (!swsctx) {
            LOGE("Failed to allocate SwsContext");
            av_frame_free(&rgb_frame);
            av_free(rgb_buffer);
            return false;
        }

        // Perform pixel format conversion
        sws_scale(swsctx, decoder->frame->data, decoder->frame->linesize, 0, decoder->frame->height,
            rgb_frame->data, rgb_frame->linesize);

        // Save the RGB image as a binary file
        FILE* file = fopen(filename, "wb");
        if (!file) {
            LOGE("Failed to open file for writing: %s", filename);
            av_frame_free(&rgb_frame);
            av_free(rgb_buffer);
            sws_freeContext(swsctx);
            return false;
        }
        fwrite(rgb_frame->data[0], 1, rgb_size, file);
        fclose(file);

        // Free allocated memory
        av_frame_unref(decoder->frame);
        av_frame_free(&rgb_frame);
        av_free(rgb_buffer);
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
    start = time(NULL);
    decoder->packet_sink.ops = &ops;
}