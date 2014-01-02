 #include "opencv2/highgui/highgui.hpp"

#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/core/core.hpp"
#include <iostream>
#include <omp.h>

using namespace cv;
using namespace std;

#define video_name_sx "mandelbrot_sx"
#define dest_video_sx "./mandelbrot_new_sx"
#define video_name_dx "mandelbrot_dx" 
#define dest_video_dx "./mandelbrot_new_dx"

#include "profile_tracker/profile_tracker.h"
int apply_filter_1(const Mat &frame){
if( ProfileTracker x = ProfileTrackParams(16, 0)) {
    int count = frame.cols;
//    #pragma omp parallel for
    if( ProfileTracker x = ProfileTrackParams(16, 19, count - 0))
    for (int i = 0; i < count; ++i)
    {
        Size gaussian_size(0, 0);
        GaussianBlur(frame.col(i), frame.col(i), gaussian_size, 3);
        medianBlur(frame.col(i), frame.col(i), 7);
        erode(frame.col(i), frame.col(i), 1000);

        //GaussianBlur(frame.col(i), frame.col(i), gaussian_size, 3);
        //medianBlur(frame.col(i), frame.col(i), 7);
    }  

    return 0;
}
};

int apply_filter_2(const Mat &frame){
if( ProfileTracker x = ProfileTrackParams(33, 0)) {
    
    int count = frame.cols;
//    #pragma omp parallel for
    if( ProfileTracker x = ProfileTrackParams(33, 37, count - 0))
    for (int i = 0; i < count; ++i)
    {        
        //blur(frame.col(i), frame.col(i), Size());
        Size gaussian_size(0, 0);
        GaussianBlur(frame.col(i), frame.col(i), gaussian_size, 3);
        medianBlur(frame.col(i), frame.col(i), 7);
        erode(frame.col(i), frame.col(i), 1000);
        //blur(frame.col(i), frame.col(i), Size());
        //erode(frame.col(i), frame.col(i), 1000);
        
    }  
    return 0;
}
};


int main(int argc, char* argv[]) {
if( ProfileTracker x = ProfileTrackParams(52, 0)) {
    //VideoCapture video_cap_sx("MyVideo_sx.avi"); // open the video file for reading
    //VideoCapture video_cap_dx("MyVideo_dx.avi"); // open the video file for reading
       //XInitThreads();
//    #pragma omp parallel
    if( ProfileTracker x = ProfileTrackParams(52, 57))
    {
//        #pragma omp sections
        if( ProfileTracker x = ProfileTrackParams(52, 59))
        {
            
//            #pragma omp section
            if( ProfileTracker x = ProfileTrackParams(52, 62))
            {   
                VideoCapture video_cap_sx("/home/pippo/Documents/Project/soma/source_exctractor/test_cases/TestOpenCV/mandelbrot1_sx.avi"); // open the video file for reading
                double dWidth = video_cap_sx.get(CV_CAP_PROP_FRAME_WIDTH); //get the width of frames of the video
                double dHeight = video_cap_sx.get(CV_CAP_PROP_FRAME_HEIGHT); //get the height of frames of the video
                Size frameSize(static_cast<int>(dWidth), static_cast<int>(dHeight));
                VideoWriter oVideoWriter_sx ("/home/pippo/Documents/Project/soma/source_exctractor/test_cases/TestOpenCV/mandelbrot_new_sx.avi", CV_FOURCC('P','I','M','1'), 20, frameSize, true); //initialize the VideoWriter object 
                //namedWindow("MyVideo_sx",CV_WINDOW_AUTOSIZE); //create a window called "MyVideo"
                double fps = video_cap_sx.get(CV_CAP_PROP_FPS); //get the frames per seconds of the video
                int count = 0;
                while(1)
                {
                    Mat frame;

                    bool frame_success = video_cap_sx.read(frame); // read a new frame from video
                    if (!frame_success) break;

                    apply_filter_1(frame);
                    
                    //apply_filter_2(frame);
                        
                    oVideoWriter_sx.write(frame);
                    std::cout << "sx -- " << count << std::endl;
                    //imshow("MyVideo_sx", frame); //show the frame in "MyVideo" window

                    waitKey(1/fps*100);
                }
            }

//            #pragma omp section
            if( ProfileTracker x = ProfileTrackParams(52, 91))
            {
                VideoCapture video_cap_dx("/home/pippo/Documents/Project/soma/source_exctractor/test_cases/TestOpenCV/mandelbrot1_dx.avi"); // open the video file for reading
                double dWidth = video_cap_dx.get(CV_CAP_PROP_FRAME_WIDTH); //get the width of frames of the video
                double dHeight = video_cap_dx.get(CV_CAP_PROP_FRAME_HEIGHT); //get the height of frames of the video
                Size frameSize(static_cast<int>(dWidth), static_cast<int>(dHeight));
                VideoWriter oVideoWriter_dx ("/home/pippo/Documents/Project/soma/source_exctractor/test_cases/TestOpenCV/mandelbrot_new_dx.avi", CV_FOURCC('P','I','M','1'), 20, frameSize, true); //initialize the VideoWriter object 
                //namedWindow("MyVideo_dx",CV_WINDOW_AUTOSIZE); //create a window called "MyVideo"
                double fps = video_cap_dx.get(CV_CAP_PROP_FPS); //get the frames per seconds of the video
                int count = 0;

                while(1)
                {
                    Mat frame;

                    bool frame_success = video_cap_dx.read(frame); // read a new frame from video
                    if (!frame_success) break;
                    
                    //apply_filter_1(frame);
                    apply_filter_2(frame);

                    oVideoWriter_dx.write(frame);
                    //imshow("MyVideo_dx", frame); //show the frame in "MyVideo" window
                    std::cout << "dx -- " << count << std::endl;

                    waitKey(1/fps*100);
                }
            }
        }
    }
    return 0;

}
}