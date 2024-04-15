#include "decoder.h"
#include <opencv2/opencv.hpp>
using namespace cv;
void showContinuousImage(Mat& image) {
   
        // Display the image using the fully qualified namespace
        imshow("Image", image);

        // Wait for 30 milliseconds, check if 'q' or ESC is pressed to exit
        char key = waitKey(30);
        if (key == 'q' || key == 27) {  // 'q' key or ESC key
            break;
        }
    
}