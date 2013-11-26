//#include <string>


class A {

public:
	int aa;
};


class NestedBase { 
  public: 
  virtual void callme() = 0;
  void operator()() {
    callme();
  }
};

class InstanceRun {
public:
  struct ScheduleOptions
  {
  };

  static void call(ScheduleOptions opts, NestedBase & nb) {
      std::thread t(std::ref(nb));
      t.join();
  }
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
	A *bbb;
	//std::string ss;
//	#pragma omp parallel shared(a, b, c) private(d, bbb)
	{
	  class Nested : public NestedBase {
	    Nested(double & e, float * ptr, float d, int & a, int & c, int & b, A * bbb)  : e_(e) ptr_(ptr) d_(d) a_(a) c_(c) b_(b) bbb_(bbb) { }
	double & e_;
	float * ptr_;
	float d_;
	int & a_;
	int & c_;
	int & b_;
	A * bbb_;
	
	void fx(double & e, float * ptr, float d, int & a, int & c, int & b, A * bbb)
	{
		int yy;
		double *kk;
//		#pragma omp task 
		{
		  class Nested : public NestedBase {
		    Nested(double * kk, double & e, float * ptr, float & d, int & a, int & c, int & b, A * bbb)  : kk_(kk) e_(e) ptr_(ptr) d_(d) a_(a) c_(c) b_(b) bbb_(bbb) { }
		double * kk_;
		double & e_;
		float * ptr_;
		float & d_;
		int & a_;
		int & c_;
		int & b_;
		A * bbb_;
		
		void fx(double * kk, double & e, float * ptr, float & d, int & a, int & c, int & b, A * bbb)
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
			//bbb.aa = a;
			bbb = new A();
		}
void callme() {
  fx(kk_, e_, ptr_, d_, a_, c_, b_, bbb_);
}
};
Nested _x_(kk, e, ptr, d, a, c, b, bbb);
InstanceRun::call(InstanceRun::ScheduleOptions(), _x_);
}

//		#pragma omp for
		{
		  class Nested : public NestedBase {
		    Nested(int & c)  : c_(c) { }
		int & c_;
		
		void fx(int & c)
		for (int i = 10; i < 100; ++i)
		{	
			c++;
		}
void callme() {
  fx(c_);
}
};
Nested _x_(c);
InstanceRun::call(InstanceRun::ScheduleOptions(), _x_);
}

	}
void callme() {
  fx(e_, ptr_, d_, a_, c_, b_, bbb_);
}
};
Nested _x_(e, ptr, d, a, c, b, bbb);
InstanceRun::call(InstanceRun::ScheduleOptions(), _x_);
}

	return 1;


}