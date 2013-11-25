//#include <string>


class A {

public:
	int aa;
};


void y(int x){
	x ++;
}


int main() {

	int a, b, c;
	float d;
	double e;
	c = 0;
	float *ptr;
	A bbb;
	//std::string ss;
	#pragma omp parallel shared(a, b, c) private(d, e)
	{
		int yy;
		double *kk;
		e = 

		#pragma omp task
		 
		{	
			int x;
			float *pp;
			kk = &e;
			ptr = &d;
			a = 1;
			//b = 2;
			y(c);
			d = e;
			for(int i = 0; i < b; i ++)
				a ++;

			A aaa;
			aaa.aa = a;
			bbb.aa = a;
		}

		#pragma omp for
		for (int i = 10; i < 100; ++i)
		{	
			c++;
		}

	}

	return 1;


}