#include <string>
#include <thread>
#include <vector>
#include <mutex> 
#include <map>
#include <math.h>

#include "xml_creator/tinyxml2.h"


int chartoint(const char *cc);
int chartoint(char *cc);

std::mutex mtx;

class ForParameter
{
public:
  int tid;
  int numThread;
  ForParameter(int tid, int numThread) : tid(tid), numThread(numThread) {}
};

class NestedBase { 
public: 
  ForParameter *fp;
  int pragmaID;
  virtual void callme() = 0;
  void operator()() {
    callme();
  }
};

class InstanceRun {
	
	struct ScheduleOptions {
		int pid;
		std::vector<int> threads;
		std::vector<int> barriers;
	};

	InstanceRun(std::string filename);

	//static std::mutex mtx;
public:
		
	int NumThread;
	std::thread *tl;

	std::map<int, ScheduleOptions> schedopt;
	/* = new thread[n];
	//thread tl[10];
	for(int i = 0; i < n; i ++)
		tl[i] = thread(foo, i);


	for(int i = 0; i < n; i ++)
		tl[i].join();
*/
	static InstanceRun* getInstance(std::string filename);

	void call(NestedBase & nb) {	
		
		for(int i = 0; i < (schedopt[nb.pragmaID]).barriers.size(); i ++)
			tl[schedopt[nb.pragmaID].barriers[i]].join();

		if(schedopt[nb.pragmaID].threads.size() > 1) {
			for(std::vector<int>::iterator titr = schedopt[nb.pragmaID].threads.begin(); titr != schedopt[nb.pragmaID].threads.end(); ++ titr) {
				nb.fp = new ForParameter(*titr, schedopt[nb.pragmaID].threads.size());
				tl[schedopt[nb.pragmaID].threads[*titr]] = std::thread(std::ref(nb));
			}
		} else {
		//nb.fp = new ForParameter(0, 2);
    		tl[schedopt[nb.pragmaID].threads[0]] = std::thread(std::ref(nb));
    	//t.join();
    	}
	}

	void joinAllThreads() {
		for(int i = 0; i < NumThread; i ++)
			tl[i].join();
	}

};
