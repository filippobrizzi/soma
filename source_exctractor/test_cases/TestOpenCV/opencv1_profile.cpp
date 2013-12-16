 #include "opencv2/highgui/highgui.hpp"

#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/core/core.hpp"
#include <iostream>
#include <omp.h>

using namespace cv;
using namespace std;

#include "ProfileTracker.h"
int apply_filter_1(Mat &frame){
if( ProfileTracker x = ProfileTrackParams(11, 0)) {
    int count = frame.cols;
//    #pragma omp parallel for
    if( ProfileTracker x = ProfileTrackParams(11, 14, count - 0))
    for (int i = 0; i < count; ++i)
    {
        Size gaussian_size(0, 0);
        GaussianBlur(frame.col(i), frame.col(i), gaussian_size, 3);     
    }  

    return 0;
}
};

int apply_filter_2(Mat &frame){
if( ProfileTracker x = ProfileTrackParams(23, 0)) {
    
    int count = frame.cols;
//    #pragma omp parallel for
    if( ProfileTracker x = ProfileTrackParams(23, 27, count - 0))
    for (int i = 0; i < count; ++i)
    {        
        erode(frame.col(i), frame.col(i), Mat());
    }  
    return 0;
}
};


int main(int argc, char* argv[])
{
if( ProfileTracker x = ProfileTrackParams(35, 0)) {
    //VideoCapture video_cap_sx("MyVideo_sx.avi"); // open the video file for reading
    //VideoCapture video_cap_dx("MyVideo_dx.avi"); // open the video file for reading

//    #pragma omp parallel
    if( ProfileTracker x = ProfileTrackParams(35, 41))
    {
//        #pragma omp sections
        if( ProfileTracker x = ProfileTrackParams(35, 43))
        {
            
//            #pragma omp section
            if( ProfileTracker x = ProfileTrackParams(35, 46))
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
                    
                    //apply_filter_2(frame);
                        
                    oVideoWriter_sx.write(frame);
                    imshow("MyVideo_sx", frame); //show the frame in "MyVideo" window

                    waitKey(1/fps*100);
                }
            }

//            #pragma omp section
            if( ProfileTracker x = ProfileTrackParams(35, 74))
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
                    
                    //apply_filter_1(frame);
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