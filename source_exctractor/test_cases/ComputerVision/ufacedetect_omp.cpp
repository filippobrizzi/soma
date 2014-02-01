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

using namespace std;
using namespace cv;

string cascadeName = "./haarcascade_frontalface_alt.xml";
int write_on_disk;
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
    char ch;
    extern char* optarg;
    while ( (ch = getopt(argc, argv, "s:d:i:w:f:?:h:"))!=-1 ) {
        switch(ch) {
            case 's': *video_name_sx = optarg; break;
            case 'd': *video_name_dx = optarg; break;
            case 'i': images_destinations = optarg; write_on_disk = 1; break;
            case 'f': farm_size = atoi(optarg); break;
            default: usage(argv[0]); return(1);
        }
    }
    return(0);
};


int main( int argc, char** argv ){
   
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

    omp_set_num_threads(2);
    #pragma omp parallel
    {
        #pragma omp sections
        {
            #pragma omp section
            {

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
                while(1){
                    
                    for(int j = 0; j < farm_size; j ++)
                        frame_success = capture_sx.read(frame_sx[j]); // read a new frame from video

                    if (!frame_success) break;

                    sx(frame_sx, canvas_sx, cascade_sx, scale_sx, count);
                    count ++;
                }
            }
            #pragma omp section
            {
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
                while(1) {    

                    for(int j = 0; j < farm_size; j ++)
                        frame_success = capture_dx.read(frame_dx[j]);
                    
                    if (!frame_success) break;

                    dx(frame_dx, canvas_dx, cascade_dx, scale_dx, count);
                    count ++;
                }
            }
        }
    }

    return 0;
}


void dx(UMat* frame_dx, Mat* canvas_dx, CascadeClassifier* cascade_dx, double scale_dx, int i) {
    omp_set_nested(1);
    #pragma omp parallel for
    for(int j = 0; j < farm_size; j ++){
        detectAndDraw( frame_dx[j], canvas_dx[j], cascade_dx[j], scale_dx);
        if(write_on_disk) {
            stringstream filename_dx;
            filename_dx << images_destinations << "/img_" << i << "_" << j << "_dx.jpg";
            imwrite(filename_dx.str(), canvas_dx[j]);
        }
    }
}

void sx(UMat* frame_sx, Mat* canvas_sx, CascadeClassifier* cascade_sx, double scale_sx, int i) {
    omp_set_nested(1);
    #pragma omp parallel for
    for(int j = 0; j < farm_size; j ++){
        detectAndDraw( frame_sx[j], canvas_sx[j], cascade_sx[j], scale_sx);
        if(write_on_disk) {
            stringstream filename_sx;
            filename_sx << images_destinations << "/img_" << i << "_" << j << "_sx.jpg";
            imwrite(filename_sx.str(), canvas_sx[j]);
        }
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