
#include <fstream>
#include <time.h>     
#include <iostream>
#include <thread>
#include <unistd.h>


#define log_file "log_file.xml"


struct ProfileTrackParams {

	ProfileTrackParams(int funct_id, int pragma_line)
		: funct_id_(funct_id) ,pragma_line_(pragma_line), num_for_iteration_set_(false) {}
	/* Costructor for parallel for */
	ProfileTrackParams(int funct_id, int pragma_line, int n) 
		: funct_id_(funct_id), pragma_line_(pragma_line), num_for_iteration_(n), num_for_iteration_set_(true) {}

	int funct_id_; 
	int pragma_line_;
	/* In the case of a parallel for this variable saves the number of the iteration of the for */
	int num_for_iteration_;
	bool num_for_iteration_set_;
};

/*
 * ----- Class that keep track of the children time and the father of the current pragma in execution ----
 */
class ProfileTracker {

	time_t start_time_;
	time_t end_time_;
	//clock_t start_time_;
	//clock_t end_time_;

	int num_for_iteration_;
	bool num_for_iteration_set_;

	/* These functions print the result of the profiling in a log file */
	void PrintPragma();
	void PrintFunction();

public:
	int pragma_line_;
	int funct_id_;

	double elapsed_time_;
	/* Time spent by the children of the current pragma or function */
	double children_elapsed_time_;
	
	/* Keeps track of which function/pragma has invoked the current function/pragma */
	ProfileTracker *previous_pragma_executed_;

	/* In the costructor a timer is started */
	ProfileTracker(const ProfileTrackParams & p);
	/* In the destructor the timer is stopped and the elapsed time is written in the log file */
	~ProfileTracker();

	/* This is necessary to allow to create an object inside the declaration of an if stmt */
	operator bool() const  { return true; }
};

/*
 * ---- Singleton class that open and close the log file -----
 */
class ProfileTrackerLog {

	/* Keeps track of which function/pragma has invoked the current function/pragma */
	ProfileTracker *current_pragma_executing_;

	/* Create the log file and write in it the hardware spec */
	ProfileTrackerLog ();

	void WriteArchitecturesSpec();
	size_t getTotalSystemMemory();    

public:
	/* File where the log is written */
	std::ofstream log_file_;

	static ProfileTrackerLog* getInstance();
	/* Substitute the pointer of the current pragma in execution and return the previous value */
	ProfileTracker *ReplaceCurrentPragma(ProfileTracker *current_pragma_executing_);
	
	/* Save and close the log file */
	~ProfileTrackerLog();

};

