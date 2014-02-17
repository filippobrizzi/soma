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
#include <stdlib.h>
#include <unistd.h>
#include "thread_pool/threads_pool.h"

using namespace std;
using namespace cv;

struct timespec start_time_;
string cascadeName;
int write_on_disk;
int print_time;
int farm_size; 
string images_destinations;



void detectAndDraw( UMat& img, Mat& canvas, CascadeClassifier& cascade, double scale);
void dx(UMat* img, Mat* canvas, CascadeClassifier* cascade, double scale0, int i);
void sx(UMat* img, Mat* canvas, CascadeClassifier* cascade, double scale0, int i);

int usage (char *exec) {
    printf("\nUsage : %s\n",exec);
    printf("\t-s (left video path), -d (right video path)\n"); 
    printf("\t[-i] (destination path of the annotated immages, if left empty write on disk will be disabled)\n");
    printf("\t-f (farm size)\n");
    printf("\t-c (cascade file path\n");
    printf("\t -t (print images time)\n");
    return(0);
};

int parse(string *video_name_sx, 
          string *video_name_dx,
          char* argv[], int argc){
    // set parameters wrt argv and argc
    if(argc == 1){
        usage(argv[0]);
        return(1);
    }

    write_on_disk = 0;
    print_time = 0;
    char ch;
    extern char* optarg;
    while ( (ch = getopt(argc, argv, "s:d:i:f:c:?:h:t:"))!=-1 ) {
        switch(ch) {
            case 's': *video_name_sx = optarg; break;
            case 'd': *video_name_dx = optarg; break;
            case 'i': images_destinations = optarg; write_on_disk = 1; break;
            case 'f': farm_size = atoi(optarg); break;
            case 'c': cascadeName = optarg; break;
            case 't': print_time = 1; break;
            default: usage(argv[0]); return(1);
        }
    }
    return(0);
};

//clock_t start_time_;
int main( int argc, char** argv ){
    //start_time_ = clock();
    //gettimeofday(&start_time_,NULL);
    clock_gettime(CLOCK_MONOTONIC, &start_time_);

    string video_name_sx; 
    string video_name_dx;

    if(parse(&video_name_sx, &video_name_dx, argv, argc) == 1)
        return 1;

    VideoCapture capture_dx;
    VideoCapture capture_sx;
    capture_dx.open(video_name_dx);
    capture_sx.open(video_name_sx);

    if(!capture_dx.isOpened() || !capture_sx.isOpened()) {
        cout << "VIDEO NOT OPENED" << endl;
        return -1;

    }
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

                UMat *frame_sx = new UMat[farm_size]; 
                UMat image_sx;
                Mat *canvas_sx = new Mat[farm_size];
                double scale_sx = 1;

                CascadeClassifier *cascade_sx = new CascadeClassifier[farm_size];

                for(int i = 0; i < farm_size; i ++)
                    cascade_sx[i].load(cascadeName);

                cout << "Video sx capturing has been started ..." << endl;
                bool frame_success;
                int count = 0;
                while(count < 220){
                    
                    for(int j = 0; j < farm_size; j ++)
                        frame_success = capture_sx.read(frame_sx[j]); // read a new frame from video

                    if (!frame_success) break;

                    sx(frame_sx, canvas_sx, cascade_sx, scale_sx, count);
                    count ++;
                }
            launch_todo_job(); 
}
void callme(ForParameter for_param) {
  fx(for_param, capture_sx_);
}
};
std::shared_ptr<NestedBase> nested_b = std::make_shared<Nested>(91, capture_sx);
if(ThreadPool::getInstance("source_exctractor/test_cases/ComputerVision/ufacedetect.cpp")->call(nested_b)) 
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
                UMat *frame_dx = new UMat[farm_size];
                UMat image_dx;
                Mat *canvas_dx = new Mat[farm_size];
                double scale_dx = 1;

                CascadeClassifier *cascade_dx = new CascadeClassifier[farm_size];

                for(int i = 0; i < farm_size; i ++)
                    cascade_dx[i].load(cascadeName);

                cout << "Video dx capturing has been started ..." << endl;
                bool frame_success;
                int count = 0;
                while(count < 220) {    

                    for(int j = 0; j < farm_size; j ++)
                        frame_success = capture_dx.read(frame_dx[j]);
                    
                    if (!frame_success) break;

                    dx(frame_dx, canvas_dx, cascade_dx, scale_dx, count);
                    count ++;
                }
            launch_todo_job(); 
}
void callme(ForParameter for_param) {
  fx(for_param, capture_dx_);
}
};
std::shared_ptr<NestedBase> nested_b = std::make_shared<Nested>(118, capture_dx);
if(ThreadPool::getInstance("source_exctractor/test_cases/ComputerVision/ufacedetect.cpp")->call(nested_b)) 
  todo_job_.push(nested_b); 
}
        launch_todo_job(); 
}
void callme(ForParameter for_param) {
  fx(for_param, capture_sx_, capture_dx_);
}
};
std::shared_ptr<NestedBase> nested_b = std::make_shared<Nested>(89, capture_sx, capture_dx);
if(ThreadPool::getInstance("source_exctractor/test_cases/ComputerVision/ufacedetect.cpp")->call(nested_b)) 
  todo_job_.push(nested_b); 
}
    launch_todo_job(); 
}
void callme(ForParameter for_param) {
  fx(for_param, capture_sx_, capture_dx_);
}
};
std::shared_ptr<NestedBase> nested_b = std::make_shared<Nested>(87, capture_sx, capture_dx);
if(ThreadPool::getInstance("source_exctractor/test_cases/ComputerVision/ufacedetect.cpp")->call(nested_b)) 
  nested_b->callme(ForParameter(0,1));
}

    return 0;
}


void dx(UMat* frame_dx, Mat* canvas_dx, CascadeClassifier* cascade_dx, double scale_dx, int i) {
//    #pragma omp parallel for
    //for(int j = 0; j < farm_size; j ++){
{
  class Nested : public NestedBase {
  public: 
    virtual std::shared_ptr<NestedBase> clone() const { return std::make_shared<Nested>(*this); } 
    Nested(int pragma_id,  cv::UMat * frame_dx,  cv::Mat * canvas_dx,  cv::CascadeClassifier * cascade_dx, double scale_dx, int i)  : NestedBase(pragma_id), frame_dx_(frame_dx) , canvas_dx_(canvas_dx) , cascade_dx_(cascade_dx) , scale_dx_(scale_dx) , i_(i) {}
 cv::UMat * frame_dx_;
 cv::Mat * canvas_dx_;
 cv::CascadeClassifier * cascade_dx_;
double scale_dx_;
int i_;

void fx(ForParameter for_param,  cv::UMat * frame_dx,  cv::Mat * canvas_dx,  cv::CascadeClassifier * cascade_dx, double & scale_dx, int & i) {
for(int j = 0 + for_param.thread_id_*(farm_size - 0)/for_param.num_threads_; j < 0 + (for_param.thread_id_ + 1)*(farm_size - 0)/for_param.num_threads_; j ++ ) { 
        detectAndDraw( frame_dx[j], canvas_dx[j], cascade_dx[j], scale_dx);
        if(write_on_disk) {
            stringstream filename_dx;
            filename_dx << images_destinations << "/img_" << i << "_" << j << "_dx.jpg";
            imwrite(filename_dx.str(), canvas_dx[j]);
        }
        if(print_time) {
            //clock_t end_time_ = clock();
            //float elapsed_time_ = ((double)(end_time_ - start_time_))/CLOCKS_PER_SEC;
            struct timespec end_time_;
            clock_gettime(CLOCK_MONOTONIC, &end_time_);
            double elapsed_time_ = (end_time_.tv_sec - start_time_.tv_sec);
            elapsed_time_ += (end_time_.tv_nsec - start_time_.tv_nsec) / 1000000000.0;
            std::cout << std::this_thread::get_id() << " img_" << i << "_" << j << "_dx.jpg " << elapsed_time_ << std::endl;
        }
    }
launch_todo_job();
 }
void callme(ForParameter for_param) {
  fx(for_param, frame_dx_, canvas_dx_, cascade_dx_, scale_dx_, i_);
}
};
std::shared_ptr<NestedBase> nested_b = std::make_shared<Nested>(152, frame_dx, canvas_dx, cascade_dx, scale_dx, i);
if(ThreadPool::getInstance("source_exctractor/test_cases/ComputerVision/ufacedetect.cpp")->call(nested_b)) 
  nested_b->callme(ForParameter(0,1));
}
}

void sx(UMat* frame_sx, Mat* canvas_sx, CascadeClassifier* cascade_sx, double scale_sx, int i) {
//    #pragma omp parallel for
    //for(int j = 0; j < farm_size; j ++){
{
  class Nested : public NestedBase {
  public: 
    virtual std::shared_ptr<NestedBase> clone() const { return std::make_shared<Nested>(*this); } 
    Nested(int pragma_id,  cv::UMat * frame_sx,  cv::Mat * canvas_sx,  cv::CascadeClassifier * cascade_sx, double scale_sx, int i)  : NestedBase(pragma_id), frame_sx_(frame_sx) , canvas_sx_(canvas_sx) , cascade_sx_(cascade_sx) , scale_sx_(scale_sx) , i_(i) {}
 cv::UMat * frame_sx_;
 cv::Mat * canvas_sx_;
 cv::CascadeClassifier * cascade_sx_;
double scale_sx_;
int i_;

void fx(ForParameter for_param,  cv::UMat * frame_sx,  cv::Mat * canvas_sx,  cv::CascadeClassifier * cascade_sx, double & scale_sx, int & i) {
for(int j = 0 + for_param.thread_id_*(farm_size - 0)/for_param.num_threads_; j < 0 + (for_param.thread_id_ + 1)*(farm_size - 0)/for_param.num_threads_; j ++ ) { 
        detectAndDraw( frame_sx[j], canvas_sx[j], cascade_sx[j], scale_sx);
        if(write_on_disk) {
            stringstream filename_sx;
            filename_sx << images_destinations << "/img_" << i << "_" << j << "_sx.jpg";
            imwrite(filename_sx.str(), canvas_sx[j]);
        }
        if(print_time) {
            struct timespec end_time_;
            clock_gettime(CLOCK_MONOTONIC, &end_time_);
            double elapsed_time_ = (end_time_.tv_sec - start_time_.tv_sec);
            elapsed_time_ += (end_time_.tv_nsec - start_time_.tv_nsec) / 1000000000.0;
            std::cout << std::this_thread::get_id() << " img_" << i << "_" << j << "_sx.jpg " << elapsed_time_ << std::endl;
        }
    }
launch_todo_job();
 }
void callme(ForParameter for_param) {
  fx(for_param, frame_sx_, canvas_sx_, cascade_sx_, scale_sx_, i_);
}
};
std::shared_ptr<NestedBase> nested_b = std::make_shared<Nested>(169, frame_sx, canvas_sx, cascade_sx, scale_sx, i);
if(ThreadPool::getInstance("source_exctractor/test_cases/ComputerVision/ufacedetect.cpp")->call(nested_b)) 
  nested_b->callme(ForParameter(0,1));
}
}

void detectAndDraw( UMat& img, Mat& canvas, CascadeClassifier& cascade, double scale0)
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

                cascade.detectMultiScale( gray, faces,
                    1.1, 3, 0
                    //|CASCADE_FIND_BIGGEST_OBJECT
                    //|CASCADE_DO_ROUGH_SEARCH
                    |CASCADE_SCALE_IMAGE,
                    Size(30, 30) );

                flip(gray, gray, 1);
                cascade.detectMultiScale( gray, faces2,
                                         1.1, 2, 0
                                         //|CASCADE_FIND_BIGGEST_OBJECT
                                         //|CASCADE_DO_ROUGH_SEARCH
                                         |CASCADE_SCALE_IMAGE
                                         ,
                                         Size(30, 30) );

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