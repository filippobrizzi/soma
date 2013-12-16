#include <stdio.h>
//#include <iostream>
#include <unistd.h>

//int y();

#include "/home/pippo/Documents/Project/soma/source_exctractor/src/thread_pool/threads_pool.h"
int main() {

	int a;
//	#pragma omp parallel private(a)
	{
	  class Nested : public NestedBase {
	  public: 
	    virtual std::shared_ptr<NestedBase> clone() const { return std::make_shared<Nested>(*this); } 
	    Nested(int pragma_id, int a)  : NestedBase(pragma_id), a_(a) {}
	int a_;
	
	void fx(ForParameter for_param, int a){	
 		
 		sleep(1);
 		
 	}
void callme(ForParameter for_param) {
fx(for_param, a_);
}
};
ThreadPool::getInstance("test_cases/omp_parallel.c")->call(std::make_shared<Nested>(11, a));
}

// 	#pragma omp parallel
 	{
 	  class Nested : public NestedBase {
 	  public: 
 	    virtual std::shared_ptr<NestedBase> clone() const { return std::make_shared<Nested>(*this); } 
 	    Nested(int pragma_id)  : NestedBase(pragma_id){}
 	
 	void fx(ForParameter for_param){ 
 	
 		sleep(1);
 		
 	}
void callme(ForParameter for_param) {
fx(for_param);
}
};
ThreadPool::getInstance("test_cases/omp_parallel.c")->call(std::make_shared<Nested>(18));
}
 	
 //	int a = y();

 	//#pragma omp master
 	//printf("hello world\n");

//	#pragma omp parallel for
	//for(int i=0;i<2;i++) {
{
  class Nested : public NestedBase {
  public: 
    virtual std::shared_ptr<NestedBase> clone() const { return std::make_shared<Nested>(*this); } 
    Nested(int pragma_id)  : NestedBase(pragma_id){}

void fx(ForParameter for_param) {
for(int i = 0 + for_param.thread_id_*(2 - 0)/for_param.num_threads_; i < 0 + (for_param.thread_id_ + 1)*(2 - 0)/for_param.num_threads_; i ++ ) { 
		sleep(1);
	}
}
void callme(ForParameter for_param) {
fx(for_param);
}
};
ThreadPool::getInstance("test_cases/omp_parallel.c")->call(std::make_shared<Nested>(30));
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