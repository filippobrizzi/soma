#include <iostream>
#include <boost/bind.hpp>
#include <thread>
#include <functional>

class ForParameter 
{
public:
	//value from 0 to n-1
	int tid;
	int numThread;
	
	ForParameter(int tid, int numThread) : tid(tid), numThread(numThread) {}
};

class NestedBase
{
public:
	ForParameter *fp;
	unsigned pragmaID;

	virtual void callme() = 0;
	void operator()()
	{
		callme();
	}
};


class InstanceRun
{
public:
	struct ScheduleOptions
	{

	};
/*
	thread *tl = new thread[n];
	//thread tl[10];
	for(int i = 0; i < n; i ++)
		tl[i] = thread(foo, i);


	for(int i = 0; i < n; i ++)
		tl[i].join();

*/
	static void call(ScheduleOptions opts, NestedBase & nb)
	{	

		nb.fp = new ForParameter(0, 2);
    	std::thread t(std::ref(nb));
    	t.join();
	}	

};

int main(int argc, char * argv []){
	int b = 0;

	
	{
		struct Nested: public NestedBase
		{
			Nested(unsigned pragmaID, int & x1, float x2) : pragmaID(pragmaID), x1_(x1), x2_(x2) {}

			int & x1_; 
			float x2_; 

			void fx(int & x1, float x2) // NAMES ARE SAME OF ORIGINAL VARIABLES
			{
				// inject code here
				for(int i = 0 + fp->tid * (10 - 0)/fp->numThread; i < 0 + (fp->tid+1) * (10 - 0)/fp->numThread; i ++) {
					x1++;
					std::cout << "PUPPA" << std::endl;
				}
			}

			void callme()
			{
				fx(x1_,x2_);
				std::cout << "PUPPA 3" << std::endl;
			}

		};

		std::cout << "pre:" << b << std::endl;
		Nested _x_(pragmaID, b,2.0f);
		InstanceRun::call(InstanceRun::ScheduleOptions(), _x_);
		InstanceRun::getInstance()->call(_x_);

		std::cout << "post:" << b << std::endl;
	}

	
	return b;
}


