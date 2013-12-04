#include "ProfileTracker.h"


/*
 * ---- PROFILE TRACKER LOG ----
 */
ProfileTrackerLog::ProfileTrackerLog () {
	current_pragma_executing_ = NULL;
	log_file_.open(log_file);
	log_file_ << "<LogFile>" << std::endl;
	WriteArchitecturesSpec();
}

void ProfileTrackerLog::WriteArchitecturesSpec() {
	log_file_ << "	<Hardware ";
	log_file_ << "NumberofCores=\"" << std::thread::hardware_concurrency() << "\" ";
  	log_file_ << "MemorySize=\"" << getTotalSystemMemory() << "\"/>" << std::endl;
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
	log_file_ << "</LogFile>" << std::endl;
	log_file_.close();
}

ProfileTracker *ProfileTrackerLog::ReplaceCurrentPragma(ProfileTracker *current_pragma_executing) {
	ProfileTracker *tmp = current_pragma_executing_;
	current_pragma_executing_ = current_pragma_executing;
	return tmp;
}


/* 
 * ---- PROFILE TRACKER ----
 */
ProfileTracker::ProfileTracker(const ProfileTrackParams & p) {
	previous_pragma_executed_ = ProfileTrackerLog::getInstance()->ReplaceCurrentPragma(this);
	
	children_elapsed_time_ = 0;	

	pragma_line_ = p.pragma_line_;
	funct_id_ = p.funct_id_;
	num_for_iteration_set_ = p.num_for_iteration_;

	if(num_for_iteration_set_)
		num_for_iteration_ = p.num_for_iteration_;

	time(&start_time_);
}

ProfileTracker::~ProfileTracker() {
	time(&end_time_);
	elapsed_time_ = difftime(end_time_, start_time_);
	if(previous_pragma_executed_) {
		previous_pragma_executed_->children_elapsed_time_ += elapsed_time_;
	}	
	ProfileTrackerLog::getInstance()->ReplaceCurrentPragma(previous_pragma_executed_);

	if(pragma_line_ == 0)
		PrintFunction();
	else
		PrintPragma();

}
 
void ProfileTracker::PrintPragma() {
	ProfileTrackerLog::getInstance()->log_file_ << "	<Pragma" \
											 << " fid=\"" << funct_id_ << "\" pid=\"" << pragma_line_ << "\" ";
	if(previous_pragma_executed_) {
		if(previous_pragma_executed_->pragma_line_ != 0)
			ProfileTrackerLog::getInstance()->log_file_ << "callerid=\"" << previous_pragma_executed_->pragma_line_ << "\" "; 
		else 
			ProfileTrackerLog::getInstance()->log_file_ << "callerid=\"" << previous_pragma_executed_->funct_id_ << "\" "; 
	}
	ProfileTrackerLog::getInstance()->log_file_ << "elapsedTime=\"" << elapsed_time_ << "\" " \
											   << "childrenTime=\"" << children_elapsed_time_ << "\"";
	if(num_for_iteration_set_)
	 	ProfileTrackerLog::getInstance()->log_file_ << " loops=\"" << num_for_iteration_ << "\"";
	ProfileTrackerLog::getInstance()->log_file_ << "/>" << std::endl; 

}

void ProfileTracker::PrintFunction() {
	ProfileTrackerLog::getInstance()->log_file_ << "	<Function" \
												 << " fid=\"" << funct_id_ << "\" ";
	if(previous_pragma_executed_) {
		if(previous_pragma_executed_->pragma_line_ != 0)
			ProfileTrackerLog::getInstance()->log_file_ << "callerid=\"" << previous_pragma_executed_->pragma_line_ << "\" "; 
		else 
			ProfileTrackerLog::getInstance()->log_file_ << "callerid=\"" << previous_pragma_executed_->funct_id_ << "\" "; 
	}
	ProfileTrackerLog::getInstance()->log_file_ << "elapsedTime=\"" << elapsed_time_ <<  "\" " \
											   << "childrenTime=\"" << children_elapsed_time_ << "\"/>" << std::endl; 


}
