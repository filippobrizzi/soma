#include <string>
#include <thread>
#include <vector>
#include <mutex> 
#include <map>
#include <math.h>
#include <iostream>

#include "xml_creator/tinyxml2.h"


int chartoint(const char *cc);
int chartoint(char *cc);

//std::mutex mtx;

class ForParameter
{
public:
  int tid;
  int numThread;
  ForParameter(int tid, int numThread) : tid(tid), numThread(numThread) {}
};

class NestedBase { 
public: 
	NestedBase(int pragmaID) : pragmaID(pragmaID) {}
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
	std::map<int, bool> completedPragma;
	int NumThread;
	std::thread *tl;

	std::map<int, ScheduleOptions> schedopt;

	static InstanceRun* getInstance(std::string filename);

	void call(NestedBase & nb) {
		if(schedopt[nb.pragmaID].threads.size() > 1) {
			for(std::vector<int>::iterator titr = schedopt[nb.pragmaID].threads.begin(); titr != schedopt[nb.pragmaID].threads.end(); ++ titr) {
				nb.fp = new ForParameter(*titr, schedopt[nb.pragmaID].threads.size());
				tl[schedopt[nb.pragmaID].threads[*titr]] = std::thread(std::ref(nb));
			}
		} else if(schedopt[nb.pragmaID].threads.size() == 1) {
    		tl[schedopt[nb.pragmaID].threads[0]] = std::thread(std::ref(nb));
    	}

    	int t;
    	for(int i = 0; i < (schedopt[nb.pragmaID]).barriers.size(); i ++) {
    		while(getCompletedPragma(schedopt[nb.pragmaID].barriers[i]) == false)
			
			t = schedopt[schedopt[nb.pragmaID].barriers[i]].threads[0];
			if(tl[t].joinable())
				tl[t].join();    	
    	}

	}
	void setCompletedPragma(int pragmaID);
	bool getCompletedPragma(int pragmaID);


	/*void joinAllThreads() {
		for(int i = 0; i < NumThread; i ++)
			tl[i].join();
	}
*/
};
