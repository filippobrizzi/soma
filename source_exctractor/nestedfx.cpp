// TODO: use delegates/C++11 lambda/Function to transform the ::bind into a REAL empty function pointer
// http://stackoverflow.com/questions/527413/how-boostfunction-and-boostbind-work/527983#527983

/**
 * functional dello standard C++ e di Boost vs funzioni reali vs lambda
 */
#include <iostream>
#include <boost/bind.hpp>
//#include <functional>


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
	InstanceRun(File scheduleFile); //read the file (xml) and create the ScheduleOptions struct.

	vector<std::Thread> thread_list = new vector<std::Thread>(ScheduleOptions.numThread);
	std::Vector getVectorForFunction(unsigned pid);


	static void call(unsigned pid, NestedBase & nb)
	{	
		std::Thread t = getVectorForFunction(pid);
		nb(); //transform to function pointer
		t(nb());
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
			}

			void callme()
			{
				fx(x1_,x2_);
			}

		};

		std::cout << "pre:" << b << std::endl;
		Nested _x_(b,2.0f);
		InstanceRun::call(pragmaLine, _x_);
		std::cout << "post:" << b << std::endl;
	}

	
	return b;
}


