#include "opencv2/highgui/highgui.hpp"

#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/core/core.hpp"
#include <iostream>
#include <omp.h>

using namespace cv;
using namespace std;

#include "/home/pippo/Documents/Library/clomp-master/include/myprogram/profiling/ProfileTracker.h"
int apply_filter_1(const Mat &frame){
if( ProfileTracker x = ProfileTrackParams(11, 0)) {
//    #pragma omp parallel for
    if( ProfileTracker x = ProfileTrackParams(11, 13,  - 0))
    for (int i = 0; i < frame.cols; ++i)
    {
        Size gaussian_size(0, 0);
        GaussianBlur(frame.col(i), frame.col(i), gaussian_size, 3);     
    }  

    return 0;
}
};

int apply_filter_2(const Mat &frame){
if( ProfileTracker x = ProfileTrackParams(22, 0)) {
    
    
//    #pragma omp parallel for
    if( ProfileTracker x = ProfileTrackParams(22, 26,  - 0))
    for (int i = 0; i < frame.cols; ++i)
    {        
        erode(frame.col(i), frame.col(i), Mat());
    }  
    return 0;
}
};


int main(int argc, char* argv[])
{
if( ProfileTracker x = ProfileTrackParams(35, 0)) {

//    #pragma omp parallel
    if( ProfileTracker x = ProfileTrackParams(34, 38))
    {
//        #pragma omp sections
        if( ProfileTracker x = ProfileTrackParams(34, 40))
        {
            
//            #pragma omp section
            if( ProfileTracker x = ProfileTrackParams(34, 43))
            {   
                VideoCapture video_cap_sx("MyVideo_sx.avi"); // open the video file for reading
                double dWidth = video_cap_sx.get(CV_CAP_PROP_FRAME_WIDTH); //get the width of frames of the video
                double dHeight = video_cap_sx.get(CV_CAP_PROP_FRAME_HEIGHT); //get the height of frames of the video
                Size frameSize(static_cast<int>(dWidth), static_cast<int>(dHeight));
                VideoWriter oVideoWriter_sx ("./MyVideo_sx_new.avi", CV_FOURCC('P','I','M','1'), 20, frameSize, true); //initialize the VideoWriter object 
                namedWindow("MyVideo_sx",CV_WINDOW_AUTOSIZE); //create a window called "MyVideo"
                double fps = video_cap_sx.get(CV_CAP_PROP_FPS); //get the frames per seconds of the video

                while(1)
                {
                    Mat frame;

                    bool frame_success = video_cap_sx.read(frame); // read a new frame from video
                    if (!frame_success) break;

                    apply_filter_1(frame);
                    
                    apply_filter_2(frame);
                        
                    oVideoWriter_sx.write(frame);
                    imshow("MyVideo_sx", frame); //show the frame in "MyVideo" window

                    waitKey(1/fps*100);
                }
            }

//            #pragma omp section
            if( ProfileTracker x = ProfileTrackParams(34, 71))
            {
                VideoCapture video_cap_dx("MyVideo_dx.avi"); // open the video file for reading
                double dWidth = video_cap_dx.get(CV_CAP_PROP_FRAME_WIDTH); //get the width of frames of the video
                double dHeight = video_cap_dx.get(CV_CAP_PROP_FRAME_HEIGHT); //get the height of frames of the video
                Size frameSize(static_cast<int>(dWidth), static_cast<int>(dHeight));
                VideoWriter oVideoWriter_dx ("./MyVideo_dx_new.avi", CV_FOURCC('P','I','M','1'), 20, frameSize, true); //initialize the VideoWriter object 
                namedWindow("MyVideo_dx",CV_WINDOW_AUTOSIZE); //create a window called "MyVideo"
                double fps = video_cap_dx.get(CV_CAP_PROP_FPS); //get the frames per seconds of the video

                while(1)
                {
                    Mat frame;

                    bool frame_success = video_cap_dx.read(frame); // read a new frame from video
                    if (!frame_success) break;
                    
                    apply_filter_1(frame);
                    apply_filter_2(frame);

                    oVideoWriter_dx.write(frame);
                    imshow("MyVideo_dx", frame); //show the frame in "MyVideo" window

                    waitKey(1/fps*100);
                }
            }
        }
    }
    return 0;

}
}