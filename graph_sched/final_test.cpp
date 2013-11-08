class X{
public:
  float* y(int i, int *c);
};

float* X::y (int i, int *c) {
	int a = i;
	int b;
  int n;
  float *d;
	#pragma omp parallel firstprivate(n) 
	{
		a = 3;
	}
	if(a == 5) {
		#pragma omp parallel for firstprivate(n) shared(b, a) 
 		for(int j = 124; j < 1000; j ++)
  			 a += i;
	}
	return d;
}


int main() {

 int a,n, b;
 b=0;
 #pragma omp parallel for
 for(int i=b;i< a; i += 1) {
   a += i;
 }

 #pragma omp parallel
 {
  #pragma omp task
  {
    b = 5;
  }

  #pragma omp task
  {
  	/* code */
  }

  #pragma omp task
  {
    #pragma omp task
    {

    }

    #pragma omp task
    {
      X w;
      int y;
      int *z;
      w.y(y, z);
    }

  }
  #pragma omp task
  {

  }

 }

 #pragma omp parallel
 {
    #pragma omp for
    for(int i=b;i< a; i += 1) {
      a += i;
    }
 }

}