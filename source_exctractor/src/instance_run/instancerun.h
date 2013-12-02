#include <string>
#include <thread>
#include <vector>
#include <mutex> 
#include <map>
#include <math.h>
#include <iostream>
#include <chrono>

#include "xml_creator/tinyxml2.h"


int chartoint(const char *cc);
int chartoint(char *cc);

//class InstanceRun;

class ForParameter {
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
  	int ActivationTime;
  	
  	virtual void callme() = 0;
  	
  	void operator()(ForParameter *fp);
};

class InstanceRun {
	
	struct ScheduleOptions {
		int pid;
		int ActivationTime;
		int ForSplit;
		//std::vector<int> threads;
		std::vector<int> barriers;
	};

	InstanceRun(std::string filename);

	std::chrono::time_point<std::chrono::system_clock> start;
public:
	
	std::map<int, std::thread *> runningThreads;
	std::map<int, ScheduleOptions> schedopt;

	static InstanceRun* getInstance(std::string filename);

	void call(NestedBase & nb);
	std::chrono::time_point<std::chrono::system_clock> getTimeStart() { return start; };

};
