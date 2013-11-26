#include <omp.h>
#include <stdio.h>

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
 
int y();
int z();

int main() {
	int a, b, c;

//	#pragma omp parallel 
	{
		class Nested : public NestedBase {
		  	Nested(int & a, int & b, int & c)  : a_(a) b_(b) c_(c) { }
			int & a_;
			int & b_;
			int & c_;
		
			void fx(int & a, int & b, int & c)
			{
			
	//		#pragma omp task
				{
			  		class Nested : public NestedBase {
			    		Nested(int & a, int & b, int & c)  : a_(a) b_(b) c_(c) { }
						int & a_;
						int & b_;
						int & c_;
			
						void fx(int & a, int & b, int & c)
						{
							a = 5;

	//			#pragma omp task
							{
				  				class Nested : public NestedBase {
				    				Nested(int & b)  : b_(b) { }
									int & b_;
				
									void fx(int & b)
									{
										b = 5;
									}		
									void callme() {
	  									fx(b_);
									}
								};
								Nested _x_(b);
								InstanceRun::call(InstanceRun::ScheduleOptions(), _x_);
							}

	//			#pragma omp task
							{
					  			class Nested : public NestedBase {
					    			Nested(int & c)  : c_(c) { }
									int & c_;
					
									void fx(int & c)
									{
										c = 6;
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
						  fx(a_, b_, c_);
						}
					};
					Nested _x_(a, b, c);
					InstanceRun::call(InstanceRun::ScheduleOptions(), _x_);
				}
			}
			void callme() {
	  			fx(a_, b_, c_);
			}
		};
		Nested _x_(a, b, c);
		InstanceRun::call(InstanceRun::ScheduleOptions(), _x_);
	}
}


