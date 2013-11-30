
#include "instancerun.h"

std::mutex mtx;


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


	const char* NumThread = doc.FirstChildElement("File")->FirstChildElement("NumThread")->GetText();
	this->NumThread = chartoint(NumThread);
	this->tl = new std::thread[this->NumThread];

	tinyxml2::XMLElement *pragmaelement = doc.FirstChildElement("File")->FirstChildElement("NumThread")->NextSiblingElement("Pragma");
	while(pragmaelement != NULL) {
		ScheduleOptions schedopt;

		const char* pid = pragmaelement->FirstChildElement("ID")->GetText();
		int id = chartoint(pid);
		schedopt.pid = id;

		tinyxml2::XMLElement *threadID = pragmaelement->FirstChildElement("Threads");
		if(threadID != NULL)
			threadID = threadID->FirstChildElement("ThreadID");
		while(threadID != NULL){

			const char * tid = threadID->GetText();
			schedopt.threads.push_back(chartoint(tid));

			threadID = threadID->NextSiblingElement("ThreadID");
		}

		tinyxml2::XMLElement *barriers = pragmaelement->FirstChildElement("Barriers");
		if(barriers != NULL)
			barriers = barriers->FirstChildElement("PragmaID");
		while(barriers != NULL){
			const char *tid = barriers->GetText();
			schedopt.barriers.push_back(chartoint(tid));

			barriers = barriers->NextSiblingElement("PragmaID");
		}
		
		this->schedopt[id] = schedopt;

		pragmaelement = pragmaelement->NextSiblingElement("Pragma");
	}
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