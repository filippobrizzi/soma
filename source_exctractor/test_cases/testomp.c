#include <omp.h>
#include <stdio.h>

int y();
int z();

int main() {
	int c = 0;
	float d = 1.2;
		int x;

	#pragma omp parallel for period(10)
		for(int i = 0; i < 10; ++i)
		{
			d = 4;
			x = 1;
		}

	#pragma omp parallel shared(d) period(1001)
	{
		int a = 0;
		z();
		#pragma omp task
		{
			//a = 3;
			//x = 1;
			d = 1;
		}

		#pragma omp for period(10)
		for(int i = 0; i < 10; ++i)
		{
			d = 4;
			x = 1;
		}
		y();

	}
	int b = 0;
	#pragma omp parallel for shared(b)
	for(int i = 0; i < 3; i ++)
		b++;

	return 1;
}

int y() {
	int a;
	#pragma omp parallel
	{
		a = 0;

	}
	return a;
}

int z() {
	int b;
	#pragma omp parallel
	{
		b = 1;
	}
	return b;
}