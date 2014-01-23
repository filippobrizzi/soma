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

void detectAndDraw( UMat& img, Mat& canvas, CascadeClassifier& cascade, double scale);
void dx(UMat* img, Mat* canvas, CascadeClassifier* cascade, double scale0, int i);
void sx(UMat* img, Mat* canvas, CascadeClassifier* cascade, double scale0, int i);


string cascadeName = "/home/pippo/Documents/Project/soma/source_exctractor/test_cases/ComputerVision/haarcascade_frontalface_alt.xml";

int main( int argc, const char** argv ){
   
    String inputName;

    VideoCapture capture_dx;
    VideoCapture capture_sx;
    capture_dx.open("/home/pippo/Documents/Project/soma/source_exctractor/test_cases/ComputerVision/MyVideo_dx.avi");
    capture_sx.open("/home/pippo/Documents/Project/soma/source_exctractor/test_cases/ComputerVision/MyVideo_sx.avi");

    if(!capture_dx.isOpened() || !capture_sx.isOpened()) {
        cout << "VIDEO NOT OPENED" << endl;
        return -1;

    }
    //omp_set_num_threads(2);
    #pragma omp parallel
    {
        #pragma omp sections
        {
            #pragma omp section
            {

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
            }
            #pragma omp section
            {
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
            }
        }
    }

    return 0;
}


void dx(UMat* frame_dx, Mat* canvas_dx, CascadeClassifier* cascade_dx, double scale_dx, int i) {
    //omp_set_num_threads(2);
    //omp_set_nested(1);
    #pragma omp parallel for
    for(int j = 0; j < 4; j ++){
        detectAndDraw( frame_dx[j], canvas_dx[j], cascade_dx[j], scale_dx);
        stringstream filename_dx;
        filename_dx << "images/img_" << i << "_" << j << "_dx.jpg";
        std::cout << "--------------- " << filename_dx.str() << std::endl;
        //imwrite(filename_dx.str(), canvas_dx[j]);
    }

}

void sx(UMat* frame_sx, Mat* canvas_sx, CascadeClassifier* cascade_sx, double scale_sx, int i) {
    //omp_set_num_threads(2);
    //omp_set_nested(1);
    #pragma omp parallel for
    for(int j = 0; j < 4; j ++){
        detectAndDraw( frame_sx[j], canvas_sx[j], cascade_sx[j], scale_sx);
        stringstream filename_sx;
        filename_sx << "images/img_" << i << "_" << j << "_sx.jpg";
        std::cout << "--------------- " << filename_sx.str() << std::endl;
        //imwrite(filename_sx.str(), canvas_sx[j]);
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