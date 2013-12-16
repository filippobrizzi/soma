#include <omp.h>
#include <stdio.h>

int y();
int z();

int main() {
	int a, b, c;

	#pragma omp parallel 
	{
		
		#pragma omp task
		{
			a = 5;

			#pragma omp task
			{
				b = 5;
			}		

			#pragma omp task
			{
				c = 6;
			}
			
		}
		
	}
}


