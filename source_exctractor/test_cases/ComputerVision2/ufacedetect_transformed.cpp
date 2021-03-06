#include "opencv2/objdetect.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/core/utility.hpp"
#include "opencv2/core/ocl.hpp"
#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#include <omp.h>
#include <cctype>
#include <iostream>
#include <iterator>
#include <stdio.h>

using namespace std;
using namespace cv;

#include "thread_pool/threads_pool.h"
void detectAndDraw_sx( UMat& img, Mat& canvas, CascadeClassifier& cascade, double scale);
void detectAndDraw_dx( UMat& img, Mat& canvas, CascadeClassifier& cascade, double scale);
void dx(UMat* img, Mat* canvas, CascadeClassifier* cascade, double scale0, int i);
void sx(UMat* img, Mat* canvas, CascadeClassifier* cascade, double scale0, int i);


string cascadeName = "/home/pippo/Documents/Project/soma/source_exctractor/test_cases/ComputerVision2/haarcascade_frontalface_alt.xml";

int main( int argc, const char** argv ){
   
    String inputName;

    VideoCapture capture_dx;
    VideoCapture capture_sx;
    capture_dx.open("/home/pippo/Documents/Project/soma/source_exctractor/test_cases/ComputerVision2/MyVideo_dx.avi");
    capture_sx.open("/home/pippo/Documents/Project/soma/source_exctractor/test_cases/ComputerVision2/MyVideo_sx.avi");

    if(!capture_dx.isOpened() || !capture_sx.isOpened()) {
        cout << "VIDEO NOT OPENED" << endl;
        return -1;

    }
    //omp_set_num_threads(2);
    //omp_set_nested(1);
//    #pragma omp parallel
    {
      class Nested : public NestedBase {
      public: 
        virtual std::shared_ptr<NestedBase> clone() const { return std::make_shared<Nested>(*this); } 
        Nested(int pragma_id,  cv::VideoCapture & capture_sx,  cv::VideoCapture & capture_dx)  : NestedBase(pragma_id), capture_sx_(capture_sx) , capture_dx_(capture_dx) {}
     cv::VideoCapture & capture_sx_;
     cv::VideoCapture & capture_dx_;
    
    void fx(ForParameter for_param,  cv::VideoCapture & capture_sx,  cv::VideoCapture & capture_dx){
//        #pragma omp sections
        {
          class Nested : public NestedBase {
          public: 
            virtual std::shared_ptr<NestedBase> clone() const { return std::make_shared<Nested>(*this); } 
            Nested(int pragma_id,  cv::VideoCapture & capture_sx,  cv::VideoCapture & capture_dx)  : NestedBase(pragma_id), capture_sx_(capture_sx) , capture_dx_(capture_dx) {}
         cv::VideoCapture & capture_sx_;
         cv::VideoCapture & capture_dx_;
        
        void fx(ForParameter for_param,  cv::VideoCapture & capture_sx,  cv::VideoCapture & capture_dx){
//            #pragma omp section
            {
              class Nested : public NestedBase {
              public: 
                virtual std::shared_ptr<NestedBase> clone() const { return std::make_shared<Nested>(*this); } 
                Nested(int pragma_id,  cv::VideoCapture & capture_sx)  : NestedBase(pragma_id), capture_sx_(capture_sx) {}
             cv::VideoCapture & capture_sx_;
            
            void fx(ForParameter for_param,  cv::VideoCapture & capture_sx){

                UMat *frame_sx = new UMat[4]; 
                UMat image_sx;
                Mat *canvas_sx = new Mat[4];
                double scale_sx = 1;

                CascadeClassifier *cascade_sx = new CascadeClassifier[4];

                cout << "Video sx capturing has been started ..." << endl;

                for(int i = 0; i < 4; i ++)
                    cascade_sx[i].load(cascadeName);

                for(int i = 0 ; i < 10; i ++) {
                    
                    for(int j = 0; j < 4; j ++)
                        capture_sx >> frame_sx[j];

                    sx(frame_sx, canvas_sx, cascade_sx, scale_sx, i);
                }
            launch_todo_job(); 
}
void callme(ForParameter for_param) {
  fx(for_param, capture_sx_);
}
};
std::shared_ptr<NestedBase> nested_b = std::make_shared<Nested>(47, capture_sx);
if(ThreadPool::getInstance("source_exctractor/test_cases/ComputerVision2/ufacedetect.cpp")->call(nested_b)) 
  todo_job_.push(nested_b); 
}
//            #pragma omp section
            {
              class Nested : public NestedBase {
              public: 
                virtual std::shared_ptr<NestedBase> clone() const { return std::make_shared<Nested>(*this); } 
                Nested(int pragma_id,  cv::VideoCapture & capture_dx)  : NestedBase(pragma_id), capture_dx_(capture_dx) {}
             cv::VideoCapture & capture_dx_;
            
            void fx(ForParameter for_param,  cv::VideoCapture & capture_dx){
                UMat *frame_dx = new UMat[4];
                UMat image_dx;
                Mat *canvas_dx = new Mat[4];
                double scale_dx = 1;

                CascadeClassifier *cascade_dx = new CascadeClassifier[4];

                cout << "Video dx capturing has been started ..." << endl;

                for(int i = 0; i < 4; i ++)
                    cascade_dx[i].load(cascadeName);

                for(int i = 0 ; i < 10; i ++) {
                    
                    for(int j = 0; j < 4; j ++)
                        capture_dx >> frame_dx[j];

                    dx(frame_dx, canvas_dx, cascade_dx, scale_dx, i);
                }
            launch_todo_job(); 
}
void callme(ForParameter for_param) {
  fx(for_param, capture_dx_);
}
};
std::shared_ptr<NestedBase> nested_b = std::make_shared<Nested>(70, capture_dx);
if(ThreadPool::getInstance("source_exctractor/test_cases/ComputerVision2/ufacedetect.cpp")->call(nested_b)) 
  todo_job_.push(nested_b); 
}
        launch_todo_job(); 
}
void callme(ForParameter for_param) {
  fx(for_param, capture_sx_, capture_dx_);
}
};
std::shared_ptr<NestedBase> nested_b = std::make_shared<Nested>(45, capture_sx, capture_dx);
if(ThreadPool::getInstance("source_exctractor/test_cases/ComputerVision2/ufacedetect.cpp")->call(nested_b)) 
  todo_job_.push(nested_b); 
}
    launch_todo_job(); 
}
void callme(ForParameter for_param) {
  fx(for_param, capture_sx_, capture_dx_);
}
};
std::shared_ptr<NestedBase> nested_b = std::make_shared<Nested>(43, capture_sx, capture_dx);
if(ThreadPool::getInstance("source_exctractor/test_cases/ComputerVision2/ufacedetect.cpp")->call(nested_b)) 
  nested_b->callme(ForParameter(0,1));
}

    return 0;
}


void dx(UMat* frame_dx, Mat* canvas_dx, CascadeClassifier* cascade_dx, double scale_dx, int i) {
    //omp_set_num_threads(2);
//    #pragma omp parallel for
    //for(int j = 0; j < 4; j ++){
{
  class Nested : public NestedBase {
  public: 
    virtual std::shared_ptr<NestedBase> clone() const { return std::make_shared<Nested>(*this); } 
    Nested(int pragma_id,  cv::UMat * frame_dx,  cv::Mat * canvas_dx,  cv::CascadeClassifier * cascade_dx, double & scale_dx, int & i)  : NestedBase(pragma_id), frame_dx_(frame_dx) , canvas_dx_(canvas_dx) , cascade_dx_(cascade_dx) , scale_dx_(scale_dx) , i_(i) {}
 cv::UMat * frame_dx_;
 cv::Mat * canvas_dx_;
 cv::CascadeClassifier * cascade_dx_;
double & scale_dx_;
int & i_;

void fx(ForParameter for_param,  cv::UMat * frame_dx,  cv::Mat * canvas_dx,  cv::CascadeClassifier * cascade_dx, double & scale_dx, int & i) {
for(int j = 0 + for_param.thread_id_*(4 - 0)/for_param.num_threads_; j < 0 + (for_param.thread_id_ + 1)*(4 - 0)/for_param.num_threads_; j ++ ) { 
        detectAndDraw_dx( frame_dx[j], canvas_dx[j], cascade_dx[j], scale_dx);
        stringstream filename_dx;
        filename_dx << "images/img_" << i << "_" << j << "_dx.jpg";
        std::cout << "--------------- " << filename_dx.str() << std::endl;
        //std::cout << "---------" << omp_get_thread_num() << "------ " << filename_dx.str() << std::endl;
        //imwrite(filename_dx.str(), canvas_dx[j]);
    }
launch_todo_job(); 
 }
void callme(ForParameter for_param) {
  fx(for_param, frame_dx_, canvas_dx_, cascade_dx_, scale_dx_, i_);
}
};
std::shared_ptr<NestedBase> nested_b = std::make_shared<Nested>(101, frame_dx, canvas_dx, cascade_dx, scale_dx, i);
if(ThreadPool::getInstance("source_exctractor/test_cases/ComputerVision2/ufacedetect.cpp")->call(nested_b)) 
  nested_b->callme(ForParameter(0,1));
}

}

void sx(UMat* frame_sx, Mat* canvas_sx, CascadeClassifier* cascade_sx, double scale_sx, int i) {
    //omp_set_num_threads(2);
//    #pragma omp parallel for
    //for(int j = 0; j < 4; j ++){
{
  class Nested : public NestedBase {
  public: 
    virtual std::shared_ptr<NestedBase> clone() const { return std::make_shared<Nested>(*this); } 
    Nested(int pragma_id,  cv::UMat * frame_sx,  cv::Mat * canvas_sx,  cv::CascadeClassifier * cascade_sx, double & scale_sx, int & i)  : NestedBase(pragma_id), frame_sx_(frame_sx) , canvas_sx_(canvas_sx) , cascade_sx_(cascade_sx) , scale_sx_(scale_sx) , i_(i) {}
 cv::UMat * frame_sx_;
 cv::Mat * canvas_sx_;
 cv::CascadeClassifier * cascade_sx_;
double & scale_sx_;
int & i_;

void fx(ForParameter for_param,  cv::UMat * frame_sx,  cv::Mat * canvas_sx,  cv::CascadeClassifier * cascade_sx, double & scale_sx, int & i) {
for(int j = 0 + for_param.thread_id_*(4 - 0)/for_param.num_threads_; j < 0 + (for_param.thread_id_ + 1)*(4 - 0)/for_param.num_threads_; j ++ ) { 
        detectAndDraw_sx( frame_sx[j], canvas_sx[j], cascade_sx[j], scale_sx);
        stringstream filename_sx;
        filename_sx << "images/img_" << i << "_" << j << "_sx.jpg";
        //std::cout << "---------" << omp_get_thread_num() << " / " << omp_get_num_threads() << "------ " << filename_sx.str() << std::endl;
        std::cout << "--------------- " << filename_sx.str() << std::endl;
        //imwrite(filename_sx.str(), canvas_sx[j]);
    }
launch_todo_job(); 
 }
void callme(ForParameter for_param) {
  fx(for_param, frame_sx_, canvas_sx_, cascade_sx_, scale_sx_, i_);
}
};
std::shared_ptr<NestedBase> nested_b = std::make_shared<Nested>(115, frame_sx, canvas_sx, cascade_sx, scale_sx, i);
if(ThreadPool::getInstance("source_exctractor/test_cases/ComputerVision2/ufacedetect.cpp")->call(nested_b)) 
  nested_b->callme(ForParameter(0,1));
}
}

void detectAndDraw_dx( UMat& img, Mat& canvas, CascadeClassifier& cascade, double scale0)
{
    int i = 0;
    double t = 0, scale=1;
    vector<Rect> faces, faces2;
    const static Scalar colors[] =
    {
        Scalar(0,0,255),
        Scalar(0,128,255),
        Scalar(0,255,255),
        Scalar(0,255,0),
        Scalar(255,128,0),
        Scalar(255,255,0),
        Scalar(255,0,0),
        Scalar(255,0,255)
    };
    static UMat gray, smallImg;

    t = (double)getTickCount();

    resize( img, smallImg, Size(), scale0, scale0, INTER_LINEAR );
    cvtColor( smallImg, gray, COLOR_BGR2GRAY );
    equalizeHist( gray, gray );
    
//    #pragma omp parallel
    {
      class Nested : public NestedBase {
      public: 
        virtual std::shared_ptr<NestedBase> clone() const { return std::make_shared<Nested>(*this); } 
        Nested(int pragma_id,  cv::CascadeClassifier & cascade, vector<Rect> & faces, vector<Rect> & faces2)  : NestedBase(pragma_id), cascade_(cascade) , faces_(faces) , faces2_(faces2) {}
     cv::CascadeClassifier & cascade_;
    vector<Rect> & faces_;
    vector<Rect> & faces2_;
    
    void fx(ForParameter for_param,  cv::CascadeClassifier & cascade, vector<Rect> & faces, vector<Rect> & faces2){
//        #pragma omp sections
        {
          class Nested : public NestedBase {
          public: 
            virtual std::shared_ptr<NestedBase> clone() const { return std::make_shared<Nested>(*this); } 
            Nested(int pragma_id,  cv::CascadeClassifier & cascade, vector<Rect> & faces, vector<Rect> & faces2)  : NestedBase(pragma_id), cascade_(cascade) , faces_(faces) , faces2_(faces2) {}
         cv::CascadeClassifier & cascade_;
        vector<Rect> & faces_;
        vector<Rect> & faces2_;
        
        void fx(ForParameter for_param,  cv::CascadeClassifier & cascade, vector<Rect> & faces, vector<Rect> & faces2){
//            #pragma omp section
            {
              class Nested : public NestedBase {
              public: 
                virtual std::shared_ptr<NestedBase> clone() const { return std::make_shared<Nested>(*this); } 
                Nested(int pragma_id,  cv::CascadeClassifier & cascade, vector<Rect> & faces)  : NestedBase(pragma_id), cascade_(cascade) , faces_(faces) {}
             cv::CascadeClassifier & cascade_;
            vector<Rect> & faces_;
            
            void fx(ForParameter for_param,  cv::CascadeClassifier & cascade, vector<Rect> & faces){
                cascade.detectMultiScale( gray, faces,
                    1.1, 3, 0
                    //|CASCADE_FIND_BIGGEST_OBJECT
                    //|CASCADE_DO_ROUGH_SEARCH
                    |CASCADE_SCALE_IMAGE,
                    Size(30, 30) );
            launch_todo_job(); 
}
void callme(ForParameter for_param) {
  fx(for_param, cascade_, faces_);
}
};
std::shared_ptr<NestedBase> nested_b = std::make_shared<Nested>(154, cascade, faces);
if(ThreadPool::getInstance("source_exctractor/test_cases/ComputerVision2/ufacedetect.cpp")->call(nested_b)) 
  todo_job_.push(nested_b); 
}
//            #pragma omp section
            {
              class Nested : public NestedBase {
              public: 
                virtual std::shared_ptr<NestedBase> clone() const { return std::make_shared<Nested>(*this); } 
                Nested(int pragma_id,  cv::CascadeClassifier & cascade, vector<Rect> & faces2)  : NestedBase(pragma_id), cascade_(cascade) , faces2_(faces2) {}
             cv::CascadeClassifier & cascade_;
            vector<Rect> & faces2_;
            
            void fx(ForParameter for_param,  cv::CascadeClassifier & cascade, vector<Rect> & faces2){    
                flip(gray, gray, 1);
                cascade.detectMultiScale( gray, faces2,
                                         1.1, 2, 0
                                         //|CASCADE_FIND_BIGGEST_OBJECT
                                         //|CASCADE_DO_ROUGH_SEARCH
                                         |CASCADE_SCALE_IMAGE
                                         ,
                                         Size(30, 30) );
            launch_todo_job(); 
}
void callme(ForParameter for_param) {
  fx(for_param, cascade_, faces2_);
}
};
std::shared_ptr<NestedBase> nested_b = std::make_shared<Nested>(163, cascade, faces2);
if(ThreadPool::getInstance("source_exctractor/test_cases/ComputerVision2/ufacedetect.cpp")->call(nested_b)) 
  todo_job_.push(nested_b); 
}
        launch_todo_job(); 
}
void callme(ForParameter for_param) {
  fx(for_param, cascade_, faces_, faces2_);
}
};
std::shared_ptr<NestedBase> nested_b = std::make_shared<Nested>(152, cascade, faces, faces2);
if(ThreadPool::getInstance("source_exctractor/test_cases/ComputerVision2/ufacedetect.cpp")->call(nested_b)) 
  todo_job_.push(nested_b); 
}
    launch_todo_job(); 
}
void callme(ForParameter for_param) {
  fx(for_param, cascade_, faces_, faces2_);
}
};
std::shared_ptr<NestedBase> nested_b = std::make_shared<Nested>(150, cascade, faces, faces2);
if(ThreadPool::getInstance("source_exctractor/test_cases/ComputerVision2/ufacedetect.cpp")->call(nested_b)) 
  nested_b->callme(ForParameter(0,1));
}
    
    for( vector<Rect>::const_iterator r = faces2.begin(); r !=faces2.end(); r++ )    
        faces.push_back(Rect(smallImg.cols - r->x - r->width, r->y,r->width, r->height));

    t = (double)getTickCount() - t;
    smallImg.copyTo(canvas);

    for( vector<Rect>::const_iterator r = faces.begin(); r !=faces.end(); r++, i++ ) {
        Point center;
        Scalar color = colors[i%8];
        int radius;

        double aspect_ratio = (double)r->width/r->height;
        if( 0.75 < aspect_ratio && aspect_ratio < 1.3 )
        {
            center.x = cvRound((r->x + r->width*0.5)*scale);
            center.y = cvRound((r->y + r->height*0.5)*scale);
            radius = cvRound((r->width + r->height)*0.25*scale);
            circle( canvas, center, radius, color, 3, 8, 0 );
        }
        else
            rectangle( canvas, Point(cvRound(r->x*scale), cvRound(r->y*scale)),
                       Point(cvRound((r->x + r->width-1)*scale),cvRound((r->y + r->height-1)*scale)),
                       color, 3, 8, 0);

    }
}

void detectAndDraw_sx( UMat& img, Mat& canvas, CascadeClassifier& cascade, double scale0)
{
    int i = 0;
    double t = 0, scale=1;
    vector<Rect> faces, faces2;
    const static Scalar colors[] =
    {
        Scalar(0,0,255),
        Scalar(0,128,255),
        Scalar(0,255,255),
        Scalar(0,255,0),
        Scalar(255,128,0),
        Scalar(255,255,0),
        Scalar(255,0,0),
        Scalar(255,0,255)
    };
    static UMat gray, smallImg;

    t = (double)getTickCount();

    resize( img, smallImg, Size(), scale0, scale0, INTER_LINEAR );
    cvtColor( smallImg, gray, COLOR_BGR2GRAY );
    equalizeHist( gray, gray );
//    #pragma omp parallel
    {
      class Nested : public NestedBase {
      public: 
        virtual std::shared_ptr<NestedBase> clone() const { return std::make_shared<Nested>(*this); } 
        Nested(int pragma_id,  cv::CascadeClassifier & cascade, vector<Rect> & faces, vector<Rect> & faces2)  : NestedBase(pragma_id), cascade_(cascade) , faces_(faces) , faces2_(faces2) {}
     cv::CascadeClassifier & cascade_;
    vector<Rect> & faces_;
    vector<Rect> & faces2_;
    
    void fx(ForParameter for_param,  cv::CascadeClassifier & cascade, vector<Rect> & faces, vector<Rect> & faces2){
//        #pragma omp sections
        {
          class Nested : public NestedBase {
          public: 
            virtual std::shared_ptr<NestedBase> clone() const { return std::make_shared<Nested>(*this); } 
            Nested(int pragma_id,  cv::CascadeClassifier & cascade, vector<Rect> & faces, vector<Rect> & faces2)  : NestedBase(pragma_id), cascade_(cascade) , faces_(faces) , faces2_(faces2) {}
         cv::CascadeClassifier & cascade_;
        vector<Rect> & faces_;
        vector<Rect> & faces2_;
        
        void fx(ForParameter for_param,  cv::CascadeClassifier & cascade, vector<Rect> & faces, vector<Rect> & faces2){
//            #pragma omp section
            {
              class Nested : public NestedBase {
              public: 
                virtual std::shared_ptr<NestedBase> clone() const { return std::make_shared<Nested>(*this); } 
                Nested(int pragma_id,  cv::CascadeClassifier & cascade, vector<Rect> & faces)  : NestedBase(pragma_id), cascade_(cascade) , faces_(faces) {}
             cv::CascadeClassifier & cascade_;
            vector<Rect> & faces_;
            
            void fx(ForParameter for_param,  cv::CascadeClassifier & cascade, vector<Rect> & faces){
                cascade.detectMultiScale( gray, faces,
                    1.1, 3, 0
                    //|CASCADE_FIND_BIGGEST_OBJECT
                    //|CASCADE_DO_ROUGH_SEARCH
                    |CASCADE_SCALE_IMAGE,
                    Size(30, 30) );
            launch_todo_job(); 
}
void callme(ForParameter for_param) {
  fx(for_param, cascade_, faces_);
}
};
std::shared_ptr<NestedBase> nested_b = std::make_shared<Nested>(231, cascade, faces);
if(ThreadPool::getInstance("source_exctractor/test_cases/ComputerVision2/ufacedetect.cpp")->call(nested_b)) 
  todo_job_.push(nested_b); 
}
//            #pragma omp section
            {
              class Nested : public NestedBase {
              public: 
                virtual std::shared_ptr<NestedBase> clone() const { return std::make_shared<Nested>(*this); } 
                Nested(int pragma_id,  cv::CascadeClassifier & cascade, vector<Rect> & faces2)  : NestedBase(pragma_id), cascade_(cascade) , faces2_(faces2) {}
             cv::CascadeClassifier & cascade_;
            vector<Rect> & faces2_;
            
            void fx(ForParameter for_param,  cv::CascadeClassifier & cascade, vector<Rect> & faces2){    
                flip(gray, gray, 1);
                cascade.detectMultiScale( gray, faces2,
                                         1.1, 2, 0
                                         //|CASCADE_FIND_BIGGEST_OBJECT
                                         //|CASCADE_DO_ROUGH_SEARCH
                                         |CASCADE_SCALE_IMAGE
                                         ,
                                         Size(30, 30) );
            launch_todo_job(); 
}
void callme(ForParameter for_param) {
  fx(for_param, cascade_, faces2_);
}
};
std::shared_ptr<NestedBase> nested_b = std::make_shared<Nested>(240, cascade, faces2);
if(ThreadPool::getInstance("source_exctractor/test_cases/ComputerVision2/ufacedetect.cpp")->call(nested_b)) 
  todo_job_.push(nested_b); 
}
        launch_todo_job(); 
}
void callme(ForParameter for_param) {
  fx(for_param, cascade_, faces_, faces2_);
}
};
std::shared_ptr<NestedBase> nested_b = std::make_shared<Nested>(229, cascade, faces, faces2);
if(ThreadPool::getInstance("source_exctractor/test_cases/ComputerVision2/ufacedetect.cpp")->call(nested_b)) 
  todo_job_.push(nested_b); 
}
    launch_todo_job(); 
}
void callme(ForParameter for_param) {
  fx(for_param, cascade_, faces_, faces2_);
}
};
std::shared_ptr<NestedBase> nested_b = std::make_shared<Nested>(227, cascade, faces, faces2);
if(ThreadPool::getInstance("source_exctractor/test_cases/ComputerVision2/ufacedetect.cpp")->call(nested_b)) 
  nested_b->callme(ForParameter(0,1));
}
    
    for( vector<Rect>::const_iterator r = faces2.begin(); r !=faces2.end(); r++ )    
        faces.push_back(Rect(smallImg.cols - r->x - r->width, r->y,r->width, r->height));

    t = (double)getTickCount() - t;
    smallImg.copyTo(canvas);

    for( vector<Rect>::const_iterator r = faces.begin(); r !=faces.end(); r++, i++ ) {
        Point center;
        Scalar color = colors[i%8];
        int radius;

        double aspect_ratio = (double)r->width/r->height;
        if( 0.75 < aspect_ratio && aspect_ratio < 1.3 )
        {
            center.x = cvRound((r->x + r->width*0.5)*scale);
            center.y = cvRound((r->y + r->height*0.5)*scale);
            radius = cvRound((r->width + r->height)*0.25*scale);
            circle( canvas, center, radius, color, 3, 8, 0 );
        }
        else
            rectangle( canvas, Point(cvRound(r->x*scale), cvRound(r->y*scale)),
                       Point(cvRound((r->x + r->width-1)*scale),cvRound((r->y + r->height-1)*scale)),
                       color, 3, 8, 0);

    }
}