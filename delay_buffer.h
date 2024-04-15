#ifndef sc_delay_buffer_h
#define sc_delay_buffer_h
//
#include "common.h"

#include <stdbool.h>

#include "clock.h"
#include "trait/frame_source.h"
#include "trait/frame_sink.h"
#include "util/thread.h"
#include "util/ticks.h"
#include "util/vecdeque.h"

// forward declarations
typedef struct avframe avframe;

struct sc_delayed_frame {
    avframe* frame;
#ifndef ndebug
    sc_tick push_date;
#endif
};

struct sc_delayed_frame_queue sc_vecdeque(struct sc_delayed_frame);

struct sc_delay_buffer {
    struct sc_frame_source frame_source; // frame source trait
    struct sc_frame_sink frame_sink; // frame sink trait

    sc_tick delay;
    bool first_frame_asap;

    sc_thread thread;
    sc_mutex mutex;
    sc_cond queue_cond;
    sc_cond wait_cond;

    struct sc_clock clock;
  //  struct sc_delayed_frame_queue queue;
    bool stopped;
};

//struct sc_delay_buffer_callbacks {
//    bool (*on_new_frame)(struct sc_delay_buffer* db, const avframe* frame,
//        void* userdata);
//};

/**
 * initialize a delay buffer.
 *
 * \param delay a (strictly) positive delay
 * \param first_frame_asap if true, do not delay the first frame (useful for
                           a video stream).
 */
void
sc_delay_buffer_init(struct sc_delay_buffer* db, sc_tick delay,
    bool first_frame_asap);

#endif