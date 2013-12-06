
#include "instancerun.h"

std::mutex mtx;

void NestedBase::operator()(ForParameter for_param) {
	std::cout << "Thread: " << pragma_id_ << " tid: " << for_param.thread_id_ << " forsplit " << for_param.num_threads_ << std::endl;
  		/*std::chrono::time_point<std::chrono::system_clock> program_start_time = InstanceRun::getInstance("")->getTimeStart();
  		std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();
  		int elapsed_milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(now - program_start_time).count();
  		if(activation_time_ - elapsed_milliseconds > 0) {
  			std::chrono::milliseconds duration(activation_time_ - elapsed_milliseconds);
  			std::this_thread::sleep_for(duration);
  		}*/
    	callme(for_param);
}

InstanceRun* InstanceRun::getInstance(std::string file_name) {
	mtx.lock();
	static InstanceRun run(file_name);
	mtx.unlock();
	return &run;
}


InstanceRun::InstanceRun(std::string file_name) {
	std::string in_xml_file (file_name);
  	size_t ext = in_xml_file.find_last_of(".");
  	if (ext == std::string::npos)
    	ext = in_xml_file.length();
  	in_xml_file = in_xml_file.substr(0, ext);
  	in_xml_file.insert(ext, "_schedule.xml");

  	tinyxml2::XMLDocument xml_doc;
 	//xml_doc.LoadFile(in_xml_file.c_str());
 	xml_doc.LoadFile("test_schedule.xml");

	tinyxml2::XMLElement *pragma_element = xml_doc.FirstChildElement("File")->FirstChildElement("Pragma");
	while(pragma_element != NULL) {
		ScheduleOptions sched_opt;

		const char* pragma_id = pragma_element->FirstChildElement("ID")->GetText();
		int id = chartoint(pragma_id);
		sched_opt.pragma_id_ = id;

		sched_opt.activation_time_ = 0;
		tinyxml2::XMLElement *activation_time_element = pragma_element->FirstChildElement("ActivationTime");
		if(activation_time_element) {
			const char* activation_time = activation_time_element->GetText();
			sched_opt.activation_time_ = chartoint(activation_time);
		}

		tinyxml2::XMLElement *barriers_element = pragma_element->FirstChildElement("Barriers");
		if(barriers_element != NULL)
			barriers_element = barriers_element->FirstChildElement("PragmaID");
		while(barriers_element != NULL){
			const char *thread_id = barriers_element->GetText();
			sched_opt.barriers_.push_back(chartoint(thread_id));

			barriers_element = barriers_element->NextSiblingElement("PragmaID");
		}

		/*sched_opt.for_split_ = 1;
		tinyxml2::XMLElement *for_split_element = pragma_element->FirstChildElement("ForSplit");
		if(for_split_element) {
			const char* for_split = for_split_element->GetText();
			sched_opt.for_split_ = chartoint(for_split);
		}*/

		sched_opt_[id] = sched_opt;

		pragma_element = pragma_element->NextSiblingElement("Pragma");
	}

	program_start_time_ = std::chrono::system_clock::now();
}



int chartoint(const char *cc){
	std::string s(cc);
	char c;
	int n = 0;
	int tmp;
	int i = s.size();
 	for(std::string::iterator sitr = s.begin(); sitr != s.end(); ++ sitr){
 		c = *sitr;
 		tmp = c - 48;
 		tmp = tmp*pow(10, i-1);
 		n += tmp;
 		i --;
 	}
 	return n;
}

int chartoint(char *cc){
	const char *c = cc;
	return chartoint(c);
}


void InstanceRun::call(std::shared_ptr<NestedBase> nested_b) {
		
	running_threads_[nested_b->pragma_id_] = new std::thread[sched_opt_[nested_b->pragma_id_].for_split_];
	nested_b->activation_time_ = sched_opt_[nested_b->pragma_id_].activation_time_;

 	bool isBarrier = sched_opt_[nested_b->pragma_id_].activation_time_ == 0;
	if(!isBarrier) {
		for(int i = 0; i < sched_opt_[nested_b->pragma_id_].for_split_; i ++) {
			running_threads_[nested_b->pragma_id_][i] = std::thread(nested_b, ForParameter(i, sched_opt_[nested_b->pragma_id_].for_split_));
		}
	}

//Wait first for the outer thread (parallel or task with child) so that for sure the inside threads
//have been started.
/*    if(schedopt[nb.pragmaID].barriers.size() > 0 && runningThreads[nb.pragmaID] != NULL){
    	for(int i = 0; i < schedopt[nb.pragmaID].ForSplit; i ++){
    		std::cout << "CALL join thread: " << nb.pragmaID << std::endl;
    		runningThreads[nb.pragmaID][i].join();
    	}
    	runningThreads.erase(nb.pragmaID);
    }
*/
    for(int i = 0; i < (sched_opt_[nested_b->pragma_id_]).barriers_.size(); i ++) {
    	
    	int pragma_id = sched_opt_[nested_b->pragma_id_].barriers_.at(i);
		for(int j = 0; j < sched_opt_[pragma_id].for_split_; j ++){
			running_threads_[pragma_id][j].join();
		}
		running_threads_.erase(pragma_id); 	
    }

}