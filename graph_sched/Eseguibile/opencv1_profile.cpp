#include "opencv2/highgui/highgui.hpp"
#include <iostream>
#include <omp.h>

using namespace cv;
using namespace std;



#include "/home/pippo/Documents/Library/clomp-master/include/myprogram/profiling/ProfileTracker.h"
int main(int argc, char* argv[])
{
if( ProfileTracker x = ProfileTrackParams(11, 0)) {
    VideoCapture video_cap_sx("MyVideo_sx.avi"); // open the video file for reading
    VideoCapture video_cap_dx("MyVideo_dx.avi");

    if ( !video_cap_sx.isOpened() || !video_cap_dx.isOpened())  // if not success, exit program
    {
         cout << "Cannot open the video file" << endl;
         return -1;
    }

    //cap.set(CV_CAP_PROP_POS_MSEC, 300); //start the video at 300ms

    double fps = video_cap_sx.get(CV_CAP_PROP_FPS); //get the frames per seconds of the video

     cout << "Frame per seconds : " << fps << endl;

    namedWindow("MyVideo_sx",CV_WINDOW_AUTOSIZE); //create a window called "MyVideo"
    namedWindow("MyVideo_dx",CV_WINDOW_AUTOSIZE); //create a window called "MyVideo"


//    #pragma omp parallel
    if( ProfileTracker x = ProfileTrackParams(10, 32))
    {
//        #pragma omp sections
        if( ProfileTracker x = ProfileTrackParams(10, 34))
        {
            
//            #pragma omp section
            if( ProfileTracker x = ProfileTrackParams(10, 37))
            {
                    while(1)
                        {
                        Mat frame;

                        bool frame_success = video_cap_sx.read(frame); // read a new frame from video
                        if (!frame_success) //if not success, break loop
                            break;


                        imshow("MyVideo_sx", frame); //show the frame in "MyVideo" window

                        waitKey(1/fps*1000);
                    }
            }

//            #pragma omp section
            if( ProfileTracker x = ProfileTrackParams(10, 54))
            {
                    while(1)
                    {
                        Mat frame;

                        bool frame_success = video_cap_dx.read(frame); // read a new frame from video
                        if (!frame_success) //if not success, break loop
                            break;


                        imshow("MyVideo_dx", frame); //show the frame in "MyVideo" window

                        waitKey(1/fps*1000);
                    }
            }
        }
    }
    return 0;

}
}