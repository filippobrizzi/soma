#include <omp.h>
#include <stdio.h>

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

	std::vector <std::thread> threadPool;
	//given the task id (line number) this function returns the thread where to allocate the function
	std::thread getThread(unsigned pid);

	static void call(ScheduleOptions opts, unsigned pid, NestedBase & nb)
	{	
    	std::thread t = getThread(pid);
    	t(std::ref(nb));
    	t.join();
	}	

};

int main() {

	int a, b, c;

	{
		class Nested : public NestedBase {
			Nested(int & a, int & b, int & c) : a(a), b(b) c(c) {}
			int a, b, c;

			void fx(int & a, int & b, int & c) {
				{
					class Nested : public NestedBase {
						Nested(int & a, int & b, int & c) : a(a), b(b) c(c) {}
						int a, b, c;
						void fx(int & a, int & b, int & c) {
							a = 5;
							{
								class Nested : public NestedBase {
									Nested(int & b) : b(b) {}
									int b;
									void fx(int &b) {
										b = 5;
									}
									void callme(){
										fx(b);
									}
								};
								Nested _x_(b);
								InstanceRun::call(InstanceRun::ScheduleOptions(), 17, _x_);
							}

							{
								class Nested : public NestedBase {
									Nested(int & c) : c(c) {}
									int c;
									void fx(int &b) {
										c = 6;
									}
									void callme(){
										fx(c);
									}
								};
								Nested _x_(c);
								InstanceRun::call(InstanceRun::ScheduleOptions(), 22, _x_);
							}

						}
						void callme(){
							fx(a, b, c);
						}
					};
					Nested _x_(a, b, c);
					InstanceRun::call(InstanceRun::ScheduleOptions(), 13, _x_);
				}
			}
			void callme(){
				fx(a, b, c);
			}
		};
		Nested _x_(a, b, c);
		InstanceRun::call(InstanceRun::ScheduleOptions(), 10, _x_);
	}

}