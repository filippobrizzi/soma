#include <unistd.h>

class X{
public:
  float* y(int i, int *c);
};

#include "/Users/admin/Documents/soma/graph_sched/ProfileTracker.h"
float* X::y (int i, int *c) {

if( ProfileTracker x = ProfileTrackParams(8, 0)) {
	int a = i;
	int b;
  int n;
  float *d;
//	#pragma omp parallel firstprivate(n) deadline(0.1)
	if( ProfileTracker x = ProfileTrackParams(8, 14))
	{
    sleep(1);
	}
	if(a == 5) {
//		#pragma omp parallel for firstprivate(n) shared(b, a) deadline(0.53)
 		if( ProfileTracker x = ProfileTrackParams(8, 19, 125))
 		for(int j = 124; j < 125; j ++)
  			 sleep(1);
	}
	return d;
}
}


int main() {

if( ProfileTracker x = ProfileTrackParams(26, 0)) {

 int a,n, b;
 b=0;
 a = 10;
 n = 100;
// #pragma omp parallel for deadline(1)
 if( ProfileTracker x = ProfileTrackParams(26, 33, 1))
 for(int i=0;i< 1; i += 1) {
   sleep(1);
 }

// #pragma omp parallel deadline(0.1)
 if( ProfileTracker x = ProfileTrackParams(26, 38))
 {
  
//  #pragma omp task deadline(0.111)
  if( ProfileTracker x = ProfileTrackParams(26, 41))
  {
    b = 5;
    sleep(1);
  }

//  #pragma omp task deadline(0.15)
  if( ProfileTracker x = ProfileTrackParams(26, 47))
  {
  	/* code */
    sleep(1);
  }

//  #pragma omp task deadline(0.15)
  if( ProfileTracker x = ProfileTrackParams(26, 53))
  {
//    #pragma omp task deadline(0.25)
    if( ProfileTracker x = ProfileTrackParams(26, 55))
    {
      sleep(1);
    }
      sleep(1);
//    #pragma omp task deadline(0.25)
    if( ProfileTracker x = ProfileTrackParams(26, 60))
    {
      X w;
      int y;
      int *z;
      sleep(1);
      w.y(y, z);
    }

  }
//  #pragma omp task deadline(0.25)
  if( ProfileTracker x = ProfileTrackParams(26, 70))
  {
    sleep(1);
  }

 }

// #pragma omp parallel deadline(0.35)
 if( ProfileTracker x = ProfileTrackParams(26, 77))
 {
//    #pragma omp for deadline(0.25)
    if( ProfileTracker x = ProfileTrackParams(26, 79, n))
    for(int i=99;i< n; i += 1) {
      sleep(1);
    }
 }

}
}
