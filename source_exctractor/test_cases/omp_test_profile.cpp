
#include <unistd.h>

class X{
public:
  float* y(int i, int *c);
};

#include "/home/pippo/Documents/Library/clomp-master/include/myprogram/profiling/ProfileTracker.h"
float* X::y (int i, int *c) {
if( ProfileTracker x = ProfileTrackParams(9, 0)) {
	int a = i;
	int b;
  int n;
  float *d;
//	#pragma omp parallel shared(n)
  if( ProfileTracker x = ProfileTrackParams(9, 15))
  {
    sleep(1);
  }
	
	//if(a == 5) {
//		#pragma omp parallel for shared(b, a) 
 		if( ProfileTracker x = ProfileTrackParams(9, 21, 125))
 		for(int j = 124; j < 125; j ++)
  			 sleep(1);
	//}
	return d;
}
}


int main() {
if( ProfileTracker x = ProfileTrackParams(28, 0)) {

 int a,n, b;
 b=0;
 a = 10;
 n = 2;
// #pragma omp parallel for shared(a)
 if( ProfileTracker x = ProfileTrackParams(28, 35, 1))
 for(int i=0;i< 1; i += 1) {
   sleep(1);
 }

// #pragma omp parallel shared(a)
 if( ProfileTracker x = ProfileTrackParams(28, 40))
 {
//  #pragma omp task shared(b)
  if( ProfileTracker x = ProfileTrackParams(28, 42))
  {
    b = 5;
    sleep(1);
  }

//  #pragma omp task shared(b)
  if( ProfileTracker x = ProfileTrackParams(28, 48))
  {
  	/* code */
    sleep(1);
  }

//  #pragma omp for
    if( ProfileTracker x = ProfileTrackParams(28, 54, 2))
    for(int i=1;i< 2; i ++) {
      sleep(1);
    }
  
//  #pragma omp task shared(b)
  if( ProfileTracker x = ProfileTrackParams(28, 59))
  {
//    #pragma omp task shared(b)
    if( ProfileTracker x = ProfileTrackParams(28, 61))
    {
      sleep(1);
    }
      sleep(1);
//    #pragma omp task shared(b)
    if( ProfileTracker x = ProfileTrackParams(28, 66))
    {
      X w;
      int y;
      int *z;
      sleep(1);
      w.y(y, z);
    }

  }
//  #pragma omp task shared(b)
  if( ProfileTracker x = ProfileTrackParams(28, 76))
  {
    sleep(1);
  }

 }

// #pragma omp parallel shared(b)
 if( ProfileTracker x = ProfileTrackParams(28, 83))
 {
//    #pragma omp for
    if( ProfileTracker x = ProfileTrackParams(28, 85, n))
    for(int i=1;i< n; i += 1) {
      sleep(1);
    }
 }

}
}
