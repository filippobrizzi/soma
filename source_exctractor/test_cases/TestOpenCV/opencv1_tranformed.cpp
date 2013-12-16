#include "opencv2/highgui/highgui.hpp"

#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/core/core.hpp"
#include <iostream>
#include <omp.h>

using namespace cv;
using namespace std;

#include "/home/pippo/Documents/Project/soma/source_exctractor/src/thread_pool/threads_pool.h"
int apply_filter_1(const Mat &frame){
    int count = frame.cols;
//    #pragma omp parallel for
    //for (int i = 0; i < count; ++i)
{
  class Nested : public NestedBase {
  public: 
    virtual std::shared_ptr<NestedBase> clone() const { return std::make_shared<Nested>(*this); } 
    Nested(int pragma_id, int & count, class cv::Mat & & frame)  : NestedBase(pragma_id), count_(count) , frame_(frame) {}
int & count_;
class cv::Mat & & frame_;

void fx(ForParameter for_param, int & count, class cv::Mat & & frame) {
for(int i = 0 + for_param.thread_id_*(count - 0)/for_param.num_threads_; i < 0 + (for_param.thread_id_ + 1)*(count - 0)/for_param.num_threads_; i ++ )
    {
        Size gaussian_size(0, 0);
        GaussianBlur(frame.col(i), frame.col(i), gaussian_size, 3);     
    }  
}
void callme(ForParameter for_param) {
fx(for_param, count_, frame_);
}
};
ThreadPool::getInstance("test_cases/TestOpenCV/opencv1.cpp")->call(std::make_shared<Nested>(14, count, frame));
}

    return 0;
};

int apply_filter_2(const Mat &frame){
    
    int count = frame.cols;
//    #pragma omp parallel for
    //for (int i = 0; i < count; ++i)
{
  class Nested : public NestedBase {
  public: 
    virtual std::shared_ptr<NestedBase> clone() const { return std::make_shared<Nested>(*this); } 
    Nested(int pragma_id, int & count, class cv::Mat & & frame)  : NestedBase(pragma_id), count_(count) , frame_(frame) {}
int & count_;
class cv::Mat & & frame_;

void fx(ForParameter for_param, int & count, class cv::Mat & & frame) {
for(int i = 0 + for_param.thread_id_*(count - 0)/for_param.num_threads_; i < 0 + (for_param.thread_id_ + 1)*(count - 0)/for_param.num_threads_; i ++ )
    {        
        erode(frame.col(i), frame.col(i), Mat());
    }  
}
void callme(ForParameter for_param) {
fx(for_param, count_, frame_);
}
};
ThreadPool::getInstance("test_cases/TestOpenCV/opencv1.cpp")->call(std::make_shared<Nested>(27, count, frame));
}
    return 0;
};


int main(int argc, char* argv[])
{

//    #pragma omp parallel
    {
      class Nested : public NestedBase {
      public: 
        virtual std::shared_ptr<NestedBase> clone() const { return std::make_shared<Nested>(*this); } 
        Nested(int pragma_id)  : NestedBase(pragma_id){}
    
    void fx(ForParameter for_param){
//        #pragma omp sections
        {
          class Nested : public NestedBase {
          public: 
            virtual std::shared_ptr<NestedBase> clone() const { return std::make_shared<Nested>(*this); } 
            Nested(int pragma_id)  : NestedBase(pragma_id){}
        
        void fx(ForParameter for_param){
            
//            #pragma omp section
            {
              class Nested : public NestedBase {
              public: 
                virtual std::shared_ptr<NestedBase> clone() const { return std::make_shared<Nested>(*this); } 
                Nested(int pragma_id)  : NestedBase(pragma_id){}
            
            void fx(ForParameter for_param){   
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
void callme(ForParameter for_param) {
fx(for_param);
}
};
ThreadPool::getInstance("test_cases/TestOpenCV/opencv1.cpp")->call(std::make_shared<Nested>(44));
}

//            #pragma omp section
            {
              class Nested : public NestedBase {
              public: 
                virtual std::shared_ptr<NestedBase> clone() const { return std::make_shared<Nested>(*this); } 
                Nested(int pragma_id)  : NestedBase(pragma_id){}
            
            void fx(ForParameter for_param){
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
void callme(ForParameter for_param) {
fx(for_param);
}
};
ThreadPool::getInstance("test_cases/TestOpenCV/opencv1.cpp")->call(std::make_shared<Nested>(72));
}
        }
void callme(ForParameter for_param) {
fx(for_param);
}
};
ThreadPool::getInstance("test_cases/TestOpenCV/opencv1.cpp")->call(std::make_shared<Nested>(41));
}
    }
void callme(ForParameter for_param) {
fx(for_param);
}
};
ThreadPool::getInstance("test_cases/TestOpenCV/opencv1.cpp")->call(std::make_shared<Nested>(39));
}
    return 0;

}