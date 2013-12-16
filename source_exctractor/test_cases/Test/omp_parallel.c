#include <stdio.h>
//#include <iostream>
#include <unistd.h>

//int y();

int main() {

	int a;
	#pragma omp parallel private(a)
	{	
 		
 		sleep(1);
 		
 	}

 	#pragma omp parallel
 	{ 
 	
 		sleep(1);
 		
 	}
 	
 //	int a = y();

 	//#pragma omp master
 	//printf("hello world\n");

	#pragma omp parallel for
	for(int i=0;i<2;i++) {
		sleep(1);
	}
	return 0;
}

/*int y() {

	int b = 0;

	#pragma omp parallel
	{
		sleep(1);
	}


	return b;

}*/