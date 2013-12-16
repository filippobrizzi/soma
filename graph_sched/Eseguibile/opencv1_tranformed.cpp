#include "opencv2/highgui/highgui.hpp"
#include <iostream>
//#include <omp.h>

using namespace cv;
using namespace std;



#include "threads_pool.h"
int main(int argc, char* argv[])
{
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
    {
      class Nested : public NestedBase {
      public: 
        virtual std::shared_ptr<NestedBase> clone() const { return std::make_shared<Nested>(*this); } 
        Nested(int pragma_id, cv::VideoCapture & video_cap_sx, double & fps, cv::VideoCapture & video_cap_dx)  : NestedBase(pragma_id), video_cap_sx_(video_cap_sx) , fps_(fps) , video_cap_dx_(video_cap_dx) {}
    cv::VideoCapture & video_cap_sx_;
    double & fps_;
    cv::VideoCapture & video_cap_dx_;
    
    void fx(ForParameter for_param, cv::VideoCapture & video_cap_sx, double & fps, cv::VideoCapture & video_cap_dx){
//        #pragma omp sections
        {
          class Nested : public NestedBase {
          public: 
            virtual std::shared_ptr<NestedBase> clone() const { return std::make_shared<Nested>(*this); } 
            Nested(int pragma_id, cv::VideoCapture & video_cap_sx, double & fps, cv::VideoCapture & video_cap_dx)  : NestedBase(pragma_id), video_cap_sx_(video_cap_sx) , fps_(fps) , video_cap_dx_(video_cap_dx) {}
        cv::VideoCapture & video_cap_sx_;
        double & fps_;
        cv::VideoCapture & video_cap_dx_;
        
        void fx(ForParameter for_param, cv::VideoCapture & video_cap_sx, double & fps, cv::VideoCapture & video_cap_dx){
            
//            #pragma omp section
            {
              class Nested : public NestedBase {
              public: 
                virtual std::shared_ptr<NestedBase> clone() const { return std::make_shared<Nested>(*this); } 
                Nested(int pragma_id, cv::VideoCapture & video_cap_sx, double & fps)  : NestedBase(pragma_id), video_cap_sx_(video_cap_sx) , fps_(fps) {}
            cv::VideoCapture & video_cap_sx_;
            double & fps_;
            
            void fx(ForParameter for_param, cv::VideoCapture & video_cap_sx, double & fps){
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
void callme(ForParameter for_param) {
fx(for_param, video_cap_sx_, fps_);
}
};
ThreadPool::getInstance("../../TestOpencv/opencv1.cpp")->call(std::make_shared<Nested>(37, video_cap_sx, fps));
}

//            #pragma omp section
            {
              class Nested : public NestedBase {
              public: 
                virtual std::shared_ptr<NestedBase> clone() const { return std::make_shared<Nested>(*this); } 
                Nested(int pragma_id, cv::VideoCapture & video_cap_dx, double & fps)  : NestedBase(pragma_id), video_cap_dx_(video_cap_dx) , fps_(fps) {}
            cv::VideoCapture & video_cap_dx_;
            double & fps_;
            
            void fx(ForParameter for_param, cv::VideoCapture & video_cap_dx, double & fps){
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
void callme(ForParameter for_param) {
fx(for_param, video_cap_dx_, fps_);
}
};
ThreadPool::getInstance("../../TestOpencv/opencv1.cpp")->call(std::make_shared<Nested>(54, video_cap_dx, fps));
}
        }
void callme(ForParameter for_param) {
fx(for_param, video_cap_sx_, fps_, video_cap_dx_);
}
};
ThreadPool::getInstance("../../TestOpencv/opencv1.cpp")->call(std::make_shared<Nested>(34, video_cap_sx, fps, video_cap_dx));
}
    }
void callme(ForParameter for_param) {
fx(for_param, video_cap_sx_, fps_, video_cap_dx_);
}
};
ThreadPool::getInstance("../../TestOpencv/opencv1.cpp")->call(std::make_shared<Nested>(32, video_cap_sx, fps, video_cap_dx));
}
    return 0;

}
