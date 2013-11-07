#include "/Users/admin/Documents/clangparsing/ProfileTracker.h"


std::auto_ptr<ProfileTrackerLog> ProfileTrackerLog::log;

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
  	log_file << "MemorySize=\"2000\"/>" << std::endl;
}	

/*
size_t ProfileTrackerLog::getTotalSystemMemory() {
   	long pages = sysconf(_SC_PHYS_PAGES);
   	long page_size = sysconf(_SC_PAGE_SIZE);
   	return (pages * page_size)/1024/1024;
}
*/

ProfileTrackerLog* ProfileTrackerLog::getInstance() {
	if (!log.get()) {
        log = std::auto_ptr<ProfileTrackerLog>(new ProfileTrackerLog());
    }
    return log.get();
}

ProfileTrackerLog::~ProfileTrackerLog() {
	log_file << "</LogFile>" << std::endl;
	log_file.close();
}




/* 
 * ---- PROFILE TRACKER ----
 */
ProfileTracker::ProfileTracker(const ProfileTrackParams & p) {
	last = ProfileTrackerLog::getInstance()->top;
	ProfileTrackerLog::getInstance()->top = this;
	
	childrenElapsed = 0;	

	_pragmaLine = p.pragmaLine;
	_functID = p.functID;
	nset = p.nset;

	if(nset == true)
		n = p.n;

	time(&startTime);
}

ProfileTracker::~ProfileTracker() {
	time(&endTime);
	elapsedTime = difftime(endTime, startTime);
	if(last != NULL) {
		last->childrenElapsed += elapsedTime;
	}	
	ProfileTrackerLog::getInstance()->top = last;

	if(_pragmaLine == 0)
		printFunction();
	else
		printPragma();

}
 
void ProfileTracker::printPragma() {
	ProfileTrackerLog::getInstance()->log_file << "	<Pragma";
	ProfileTrackerLog::getInstance()->log_file << " fid=\"" << _functID << "\" pid=\"" << _pragmaLine << "\" ";
	if(last != NULL) {
		if( last->_pragmaLine != 0)
			ProfileTrackerLog::getInstance()->log_file << "callerid=\"" << last->_pragmaLine << "\" "; 
		else 
			ProfileTrackerLog::getInstance()->log_file << "callerid=\"" << last->_functID << "\" "; 
	}
	ProfileTrackerLog::getInstance()->log_file << "elapsedTime=\"" << elapsedTime << "\" ";
	ProfileTrackerLog::getInstance()->log_file << "childrenTime=\"" << childrenElapsed << "\"";
	if(nset == true)
	 	ProfileTrackerLog::getInstance()->log_file << " loops=\"" << n << "\"";
	ProfileTrackerLog::getInstance()->log_file << "/>" << std::endl; 

}

void ProfileTracker::printFunction() {
	ProfileTrackerLog::getInstance()->log_file << "	<Function";
	ProfileTrackerLog::getInstance()->log_file << " fid=\"" << _functID << "\" ";
	if(last != NULL) {
		if( last->_pragmaLine != 0)
			ProfileTrackerLog::getInstance()->log_file << "callerid=\"" << last->_pragmaLine << "\" "; 
		else 
			ProfileTrackerLog::getInstance()->log_file << "callerid=\"" << last->_functID << "\" "; 
	}
	ProfileTrackerLog::getInstance()->log_file << "elapsedTime=\"" << elapsedTime <<  "\" ";
	ProfileTrackerLog::getInstance()->log_file << "childrenTime=\"" << childrenElapsed <<"\"/>" << std::endl; 


}
