
#include <unistd.h>

#include "/Users/admin/Documents/soma/graph_sched/ProfileTracker.h"
class X{
public:
  float* y(int i, int *c);
};

float* X::y (int i, int *c) {
if( ProfileTracker x = ProfileTrackParams(9, 0)) {
	int a = i;
	int b;
  double n;
  float *d, e;
//	#pragma omp parallel shared(a, n, d, e) period(191)
  if( ProfileTracker x = ProfileTrackParams(9, 15))
  {
    sleep(1);
  }
	
	//if(a == 5) {
//		#pragma omp parallel for shared(b, a, d) period(1)
 		if( ProfileTracker x = ProfileTrackParams(9, 21, 125 - 124))
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
  int c;
//  #pragma omp parallel for shared(a, n, b, c) period(19)
  if( ProfileTracker x = ProfileTrackParams(28, 36, 1 - 0))
  for(int i=0;i< 1; i += 1) {
   sleep(1);
  }

//  #pragma omp parallel shared(a)
  if( ProfileTracker x = ProfileTrackParams(28, 41))
  {
    
//    #pragma omp for
    if( ProfileTracker x = ProfileTrackParams(28, 44, 2 - 1))
    for(int i=1;i< 2; i ++) {
      sleep(1);
    }
//    #pragma omp single
    if( ProfileTracker x = ProfileTrackParams(28, 48))
    {
//      #pragma omp task shared(b)
      if( ProfileTracker x = ProfileTrackParams(28, 50))
      {
        b = 5;
        sleep(1);
      }

//      #pragma omp task shared(b) period(19)
      if( ProfileTracker x = ProfileTrackParams(28, 56))
      {
      	/* code */
        sleep(1);
      }

//      #pragma omp task shared(b)
      if( ProfileTracker x = ProfileTrackParams(28, 62))
      {
//        #pragma omp task shared(b)
        if( ProfileTracker x = ProfileTrackParams(28, 64))
        {
          sleep(1);
        }
          sleep(1);
//        #pragma omp task shared(b)
        if( ProfileTracker x = ProfileTrackParams(28, 69))
        {
          X w;
          int y;
          int *z;
          sleep(1);
          w.y(y, z);
        }

      }
//      #pragma omp task shared(b)
      if( ProfileTracker x = ProfileTrackParams(28, 79))
      {
        sleep(1);
      }
    }


  }



//  #pragma omp parallel shared(b)
  if( ProfileTracker x = ProfileTrackParams(28, 90))
  {
//    #pragma omp for period(12)
    if( ProfileTracker x = ProfileTrackParams(28, 92, n - 1))
    for(int i=1;i< n; i += 3) {
      sleep(1);
    }
  }
}
}
