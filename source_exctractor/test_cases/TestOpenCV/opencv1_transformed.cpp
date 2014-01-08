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

#include "thread_pool/threads_pool.h"
int apply_filter_1(const Mat &frame){
    int count = frame.cols;
//    #pragma omp parallel for
    //for (int i = 0; i < count; ++i)
{
  class Nested : public NestedBase {
  public: 
    virtual std::shared_ptr<NestedBase> clone() const { return std::make_shared<Nested>(*this); } 
    Nested(int pragma_id, int & count, const cv::Mat & frame)  : NestedBase(pragma_id), count_(count) , frame_(frame) {}
int & count_;
const cv::Mat & frame_;

void fx(ForParameter for_param, int & count, const cv::Mat & frame) {
for(int i = 0 + for_param.thread_id_*(count - 0)/for_param.num_threads_; i < 0 + (for_param.thread_id_ + 1)*(count - 0)/for_param.num_threads_; i ++ )
    {
        Size gaussian_size(0, 0);
        GaussianBlur(frame.col(i), frame.col(i), gaussian_size, 3);
        medianBlur(frame.col(i), frame.col(i), 7);
        erode(frame.col(i), frame.col(i), 1000);

        //GaussianBlur(frame.col(i), frame.col(i), gaussian_size, 3);
        //medianBlur(frame.col(i), frame.col(i), 7);
    }  
launch_todo_job(); 
 }
void callme(ForParameter for_param) {
  fx(for_param, count_, frame_);
}
};
std::shared_ptr<NestedBase> nested_b = std::make_shared<Nested>(19, count, frame);
ThreadPool::getInstance("source_exctractor/test_cases/TestOpenCV/opencv1.cpp")->call(nested_b);
  //todo_job_.push(nested_b); 
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
    Nested(int pragma_id, int & count, const cv::Mat & frame)  : NestedBase(pragma_id), count_(count) , frame_(frame) {}
int & count_;
const cv::Mat & frame_;

void fx(ForParameter for_param, int & count, const cv::Mat & frame) {
for(int i = 0 + for_param.thread_id_*(count - 0)/for_param.num_threads_; i < 0 + (for_param.thread_id_ + 1)*(count - 0)/for_param.num_threads_; i ++ )
    {        
        //blur(frame.col(i), frame.col(i), Size());
        Size gaussian_size(0, 0);
        GaussianBlur(frame.col(i), frame.col(i), gaussian_size, 3);
        medianBlur(frame.col(i), frame.col(i), 7);
        erode(frame.col(i), frame.col(i), 1000);
        //blur(frame.col(i), frame.col(i), Size());
        //erode(frame.col(i), frame.col(i), 1000);
        
    }  
launch_todo_job(); 
 }
void callme(ForParameter for_param) {
  fx(for_param, count_, frame_);
}
};
std::shared_ptr<NestedBase> nested_b = std::make_shared<Nested>(37, count, frame);
ThreadPool::getInstance("source_exctractor/test_cases/TestOpenCV/opencv1.cpp")->call(nested_b);
  //todo_job_.push(nested_b); 
}
    return 0;
};


int main(int argc, char* argv[]) {
    //VideoCapture video_cap_sx("MyVideo_sx.avi"); // open the video file for reading
    //VideoCapture video_cap_dx("MyVideo_dx.avi"); // open the video file for reading
       //XInitThreads();
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
            launch_todo_job(); 
}
void callme(ForParameter for_param) {
  fx(for_param);
}
};
std::shared_ptr<NestedBase> nested_b = std::make_shared<Nested>(62);
if(ThreadPool::getInstance("source_exctractor/test_cases/TestOpenCV/opencv1.cpp")->call(nested_b)) 
  todo_job_.push(nested_b); 
}

//            #pragma omp section
            {
              class Nested : public NestedBase {
              public: 
                virtual std::shared_ptr<NestedBase> clone() const { return std::make_shared<Nested>(*this); } 
                Nested(int pragma_id)  : NestedBase(pragma_id){}
            
            void fx(ForParameter for_param){
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
            launch_todo_job(); 
}
void callme(ForParameter for_param) {
  fx(for_param);
}
};
std::shared_ptr<NestedBase> nested_b = std::make_shared<Nested>(91);
if(ThreadPool::getInstance("source_exctractor/test_cases/TestOpenCV/opencv1.cpp")->call(nested_b)) 
  todo_job_.push(nested_b); 
}
        launch_todo_job(); 
}
void callme(ForParameter for_param) {
  fx(for_param);
}
};
std::shared_ptr<NestedBase> nested_b = std::make_shared<Nested>(59);
if(ThreadPool::getInstance("source_exctractor/test_cases/TestOpenCV/opencv1.cpp")->call(nested_b)) 
  todo_job_.push(nested_b); 
}
    launch_todo_job(); 
}
void callme(ForParameter for_param) {
  fx(for_param);
}
};
std::shared_ptr<NestedBase> nested_b = std::make_shared<Nested>(57);
ThreadPool::getInstance("source_exctractor/test_cases/TestOpenCV/opencv1.cpp")->call(nested_b);
//  todo_job_.push(nested_b); 
}
    return 0;

}