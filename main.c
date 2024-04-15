#include <stdio.h>
#include "events.h"
#include "util/ticks.h"
#include "compat.h"
#include "util/log.h"
#include "util/net.h"
#include "demuxer.h"
#include "decoder.h"
//#include "delay_buffer.h"
#include "screen.h"
#include "options.h"


#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 1234
bool video_demuxer_started = true;
int main() {
    if (!net_init()) {
        fprintf(stderr, "Winsock initialization failed\n");
        exit(EXIT_FAILURE);
    }

    // Create socket
    SOCKET sockfd = net_socket();
    if (sockfd == SC_SOCKET_NONE) {
        fprintf(stderr, "Socket creation failed\n");
        net_cleanup();
        exit(EXIT_FAILURE);
    }

    uint32_t server_addr;
    if (!net_parse_ipv4(SERVER_IP, &server_addr)) {
        fprintf(stderr, "Failed to parse server IP address\n");
        net_cleanup();
        exit(EXIT_FAILURE);
    }

    // Connect to the server
    if (!net_connect(sockfd, server_addr, SERVER_PORT)) {
        fprintf(stderr, "Connection to server failed\n");
        net_cleanup();
        exit(EXIT_FAILURE);
    }

    
    struct sc_demuxer demuxer;
    struct sc_decoder decoder;
    struct sc_screen screen;
   // struct scrcpy_options* options;
    //struct sc_delay_buffer display_buffer;
    sc_demuxer_init(&demuxer, "video_demuxer", sockfd);
    sc_decoder_init(&decoder, "video");
    sc_packet_source_add_sink(&demuxer.packet_source, &decoder.packet_sink);
    sc_demuxer_start(&demuxer);


    const char* window_title = "TCRCPY";
  /*  sc_delay_buffer_init(&display_buffer,1000,
        true);*/
    //sc_frame_source_add_sink(src, &display_buffer.frame_sink);
    //src = &display_buffer.frame_source;
    struct sc_screen_params screen_params = {
           .window_title = window_title,
           //  .always_on_top = options->always_on_top,
             .window_x = SC_WINDOW_POSITION_UNDEFINED,
             .window_y = SC_WINDOW_POSITION_UNDEFINED,
             .window_width = 864,
             .window_height = 1920,
           .window_borderless = false,
           .orientation = SC_ORIENTATION_0,
           .mipmaps = false,
           .fullscreen = false,
          // .start_fps_counter = options->start_fps_counter,
    };
    struct sc_frame_source* src = &decoder.frame_source;

    if (!sc_screen_init(&screen, &screen_params)) {
        LOGE("Screen Initialization Failed");
        return -1;
    }
    bool screen_initialized = true;

    sc_frame_source_add_sink(src, &screen.frame_sink);

    
  
    
     //   sc_screen_interrupt(&screen);
 end:
    sc_demuxer_join(&demuxer);
    sc_screen_interrupt(&screen);
    sc_screen_join(&screen);
    sc_screen_destroy(&screen);
 
    
    net_cleanup();
    return 0;
}