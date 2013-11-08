#include "/home/pippo/Documents/Library/clomp-master/include/myprogram/profiling/ProfileTracker.h"


/*
 * ---- PROFILE TRACKER LOG ----
 */
ProfileTrackerLog::ProfileTrackerLog () {
	top = NULL;
	log_file.open(logFile);
	log_file << "<LogFile>" << std::endl;
	writeArchitecturesSpec();
}

void ProfileTrackerLog::writeArchitecturesSpec() {
	log_file << "	<Hardware ";
	log_file << "NumberofCores=\"" << std::thread::hardware_concurrency() << "\" ";
  	log_file << "MemorySize=\"" << getTotalSystemMemory() << "\"/>" << std::endl;
}	

size_t ProfileTrackerLog::getTotalSystemMemory() {
   	long pages = sysconf(_SC_PHYS_PAGES);
   	long page_size = sysconf(_SC_PAGE_SIZE);
   	return (pages * page_size)/1024/1024;
}

ProfileTrackerLog* ProfileTrackerLog::getInstance() {

    static ProfileTrackerLog log;
    return &log;
}

ProfileTrackerLog::~ProfileTrackerLog() {
	log_file << "</LogFile>" << std::endl;
	log_file.close();
}

ProfileTracker *ProfileTrackerLog::replaceTop(ProfileTracker *top) {
	ProfileTracker *tmp = this->top;
	this->top = top;
	return tmp;
}


/* 
 * ---- PROFILE TRACKER ----
 */
ProfileTracker::ProfileTracker(const ProfileTrackParams & p) {
	last = ProfileTrackerLog::getInstance()->replaceTop(this);
	
	childrenElapsed = 0;	

	_pragmaLine = p.pragmaLine;
	_functID = p.functID;
	nset = p.nset;

	if(nset)
		n = p.n;

	time(&startTime);
}

ProfileTracker::~ProfileTracker() {
	time(&endTime);
	elapsedTime = difftime(endTime, startTime);
	if(last) {
		last->childrenElapsed += elapsedTime;
	}	
	ProfileTrackerLog::getInstance()->replaceTop(last);
	//ProfileTrackerLog::getInstance()->top = last;

	if(_pragmaLine == 0)
		printFunction();
	else
		printPragma();

}
 
void ProfileTracker::printPragma() {
	ProfileTrackerLog::getInstance()->log_file << "	<Pragma" \
											 << " fid=\"" << _functID << "\" pid=\"" << _pragmaLine << "\" ";
	if(last) {
		if( last->_pragmaLine != 0)
			ProfileTrackerLog::getInstance()->log_file << "callerid=\"" << last->_pragmaLine << "\" "; 
		else 
			ProfileTrackerLog::getInstance()->log_file << "callerid=\"" << last->_functID << "\" "; 
	}
	ProfileTrackerLog::getInstance()->log_file << "elapsedTime=\"" << elapsedTime << "\" " \
											   << "childrenTime=\"" << childrenElapsed << "\"";
	if(nset)
	 	ProfileTrackerLog::getInstance()->log_file << " loops=\"" << n << "\"";
	ProfileTrackerLog::getInstance()->log_file << "/>" << std::endl; 

}

void ProfileTracker::printFunction() {
	ProfileTrackerLog::getInstance()->log_file << "	<Function" \
												 << " fid=\"" << _functID << "\" ";
	if(last) {
		if( last->_pragmaLine != 0)
			ProfileTrackerLog::getInstance()->log_file << "callerid=\"" << last->_pragmaLine << "\" "; 
		else 
			ProfileTrackerLog::getInstance()->log_file << "callerid=\"" << last->_functID << "\" "; 
	}
	ProfileTrackerLog::getInstance()->log_file << "elapsedTime=\"" << elapsedTime <<  "\" " \
											   << "childrenTime=\"" << childrenElapsed <<"\"/>" << std::endl; 


}
