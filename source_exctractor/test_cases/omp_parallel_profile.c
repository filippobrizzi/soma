#include <stdio.h>
//#include <iostream>
#include <unistd.h>

//int y();

#include "/home/pippo/Documents/Library/clomp-master/include/myprogram/profiling/ProfileTracker.h"
int main() {
if( ProfileTracker x = ProfileTrackParams(7, 0)) {


//	#pragma omp parallel
	if( ProfileTracker x = ProfileTrackParams(7, 11))
	{	
 		
 		sleep(1);
 		
 	}

// 	#pragma omp parallel
 	if( ProfileTracker x = ProfileTrackParams(7, 18))
 	{ 
 	
 		sleep(1);
 		
 	}
 	
 //	int a = y();

 	#pragma omp master
// 	printf("hello world\n");
//
//	#pragma if( ProfileTracker x = ProfileTrackParams(7, 29))
	omp parallel for
	if( ProfileTracker x = ProfileTrackParams(7, 30, 2 - 0))
	for(int i=0;i<2;i++) {
		sleep(1);
	}
	return 0;
}
}

/*int y() {

	int b = 0;

	#pragma omp parallel
	{
		sleep(1);
	}


	return b;

}*/