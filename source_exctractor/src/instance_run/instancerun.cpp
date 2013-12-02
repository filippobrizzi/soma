
#include "instancerun.h"

std::mutex mtx;

void NestedBase::operator()(ForParameter *fp) {
		std::cout << "operator(): " << pragmaID << std::endl;
		this->fp = fp;
  	/*	std::chrono::time_point<std::chrono::system_clock> start = InstanceRun::getInstance("")->getTimeStart();
  		std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();
  		int elapsed_milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(now-start).count();
  		if(ActivationTime - elapsed_milliseconds > 0) {
  			std::chrono::milliseconds dura(ActivationTime - elapsed_milliseconds);
  			std::this_thread::sleep_for(dura);
  		}*/
    	callme();
}

InstanceRun* InstanceRun::getInstance(std::string filename) {
	mtx.lock();
	static InstanceRun run(filename);
	mtx.unlock();
	return &run;
}


InstanceRun::InstanceRun(std::string filename) {
	std::string inXML (filename);
  	size_t ext = inXML.find_last_of(".");
  	if (ext == std::string::npos)
    	ext = inXML.length();
  	inXML = inXML.substr(0, ext);
  	inXML.insert(ext, "_schedule.xml");

  	tinyxml2::XMLDocument doc;
 	//doc.LoadFile(inXML.c_str());
 	doc.LoadFile("test_schedule.xml");

	tinyxml2::XMLElement *pragmaelement = doc.FirstChildElement("File")->FirstChildElement("Pragma");
	while(pragmaelement != NULL) {
		ScheduleOptions schedopt;

		const char* pid = pragmaelement->FirstChildElement("ID")->GetText();
		int id = chartoint(pid);
		schedopt.pid = id;

		schedopt.ActivationTime = 0;
		tinyxml2::XMLElement *activationtime = pragmaelement->FirstChildElement("ActivationTime");
		if(activationtime) {
			const char* activationTime = activationtime->GetText();
			schedopt.ActivationTime = chartoint(activationTime);
		}

		tinyxml2::XMLElement *barriers = pragmaelement->FirstChildElement("Barriers");
		if(barriers != NULL)
			barriers = barriers->FirstChildElement("PragmaID");
		while(barriers != NULL){
			const char *tid = barriers->GetText();
			schedopt.barriers.push_back(chartoint(tid));

			barriers = barriers->NextSiblingElement("PragmaID");
		}

		schedopt.ForSplit = 1;
		tinyxml2::XMLElement *forsplit = pragmaelement->FirstChildElement("ForSplit");
		if(forsplit) {
			const char* cforsplit = forsplit->GetText();
			schedopt.ForSplit = chartoint(cforsplit);
		}

		this->schedopt[id] = schedopt;

		pragmaelement = pragmaelement->NextSiblingElement("Pragma");
	}

	this->start = std::chrono::system_clock::now();
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


void InstanceRun::call(NestedBase & nb) {
		
	runningThreads[nb.pragmaID] = new std::thread[schedopt[nb.pragmaID].ForSplit];
	nb.ActivationTime = schedopt[nb.pragmaID].ActivationTime;

	if(schedopt[nb.pragmaID].ActivationTime > 0) {
		for(int i = 0; i < schedopt[nb.pragmaID].ForSplit; i ++) {
			ForParameter *fp = new ForParameter(i, schedopt[nb.pragmaID].ForSplit);
			std::cout << "CALL: " << nb.pragmaID << " with tid: " << i << " forsplit: " << schedopt[nb.pragmaID].ForSplit << std::endl;
			runningThreads[nb.pragmaID][i] = std::thread(std::ref(nb), fp);
		}
	}

    int t;
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
    for(int i = 0; i < (schedopt[nb.pragmaID]).barriers.size(); i ++) {			
    //	if(schedopt[nb.pragmaID].barriers.at(i) != nb.pragmaID){
			for(int j = 0; j < schedopt[schedopt[nb.pragmaID].barriers.at(i)].ForSplit; j ++){
				std::cout << "CALL join thread: " << schedopt[nb.pragmaID].barriers.at(i) << " " << j << std::endl;
				runningThreads[schedopt[nb.pragmaID].barriers.at(i)][j].join();
			}
			runningThreads.erase(schedopt[nb.pragmaID].barriers.at(i)); 	
    //	}
    }

}