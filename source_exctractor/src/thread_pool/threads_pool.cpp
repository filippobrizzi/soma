#include "threads_pool.h"


std::mutex singleton_mtx;
std::mutex cout_mtx;


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
    xml_doc.LoadFile("schedule.xml");

    tinyxml2::XMLElement *threads_num_element = xml_doc.FirstChildElement("Schedule")->FirstChildElement("Cores");
    const char* threads_num = threads_num_element->GetText();
    init(chartoint(threads_num));   


    tinyxml2::XMLElement *pragma_element = xml_doc.FirstChildElement("Schedule")->FirstChildElement("Pragma");
    while(pragma_element != NULL) {
        ScheduleOptions sched_opt;

        const char* pragma_id = pragma_element->FirstChildElement("id")->GetText();
        int id = chartoint(pragma_id);
        sched_opt.pragma_id_ = id;

        tinyxml2::XMLElement *pragma_type_element = pragma_element->FirstChildElement("Type");
        const char* pragma_type = pragma_type_element->GetText();    
        sched_opt.pragma_type_ = pragma_type;

        tinyxml2::XMLElement *thread_element = pragma_element->FirstChildElement("Threads");
        if(thread_element != NULL)
            thread_element = thread_element->FirstChildElement("Thread");
        while(thread_element != NULL){
            const char *thread_id = thread_element->GetText();
            sched_opt.threads_.push_back(chartoint(thread_id));

            thread_element = thread_element->NextSiblingElement("Thread");
        }

        tinyxml2::XMLElement *barriers_element = pragma_element->FirstChildElement("Barrier");
        if(barriers_element != NULL)
            barriers_element = barriers_element->FirstChildElement("id");
        while(barriers_element != NULL){
            const char *thread_id = barriers_element->GetText();
            sched_opt.barriers_.push_back(chartoint(thread_id));

            barriers_element = barriers_element->NextSiblingElement("id");
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

void ThreadPool::call(std::shared_ptr<NestedBase> nested_b) {
    int thread_number = sched_opt_[nested_b->pragma_id_].threads_.size();


    for(int i = 0; i < thread_number; i ++) {
        std::shared_ptr<NestedBase> new_nested_b = nested_b->clone();
        new_nested_b->iid = i;
        push(new_nested_b, ForParameter(i, thread_number));
    }

    /* Only pragma Parallel and Parallel For must join in the caller thread */
    if(sched_opt_[nested_b->pragma_id_].pragma_type_.compare("OMPTaskDirective") != 0) {
        int barriers_number = sched_opt_[nested_b->pragma_id_].barriers_.size();

        int barriers_id;
        for (int i = 0; i < barriers_number; i ++) {
            barriers_id = sched_opt_[nested_b->pragma_id_].barriers_[i];
            join(barriers_id);
        }
    }
}


void ThreadPool::push(std::shared_ptr<NestedBase> nested_base, 
                                     ForParameter for_param) {
    
    Jobid_t id = nested_base->pragma_id_;
        
    JobIn job_in(nested_base, for_param);
    job_in.job_id_ = id;
    job_in.job_type_ = sched_opt_[nested_base->pragma_id_].pragma_type_;
    job_in.done_cond_var_ = 
            std::unique_ptr<std::condition_variable>(new std::condition_variable());    
   
    known_jobs_[id].push_back(std::move(job_in));
    
    Jobid_t *jid = new Jobid_t[2];
    jid[0] = id; jid[1] = for_param.thread_id_;
    
    job_pop_mtx.lock();
    work_queue_.push(jid);
    job_pop_mtx.unlock();
    /* DEBUG */

    //cout_mtx.lock();std::cout << "Push " << nested_base->pragma_id_ << " - " << for_param.thread_id_ << " / " << for_param.num_threads_ << std::endl;cout_mtx.unlock();
    cout_mtx.lock();std::cout << "Push " << nested_base->pragma_id_ << " - " 
        << for_param.thread_id_ << " / " << for_param.num_threads_  << " -- "
        << known_jobs_[id].back().job_id_ << " - "
        << known_jobs_[id].back().job_.for_param_.thread_id_ << " / "
        << known_jobs_[id].back().job_.for_param_.num_threads_ << std::endl;cout_mtx.unlock();
}


void ThreadPool::push_termination_job() {

    Jobid_t *jid = new Jobid_t[2];
    jid[0] = 0; jid[1] = 0;
    work_queue_.push(jid);
}


void ThreadPool::run(int me) {
    while(true) {
        
        job_pop_mtx.lock();
        if(work_queue_.size() != 0) {

            Jobid_t *j_id = work_queue_.front();
            work_queue_.pop();
            job_pop_mtx.unlock();
            
            if(j_id[0] == 0)
                break;            
            
            JobIn &job_in = known_jobs_[j_id[0]].at(j_id[1]);
            cout_mtx.lock(); std::cout << "THREAD: " << me << " job_in " << j_id[0] << " - " << j_id[1] 
                << " -- " << known_jobs_[j_id[0]][j_id[1]].job_id_ 
                << " JOB FOR " << job_in.job_.for_param_.thread_id_ << " / " << job_in.job_.for_param_.num_threads_ 
                << " nested " << job_in.job_.nested_base_->pragma_id_
                << " IIDDD " << job_in.job_.nested_base_->iid
                << std::endl; cout_mtx.unlock();
            // TODO: exceptions here: terminatedwithexceptions
            //try {
                job_in.job_.nested_base_->callme(job_in.job_.for_param_);
            //}catch(std::exception e) {}
            
            /* Before task terminate has to wait for all its children to terminate */
            if(job_in.job_type_.compare("OMPTaskDirective") == 0) {
                int barriers_number = sched_opt_[job_in.job_id_].barriers_.size();
                int barrier_id;
                for(int i = 0; i < barriers_number; i ++) {
                    barrier_id = sched_opt_[job_in.job_id_].barriers_[i];
                    join(barrier_id);                
                }
            }
            job_in.job_completed_ = true;
            job_in.done_cond_var_->notify_one();
        }else {
            //std::cout << "AAAAAAAAAAAAAAAA   " << me << std::endl;
            job_pop_mtx.unlock();
        }
    }
}


void ThreadPool::join(Jobid_t job_id) {
    
    //cout_mtx.lock(); std::cout << "JOIN START " << job_id << std::endl; cout_mtx.unlock();
    /* Each pragma can be runned on multiple thread: e.g. parallel for */
    for(int i = 0; i < known_jobs_[job_id].size(); i ++) {
        if(known_jobs_[job_id].at(i).job_completed_ != true) {
            std::unique_lock<std::mutex> lk(cond_var_mtx);
            known_jobs_[job_id].at(i).done_cond_var_->wait(lk);
        }
    }
    //CHECK THIS
    //known_jobs_.erase(job_id);
    //cout_mtx.lock(); std::cout << "JOIN END " << job_id << std::endl; cout_mtx.unlock();
}


void ThreadPool::joinall() {
    /* Push termination job in the working queue */
    for (int i = 0; i < threads_pool_.size(); i ++)
        push_termination_job();
    
    /* Joining on all the threads in the thread pool */
    for(int i = 0; i < threads_pool_.size(); i++)
        threads_pool_[i].join();

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