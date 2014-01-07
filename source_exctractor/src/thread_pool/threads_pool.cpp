/*
 * In case of a parallel pragma is known that each pragma present in the parallel's barrier list has been 
 * invoked by the thread that runs the parallel pragma.
 *
 * In case of a barrier pragma is known that each pragma present in the barrier's barrier list has been invoked
 * by the same thread that invoked the barrieri pragma.
 */


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
    xml_doc.LoadFile("schedule.xml");

    tinyxml2::XMLElement *threads_num_element = xml_doc.FirstChildElement("Schedule")->FirstChildElement("Cores");
    const char* threads_num = threads_num_element->GetText();
    /* Set the number of thread as the number of cores plus one thread wich is used to run parallel and sections job */
    init(chartoint(threads_num) + 1);   


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
        
        if(strcmp(pragma_type, "OMPParallelDirective") == 0
            || strcmp(pragma_type, "OMPSectionsDirective") == 0
            || strcmp(pragma_type, "OMPSingleDirective") == 0) {

            sched_opt.threads_.push_back(chartoint(threads_num));
        }else {
            while(thread_element != NULL){
                const char *thread_id = thread_element->GetText();
                sched_opt.threads_.push_back(chartoint(thread_id));

                thread_element = thread_element->NextSiblingElement("Thread");
            }
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
    int thread_id;
    /* How to get my_id */
    //TODO !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    int my_id = thread_id_to_int_[get_thread_id()];
    for(int i = 0; i < thread_number; i ++) {
        thread_id = sched_opt_[nested_b->pragma_id_].threads_[i];
        if(thread_id != my_id)
            push(nested_b->clone(), ForParameter(i, thread_number), thread_id);
    }

    /* If a son and a father are on the same thread!!! */
    for(int i = 0; i < thread_number; i ++) {
        thread_id = sched_opt_[nested_b->pragma_id_].threads_[i];
        if(thread_id === my_id)
            nested_b->clone()->callme(ForParameter(i, thread_number));
    }

    /* Only pragma Parallel and Parallel For must join in the caller thread */
    if(sched_opt_[nested_b->pragma_id_].pragma_type_.compare("OMPParallelDirective") == 0
        || sched_opt_[nested_b->pragma_id_].pragma_type_.compare("OMPParallelForDirective") == 0) {
        
        /* In case of parallel the first barrerier is always the pragma itself */   
        int barriers_id = sched_opt_[nested_b->pragma_id_].barriers_[0];        
        join(barriers_id, std::this_thread::get_id());
        
        int barriers_number = sched_opt_[nested_b->pragma_id_].barriers_.size();
        for (int i = 1; i < barriers_number; i ++) {
            barriers_id = sched_opt_[nested_b->pragma_id_].barriers_[i];
            int thread_num = sched_opt_[barriers_id].threads_[0];
            std::thread::id t_id = threads_pool_[thread_num].get_id();
            join(barriers_id, t_id);
        }
    }

    if(sched_opt_[nested_b->pragma_id_].pragma_type_.compare("OMPBarrierDirective") == 0) {
        int barriers_number = sched_opt_[nested_b->pragma_id_].barriers_.size();
        int barriers_id, threads_num;
        for (int i = 0; i < barriers_number; i ++) {
            barriers_id = sched_opt_[nested_b->pragma_id_].barriers_[i];
            join(barriers_id, std::this_thread::get_id());
        }
    }
}


void ThreadPool::push(std::shared_ptr<NestedBase> nested_base, 
                                     ForParameter for_param, int thread_id) {
    
    Jobid_t id = nested_base->pragma_id_;
        
    JobIn job_in(nested_base, for_param);
    job_in.job_id_ = id;
    job_in.job_type_ = sched_opt_[nested_base->pragma_id_].pragma_type_;
    job_in.done_cond_var_ = 
            std::unique_ptr<std::condition_variable>(new std::condition_variable());    
       
    job_pop_mtx.lock();

    known_jobs_[std::make_pair(id, std::this_thread::get_id())].push_back(std::move(job_in));
    
    JobQueue j_q(id, for_param.thread_id_, std::this_thread::get_id());
    work_queue_[thread_id].push(j_q);
    
    job_pop_mtx.unlock();

    std::cout << "PUSH " << id << " - " << thread_id << std::endl;

}


void ThreadPool::push_termination_job(int thread_id) {

    JobQueue j_q(0, 0, std::this_thread::get_id());
    work_queue_[thread_id].push(j_q);
}


void ThreadPool::run(int me) {
    //TODO check the correct function !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    thread_id_to_int_[get_thread_id()] = me;
    while(true) {
        
        job_pop_mtx.lock();
        if(work_queue_[me].size() != 0) {

            JobQueue j_q = work_queue_[me].front();
            work_queue_[me].pop();
            job_pop_mtx.unlock();
            
            int pragma_id = j_q.j_id_; int thread_id = j_q.thread_id_;

            if(pragma_id == 0) 
                break;            
            
            JobKey job_key = std::make_pair(pragma_id, j_q.caller_thread_id_);

            ForParameter for_param = known_jobs_[job_key][thread_id].job_.for_param_;
            try {
                known_jobs_[job_key][thread_id].job_.nested_base_->callme(for_param);
            }catch(std::exception& e){
                known_jobs_[job_key][thread_id].terminated_with_exceptions_ = true;
                std::cerr << "Pragma " << pragma_id << " terminated with exception: " << e.what() << std::endl;                
            }

            if(known_jobs_[job_key][thread_id].job_type_.compare("OMPTaskDirective") == 0
                || known_jobs_[job_key][thread_id].job_type_.compare("OMPSingleDirective") == 0
                || known_jobs_[job_key][thread_id].job_type_.compare("OMPSectionsDirective") == 0)
            {
                int barriers_number = sched_opt_[pragma_id].barriers_.size();
                int barrier_id;
                for(int i = 0; i < barriers_number; i ++) {
                    barrier_id = sched_opt_[pragma_id].barriers_[i];
                    join(barrier_id, std::this_thread::get_id());
                }
            }
            known_jobs_[job_key][thread_id].job_completed_ = true;
            known_jobs_[job_key][thread_id].done_cond_var_->notify_one();

        }else {
            job_pop_mtx.unlock();
        }
    }
}


void ThreadPool::join(Jobid_t job_id, std::thread::id caller_thread_id) {    

    for(int i = 0; i < known_jobs_[std::make_pair(job_id, caller_thread_id)].size(); i ++) {
        if(known_jobs_[std::make_pair(job_id, caller_thread_id)][i].job_completed_ != true) {
            std::unique_lock<std::mutex> lk(cond_var_mtx);
            known_jobs_[std::make_pair(job_id, caller_thread_id)][i].done_cond_var_->wait(lk);
        }
    }
    known_jobs_.erase(std::make_pair(job_id, caller_thread_id));
}


void ThreadPool::joinall() {
    /* Push termination job in the working queue */
    for (int i = 0; i < threads_pool_.size(); i ++)
        push_termination_job(i);
    
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