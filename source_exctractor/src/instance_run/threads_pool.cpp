#include "threads_pool.h"


std::mutex singleton_mtx;


ThreadPool* ThreadPool::getInstance(std::string file_name) {
    singleton_mtx.lock();
    static ThreadPool thread_pool(file_name);
    singleton_mtx.unlock();
    return &thread_pool;
}


ThreadPool::ThreadPool(std::string file_name) {
    /* Create schdule xml file name from source code file name, e.g. test.cpp -> test_schedule.xml*/
    std::string in_xml_file (file_name);
    size_t ext = in_xml_file.find_last_of(".");
    if (ext == std::string::npos)
        ext = in_xml_file.length();
    in_xml_file = in_xml_file.substr(0, ext);
    in_xml_file.insert(ext, "_schedule.xml");

    tinyxml2::XMLDocument xml_doc;
    //xml_doc.LoadFile(in_xml_file.c_str());
    xml_doc.LoadFile("test_schedule.xml");

    tinyxml2::XMLElement *threads_num_element = xml_doc.

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

        sched_opt.for_split_ = 1;
        tinyxml2::XMLElement *for_split_element = pragma_element->FirstChildElement("ForSplit");
        if(for_split_element) {
            const char* for_split = for_split_element->GetText();
            sched_opt.for_split_ = chartoint(for_split);
        }

        sched_opt_[id] = sched_opt;

        pragma_element = pragma_element->NextSiblingElement("Pragma");
    }

}


void ThreadPool::init(int pool_size)
{
    threads_pool_.reserve(pool_size);
    for(int i = 0; i < pool_size; i++) {
        threads_pool_.push_back(std::thread(&ThreadPool::run,this, i));
    }
}

void ThreadPool::call(std::shared_ptr<NestedBase> nested_base) {
    int thread_number = sched_opt_[nested_base->pragma_id_].threads_.size();
    for(int i = 0; i < thread_number; i ++) {
        push(nested_base, ForParameter(i, thread_number));
    }

    /* Only pragma Parallel and Parallel For must join in the caller thread */
    if(sched_opt_.pragma_type_.compare("OMPTaskDirective") != 0) {
        int barriers_number = sched_opt_[nested_b->pragma_id_]).barriers_.size();
        int barriers_id;
        for (int i = 0; i < barriers_number; i ++) {
            barriers_id = sched_opt_[nested_b->pragma_id_].barriers_[i];
            join(barriers_id);
            known_jobs_.erase(barriers_id);
        }
    }
}


void ThreadPool::push(std::shared_ptr<NestedBase> nested_base, 
                                     ForParameter for_param) {
    
    jobid_t id = nested_base.pragma_id_;
        
    JobIn job_in;
    job_in.job_ = Job(nested_base, for_param);
    job_in.job_id_ = id;
    job_in.job_type_ = sched_opt_[nested_base->pragma_id_].pragma_type_;
    job_in.done_cond_var_ = 
            std::unique<std::condition_variable>(new std::condition_variable());    
   
    known_jobs_[id].push_back(job_in);
    work_queue_.push(job_in_);
}


void ThreadPool::push_termination_job() {
    JobIn job_int;
    job_in.exit_run_ = true;
    work_queue_.push_back(job_in);
}


void ThreadPool::run(int me) {
    while(true) {
        if(work_queue_.size() != 0) {
            JobIn job_in = work_queue_.front();
            if(job_in.exit_run_)
                break;
            // TODO: exceptions here: terminatedwithexceptions
            //try {
                job_in.job_.nested_base_(job_in.job_.for_param_);
            //}catch(std::exception e)
            
            /* Before task terminate has to wait for all its children to terminate */
            if(job_in.job_type_.compare("OMPTaskDirective") == 0) {
                int barriers_number = sched_opt_[job_in.job_id_].barriers_.size();
                int barrier_id:
                for(int i = 0; i < barriers_number; i ++) {
                    barrier_id = sched_opt_[job_in.job_id_].barriers_[i];
                    join(barrier_id);
                    known_jobs_.erase(barriers_id);
                }
            }

            job_in.done_cond_var_.notify_one();
        }
    }
}


void ThreadPool::join(jobid_t job_id) {
    
    /* Each pragma can be runned on multiple thread: e.g. parallel for */
    for(int i = 0; i < known_jobs_[job_id].size(); i ++) {
        std::unique_lock<std::mutex> lk(cond_var_mtx);
        known_jobs_[job_id].at(i).done_cond_var_.wait(lk);
    }
}


void ThreadPool::joinall() {
    /* Push termination job in the working queue */
    for (int i = 0; i < threads_pool_.size(); i ++)
        push_termination_job();
    
    /* Joining on all the threads in the thread pool */
    for(int i = 0; i < threads_pool_.size(); i++)
        threads_pool_[i].join();

    work_queue_.clear();
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