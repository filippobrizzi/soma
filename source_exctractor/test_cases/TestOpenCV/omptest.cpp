#include <iostream>       // std::cerr
#include <omp.h>
#include <unistd.h>

int apply_filter_1(int frame){
  #pragma omp parallel for
  for (int i = 0; i < 10; ++i)
  {
    /* code */
  }  
};

int apply_filter_2(int frame){
  #pragma omp parallel for
  for (int i = 0; i < 10; ++i)
  {
    /* code */
  }  
};

int main () {

  
  #pragma omp parallel
  {
    
    #pragma omp sections
    {
      #pragma omp section
      {
        //prendi frame 1
        apply_filter_1(1);
        apply_filter_2(1);
      }
      #pragma omp section
      {
	       //frame 2
        apply_filter_1(2);
        apply_filter_2(2);
      }

    }
  }
}
