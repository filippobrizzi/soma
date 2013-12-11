
#include <unistd.h>

class X{
public:
  float* y(int i, int *c);
};

float* X::y (int i, int *c) {
	int a = i;
	int b;
  double n;
  float *d, e;
	#pragma omp parallel shared(a, n, d, e) period(191)
  {
    sleep(1);
  }
	
	//if(a == 5) {
		#pragma omp parallel for shared(b, a, d) period(1)
 		for(int j = 124; j < 125; j ++)
  			 sleep(1);
	//}
	return d;
}


int main() {

 int a,n, b;
 b=0;
 a = 10;
 n = 2;
 int c;
 #pragma omp parallel for shared(a, n, b, c) period(19)
 for(int i=0;i< 1; i += 1) {
   sleep(1);
 }

 #pragma omp parallel shared(a)
 {
  #pragma omp task shared(b)
  {
    b = 5;
    sleep(1);
  }

  #pragma omp task shared(b) period(19)
  {
  	/* code */
    sleep(1);
  }

  #pragma omp for
    for(int i=1;i< 2; i ++) {
      sleep(1);
    }
  
  #pragma omp task shared(b)
  {
    #pragma omp task shared(b)
    {
      sleep(1);
    }
      sleep(1);
    #pragma omp task shared(b)
    {
      X w;
      int y;
      int *z;
      sleep(1);
      w.y(y, z);
    }

  }
  #pragma omp task shared(b)
  {
    sleep(1);
  }

 }

 #pragma omp parallel shared(b)
 {
    #pragma omp for period(12)
    for(int i=1;i< n; i += 3) {
      sleep(1);
    }
 }

}
