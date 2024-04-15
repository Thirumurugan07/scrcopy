#include <opencv2/opencv.hpp>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include "decoder.h"
////// Function to convert AVFrame to OpenCV Mat
////cv::Mat avframe_to_mat(const AVFrame* frame) {
////    int width = frame->width;
////    int height = frame->height;
////    int channels = 3; // Assuming BGR format
////
////    // Allocate memory for the output image in BGR format
////    cv::Mat image(height, width, CV_8UC3);
////
////    // Create a SwsContext for the conversion
////    SwsContext* swsctx = sws_getContext(width, height, AV_PIX_FMT_YUV420P,
////        width, height, AV_PIX_FMT_BGR24,
////        SWS_BICUBIC, nullptr, nullptr, nullptr);
////
////    // Perform pixel format conversion
////    sws_scale(swsctx, frame->data, frame->linesize, 0, height,
////        reinterpret_cast<uint8_t**>(image.data), image.step);
////
////    // Release SwsContext
////    sws_freeContext(swsctx);
////
////    return image;
////}
//
//// Function to save OpenCV Mat as .bin file
//void save_mat_as_bin(const cv::Mat& image, const char* filename) {
//    // Open the file for writing in binary mode
//    FILE* file = fopen(filename, "wb");
//    if (!file) {
//        printf("Error: Could not open file for writing.\n");
//        return;
//    }
//
//    // Write the image data to the binary file
//    fwrite(image.data, sizeof(unsigned char), image.total() * image.elemSize(), file);
//
//    // Close the file
//    fclose(file);
//}
//
