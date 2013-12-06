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


class ForParameter {
public:
	int thread_id_;
  	int num_threads_;
  	ForParameter(int thread_id, int num_threads) : thread_id_(thread_id), num_threads_(num_threads) {}
};

class NestedBase { 
public: 

	NestedBase(int pragma_id) : pragma_id_(pragma_id) {}

  	int pragma_id_;
  	int activation_time_;
  	
  	virtual void callme(ForParameter for_param) = 0;
  	
  	void operator()(ForParameter for_param);
};

class InstanceRun {
	
	struct ScheduleOptions {
		int pragma_id_;
		int activation_time_;
		/* In case of a parallel for, indicates in how many threads to split the for */
		int for_split_;

		/* List of pragma_id_ to wait before completing the task */
		std::vector<int> barriers_;
	};

	InstanceRun(std::string file_name);

public:
	/* List of the pragma that are currently running. When a pragma completes it is deleted from the map */
	std::map<int, std::thread *> running_threads_;
	/* Contains the schedule options for each pragma in the program */
	std::map<int, ScheduleOptions> sched_opt_;

	static InstanceRun* getInstance(std::string file_name);

	void call(std::shared_ptr<NestedBase> nested_base);
	std::chrono::time_point<std::chrono::system_clock> getTimeStart() { return program_start_time_; };

};
