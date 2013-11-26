// TODO: use delegates/C++11 lambda/Function to transform the ::bind into a REAL empty function pointer
// http://stackoverflow.com/questions/527413/how-boostfunction-and-boostbind-work/527983#527983

/**
 * functional dello standard C++ e di Boost vs funzioni reali vs lambda
 */
#include <iostream>
#include <boost/bind.hpp>
#include <thread>
#include <functional>


class NestedBase
{
public:
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



	static void call(ScheduleOptions opts, NestedBase & nb)
	{	
    	std::thread t(std::ref(nb));
    	t.join();
	}	

};

int main(int argc, char * argv []){
	int b = 0;

	
	{
		struct Nested: public NestedBase
		{
			Nested(int & x1, float x2) : x1_(x1), x2_(x2) {}

			int & x1_; 
			float x2_; 

			void fx(int & x1, float x2) // NAMES ARE SAME OF ORIGINAL VARIABLES
			{
				// inject code here
				x1++;
				std::cout << "PUPPA" << std::endl;
			}

			void callme()
			{
				fx(x1_,x2_);
			}

		};

		std::cout << "pre:" << b << std::endl;
		Nested _x_(b,2.0f);
		InstanceRun::call(InstanceRun::ScheduleOptions(), _x_);
		std::cout << "post:" << b << std::endl;
	}

	
	return b;
}


