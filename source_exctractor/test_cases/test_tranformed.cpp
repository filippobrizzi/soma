//#include <string>

class A {

public:
	int aa;
};


#include "instancerun.h"
void y(int x){
	x ++;
}


int main() {

	int a, b, c;
	float d;
	double e;
	c = 0;
	float *ptr;
	A *bbb;
	//std::string ss;
//	#pragma omp parallel shared(a, b, c) private(d, bbb)
	{
	  class Nested : public NestedBase {
	    Nested(int pragmaID, double & e, float * ptr, float d, int & a, int & b, int & c, A * bbb)  : pragmaID(pragmaID), e_(e) ptr_(ptr) d_(d) a_(a) b_(b) c_(c) bbb_(bbb) { }
	double & e_;
	float * ptr_;
	float d_;
	int & a_;
	int & b_;
	int & c_;
	A * bbb_;
	
	void fx(double & e, float * ptr, float d, int & a, int & b, int & c, A * bbb)
	{
		int yy;
		double *kk;
//		#pragma omp task 
		{
		  class Nested : public NestedBase {
		    Nested(int pragmaID, double * kk, double & e, float * ptr, float & d, int & a, int & b, int & c, A * bbb)  : pragmaID(pragmaID), kk_(kk) e_(e) ptr_(ptr) d_(d) a_(a) b_(b) c_(c) bbb_(bbb) { }
		double * kk_;
		double & e_;
		float * ptr_;
		float & d_;
		int & a_;
		int & b_;
		int & c_;
		A * bbb_;
		
		void fx(double * kk, double & e, float * ptr, float & d, int & a, int & b, int & c, A * bbb)
		{	
			int x;
			float *pp;
			kk = &e;
			ptr = &d;
			a = 1;
			b = 2;
			y(c);
			d = e;
			for(int i = 0; i < b; i ++)
				a ++;

			A aaa;
			aaa.aa = a;
			//bbb.aa = a;
			bbb = new A();
		}
void callme() {
  fx(kk_, e_, ptr_, d_, a_, b_, c_, bbb_);
}
};
Nested _x_(29, kk, e, ptr, d, a, b, c, bbb);
InstanceRun::getInstance("test_cases/test.cpp")->call(_x_);
}

//		#pragma omp for
		//for (int i = 1; i < a; ++i)
{
  class Nested : public NestedBase {
    Nested(int pragmaID, int & a, int & c)  : pragmaID(pragmaID), a_(a) c_(c) { }
int & a_;
int & c_;

void fx(int & a, int & c) {
for(int i = 1 + fp->tid*(a - 1)/fp->numThread; i < 1 + (fp->tid + 1)*(a - 1)/fp->numThread; i ++ )
		{	
			c++;
		}
}
void callme() {
  fx(a_, c_);
}
};
Nested _x_(48, a, c);
InstanceRun::getInstance("test_cases/test.cpp")->call(_x_);
}

	}
void callme() {
  fx(e_, ptr_, d_, a_, b_, c_, bbb_);
}
};
Nested _x_(25, e, ptr, d, a, b, c, bbb);
InstanceRun::getInstance("test_cases/test.cpp")->call(_x_);
}

	return 1;


InstanceRun::getInstance()->joinAllThreads();
}