
#include <unistd.h>

#include "/home/pippo/Documents/Project/soma/source_exctractor/src/thread_pool/threads_pool.h"
class X{
public:
  float* y(int i, int *c);
};

float* X::y (int i, int *c) {
	int a = i;
	int b;
  double n;
  float *d, e;
//	#pragma omp parallel shared(a, n, d, e) period(191)
  {
    class Nested : public NestedBase {
    public: 
      virtual std::shared_ptr<NestedBase> clone() const { return std::make_shared<Nested>(*this); } 
      Nested(int pragma_id, int & a, double & n, float * d, float & e)  : NestedBase(pragma_id), a_(a) , n_(n) , d_(d) , e_(e) {}
  int & a_;
  double & n_;
  float * d_;
  float & e_;
  
  void fx(ForParameter for_param, int & a, double & n, float * d, float & e){
    sleep(1);
  }
void callme(ForParameter for_param) {
fx(for_param, a_, n_, d_, e_);
}
};
ThreadPool::getInstance("test_cases/omp_test.cpp")->call(std::make_shared<Nested>(15, a, n, d, e));
}
	
	//if(a == 5) {
//		#pragma omp parallel for shared(b, a, d) period(1)
 		//for(int j = 124; j < 125; j ++)
{
  class Nested : public NestedBase {
  public: 
    virtual std::shared_ptr<NestedBase> clone() const { return std::make_shared<Nested>(*this); } 
    Nested(int pragma_id)  : NestedBase(pragma_id){}

void fx(ForParameter for_param) {
for(int j = 124 + for_param.thread_id_*(125 - 124)/for_param.num_threads_; j < 124 + (for_param.thread_id_ + 1)*(125 - 124)/for_param.num_threads_; j ++ )
  			 sleep(1);
}
void callme(ForParameter for_param) {
fx(for_param);
}
};
ThreadPool::getInstance("test_cases/omp_test.cpp")->call(std::make_shared<Nested>(21));
}
	//}
	return d;
}


int main() {

 int a,n, b;
 b=0;
 a = 10;
 n = 2;
 int c;
// #pragma omp parallel for shared(a, n, b, c) period(19)
 //for(int i=0;i< 1; i += 1) {
{
  class Nested : public NestedBase {
  public: 
    virtual std::shared_ptr<NestedBase> clone() const { return std::make_shared<Nested>(*this); } 
    Nested(int pragma_id)  : NestedBase(pragma_id){}

void fx(ForParameter for_param) {
for(int i = 0 + for_param.thread_id_*(1 - 0)/for_param.num_threads_; i < 0 + (for_param.thread_id_ + 1)*(1 - 0)/for_param.num_threads_; i += 1) { 
   sleep(1);
 }
}
void callme(ForParameter for_param) {
fx(for_param);
}
};
ThreadPool::getInstance("test_cases/omp_test.cpp")->call(std::make_shared<Nested>(36));
}

// #pragma omp parallel shared(a)
 {
   class Nested : public NestedBase {
   public: 
     virtual std::shared_ptr<NestedBase> clone() const { return std::make_shared<Nested>(*this); } 
     Nested(int pragma_id, int & b, int & a)  : NestedBase(pragma_id), b_(b) , a_(a) {}
 int & b_;
 int & a_;
 
 void fx(ForParameter for_param, int & b, int & a){
//  #pragma omp task shared(b)
  {
    class Nested : public NestedBase {
    public: 
      virtual std::shared_ptr<NestedBase> clone() const { return std::make_shared<Nested>(*this); } 
      Nested(int pragma_id, int & b)  : NestedBase(pragma_id), b_(b) {}
  int & b_;
  
  void fx(ForParameter for_param, int & b){
    b = 5;
    sleep(1);
  }
void callme(ForParameter for_param) {
fx(for_param, b_);
}
};
ThreadPool::getInstance("test_cases/omp_test.cpp")->call(std::make_shared<Nested>(43, b));
}

//  #pragma omp task shared(b) period(19)
  {
    class Nested : public NestedBase {
    public: 
      virtual std::shared_ptr<NestedBase> clone() const { return std::make_shared<Nested>(*this); } 
      Nested(int pragma_id, int & b)  : NestedBase(pragma_id), b_(b) {}
  int & b_;
  
  void fx(ForParameter for_param, int & b){
  	/* code */
    sleep(1);
  }
void callme(ForParameter for_param) {
fx(for_param, b_);
}
};
ThreadPool::getInstance("test_cases/omp_test.cpp")->call(std::make_shared<Nested>(49, b));
}

//  #pragma omp for
    //for(int i=1;i< 2; i ++) {
{
  class Nested : public NestedBase {
  public: 
    virtual std::shared_ptr<NestedBase> clone() const { return std::make_shared<Nested>(*this); } 
    Nested(int pragma_id)  : NestedBase(pragma_id){}

void fx(ForParameter for_param) {
for(int i = 1 + for_param.thread_id_*(2 - 1)/for_param.num_threads_; i < 1 + (for_param.thread_id_ + 1)*(2 - 1)/for_param.num_threads_; i ++ ) { 
      sleep(1);
    }
}
void callme(ForParameter for_param) {
fx(for_param);
}
};
ThreadPool::getInstance("test_cases/omp_test.cpp")->call(std::make_shared<Nested>(55));
}
  
//  #pragma omp task shared(b)
  {
    class Nested : public NestedBase {
    public: 
      virtual std::shared_ptr<NestedBase> clone() const { return std::make_shared<Nested>(*this); } 
      Nested(int pragma_id, int & b)  : NestedBase(pragma_id), b_(b) {}
  int & b_;
  
  void fx(ForParameter for_param, int & b){
//    #pragma omp task shared(b)
    {
      class Nested : public NestedBase {
      public: 
        virtual std::shared_ptr<NestedBase> clone() const { return std::make_shared<Nested>(*this); } 
        Nested(int pragma_id, int & b)  : NestedBase(pragma_id), b_(b) {}
    int & b_;
    
    void fx(ForParameter for_param, int & b){
      sleep(1);
    }
void callme(ForParameter for_param) {
fx(for_param, b_);
}
};
ThreadPool::getInstance("test_cases/omp_test.cpp")->call(std::make_shared<Nested>(62, b));
}
      sleep(1);
//    #pragma omp task shared(b)
    {
      class Nested : public NestedBase {
      public: 
        virtual std::shared_ptr<NestedBase> clone() const { return std::make_shared<Nested>(*this); } 
        Nested(int pragma_id, int & b)  : NestedBase(pragma_id), b_(b) {}
    int & b_;
    
    void fx(ForParameter for_param, int & b){
      X w;
      int y;
      int *z;
      sleep(1);
      w.y(y, z);
    }
void callme(ForParameter for_param) {
fx(for_param, b_);
}
};
ThreadPool::getInstance("test_cases/omp_test.cpp")->call(std::make_shared<Nested>(67, b));
}

  }
void callme(ForParameter for_param) {
fx(for_param, b_);
}
};
ThreadPool::getInstance("test_cases/omp_test.cpp")->call(std::make_shared<Nested>(60, b));
}
//  #pragma omp task shared(b)
  {
    class Nested : public NestedBase {
    public: 
      virtual std::shared_ptr<NestedBase> clone() const { return std::make_shared<Nested>(*this); } 
      Nested(int pragma_id, int & b)  : NestedBase(pragma_id), b_(b) {}
  int & b_;
  
  void fx(ForParameter for_param, int & b){
    sleep(1);
  }
void callme(ForParameter for_param) {
fx(for_param, b_);
}
};
ThreadPool::getInstance("test_cases/omp_test.cpp")->call(std::make_shared<Nested>(77, b));
}

 }
void callme(ForParameter for_param) {
fx(for_param, b_, a_);
}
};
ThreadPool::getInstance("test_cases/omp_test.cpp")->call(std::make_shared<Nested>(41, b, a));
}

// #pragma omp parallel shared(b)
 {
   class Nested : public NestedBase {
   public: 
     virtual std::shared_ptr<NestedBase> clone() const { return std::make_shared<Nested>(*this); } 
     Nested(int pragma_id, int & n, int & b)  : NestedBase(pragma_id), n_(n) , b_(b) {}
 int & n_;
 int & b_;
 
 void fx(ForParameter for_param, int & n, int & b){
//    #pragma omp for period(12)
    //for(int i=1;i< n; i += 3) {
{
  class Nested : public NestedBase {
  public: 
    virtual std::shared_ptr<NestedBase> clone() const { return std::make_shared<Nested>(*this); } 
    Nested(int pragma_id, int & n)  : NestedBase(pragma_id), n_(n) {}
int & n_;

void fx(ForParameter for_param, int & n) {
for(int i = 1 + for_param.thread_id_*(n - 1)/for_param.num_threads_; i < 1 + (for_param.thread_id_ + 1)*(n - 1)/for_param.num_threads_; i += 3) { 
      sleep(1);
    }
}
void callme(ForParameter for_param) {
fx(for_param, n_);
}
};
ThreadPool::getInstance("test_cases/omp_test.cpp")->call(std::make_shared<Nested>(86, n));
}
 }
void callme(ForParameter for_param) {
fx(for_param, n_, b_);
}
};
ThreadPool::getInstance("test_cases/omp_test.cpp")->call(std::make_shared<Nested>(84, n, b));
}

}
