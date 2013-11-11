#include <omp.h>
#include <stdio.h>

int y();
int z();

#include "/home/pippo/Documents/Library/clomp-master/include/myprogram/profiling/ProfileTracker.h"
int main() {
if( ProfileTracker x = ProfileTrackParams(7, 0)) {
	int c = 0;
	float d = 1.2;
		int x;

//	#pragma omp parallel for period(10)
		if( ProfileTracker x = ProfileTrackParams(7, 13, 10))
		for(int i = 0; i < 10; ++i)
		{
			d = 4;
			x = 1;
		}

//	#pragma omp parallel shared(d) period(1001)
	if( ProfileTracker x = ProfileTrackParams(7, 20))
	{
		int a = 0;
		z();
//		#pragma omp task
		if( ProfileTracker x = ProfileTrackParams(7, 24))
		{
			//a = 3;
			//x = 1;
			d = 1;
		}

//		#pragma omp for period(10)
		if( ProfileTracker x = ProfileTrackParams(7, 31, 10))
		for(int i = 0; i < 10; ++i)
		{
			d = 4;
			x = 1;
		}
		y();

	}
	int b = 0;
//	#pragma omp parallel for shared(b)
	if( ProfileTracker x = ProfileTrackParams(7, 41, 3))
	for(int i = 0; i < 3; i ++)
		b++;

	return 1;
}
}

int y() {
if( ProfileTracker x = ProfileTrackParams(47, 0)) {
	int a;
//	#pragma omp parallel
	if( ProfileTracker x = ProfileTrackParams(47, 50))
	{
		a = 0;

	}
	return a;
}
}

int z() {
if( ProfileTracker x = ProfileTrackParams(57, 0)) {
	int b;
//	#pragma omp parallel
	if( ProfileTracker x = ProfileTrackParams(57, 60))
	{
		b = 1;
	}
	return b;
}
}