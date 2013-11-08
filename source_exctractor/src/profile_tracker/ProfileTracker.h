
#include <fstream>
#include <time.h>     
#include <iostream>
#include <thread>
#include <unistd.h>


#define logFile "log_file.xml"


struct ProfileTrackParams {
	ProfileTrackParams(int functID, int pragmaLine) : functID(functID) ,pragmaLine(pragmaLine),nset(false) {}
	ProfileTrackParams(int functID, int pragmaLine, int n) : functID(functID) ,pragmaLine(pragmaLine), n(n), nset(true) {}

	int functID; 
	int pragmaLine;
	int n;
	bool nset;
};

/*
 * ----- Class that keep track of the children time and the father of the current pragma in execution ----
 */
class ProfileTracker {

	time_t startTime;
	time_t endTime;


	int n;
	bool nset;

	void printPragma();
	void printFunction();

public:
	int _pragmaLine;
	int _functID;

	double elapsedTime;
	double childrenElapsed;
	
	ProfileTracker *last;

	ProfileTracker(const ProfileTrackParams & p);
	~ProfileTracker();

	operator bool() const  { return true; }
};

/*
 * ---- Singleton class that open and close the log file -----
 */
class ProfileTrackerLog {

	ProfileTracker *top;


	ProfileTrackerLog ();

	void writeArchitecturesSpec();
	size_t getTotalSystemMemory();    

public:
	std::ofstream log_file;


	static ProfileTrackerLog* getInstance();

	ProfileTracker *replaceTop(ProfileTracker *top);
	~ProfileTrackerLog();

};

