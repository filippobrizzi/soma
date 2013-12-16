#include "opencv2/highgui/highgui.hpp"
#include <iostream>
//g++ `pkg-config --cflags --libs opencv`  opencv1.cpp -o opencv1.o
using namespace cv;
using namespace std;

int main( int argc, const char** argv ){

    VideoCapture cap(0); // open the video camera no. 0

    if (!cap.isOpened()){
         cout << "ERROR: Cannot open the video file" << endl;
         return -1;
    }

    namedWindow("MyVideo", CV_WINDOW_AUTOSIZE); //create a window called "MyVideo"

    double Width = cap.get(CV_CAP_PROP_FRAME_WIDTH); //get the width of frames of the video
    double Height = cap.get(CV_CAP_PROP_FRAME_HEIGHT); //get the height of frames of the video

    Size frameSize(static_cast<int>(Width), static_cast<int>(Height));

    /*VideoWriter oVideoWriter ("./MyVideo_new.avi", CV_FOURCC('P','I','M','1'), 20, frameSize, true); //initialize the VideoWriter object 

    if ( !oVideoWriter.isOpened() ) //if not initialize the VideoWriter successfully, exit the program
    {
         cout << "ERROR: Failed to write the video" << endl;
         return -1;
    }*/

    Mat frame;

    while (1){
 
         bool bSuccess = cap.read(frame); // read a new frame from video
    
         //oVideoWriter.write(frame); //writer the frame into the file

         if (!bSuccess){
              break;
         }
         waitKey(30);

         imshow("MyVideo", frame); //show the frame in "MyVideo" window
    }

return 0;
}