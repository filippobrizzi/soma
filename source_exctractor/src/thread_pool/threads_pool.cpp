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
        //for(std::map<int, ScheduleOptions>::iterator itr = sched_opt_.begin(); itr != sched_opt_.end(); ++ itr)
        //std::cout << "Pragma id: " << (*itr).second.pragma_id_ << ", type: " << (*itr).second.pragma_type_ << std::endl;

}


void ThreadPool::init(int pool_size)
{
    /* This is needed cause otherwise the main process would be considered as thread num 0*/
    thread_id_to_int_[std::this_thread::get_id()] = -1;
    //std::cout << std::this_thread::get_id() << " = -1 " << std::endl;

    threads_pool_.reserve(pool_size);
    for(int i = 0; i < pool_size; i++) {
        threads_pool_.push_back(std::thread(&ThreadPool::run,this, i));
    }

}

/* If a job has to allocate a job on its own thread, it first allocates all other job and then execute directly that job */
/* This solve the problem of a parallel for. */
bool ThreadPool::call(std::shared_ptr<NestedBase> nested_b) {
    int thread_number = sched_opt_[nested_b->pragma_id_].threads_.size();
    int thread_id;
    /* Get the integer id of the running thread */
    int my_id = thread_id_to_int_[std::this_thread::get_id()];
    /* In case of a parallel for */

    if(sched_opt_[nested_b->pragma_id_].pragma_type_.compare("OMPForDirective") == 0
        || sched_opt_[nested_b->pragma_id_].pragma_type_.compare("OMPParallelForDirective") == 0) {
        
        call_for(nested_b);

    }else {
        thread_id = sched_opt_[nested_b->pragma_id_].threads_[0];
        if(thread_id != my_id) {
            push(nested_b->clone(), ForParameter(0, 1), thread_id);
            
            if(sched_opt_[nested_b->pragma_id_].pragma_type_.compare("OMPParallelDirective") == 0) {
                int barriers_id = sched_opt_[nested_b->pragma_id_].barriers_[0];

                join(barriers_id);
                
                int thread_num = sched_opt_[nested_b->pragma_id_].threads_[0];
                std::thread::id t_id = threads_pool_[thread_num].get_id();
                int barriers_number = sched_opt_[nested_b->pragma_id_].barriers_.size();
                
                for (int i = 1; i < barriers_number; i ++) {
                    barriers_id = sched_opt_[nested_b->pragma_id_].barriers_[i];
                    join(barriers_id);
                }
            }
        }else {
            if(sched_opt_[nested_b->pragma_id_].pragma_type_.compare("OMPParallelDirective") == 0)
                call_parallel(nested_b);
            else if (sched_opt_[nested_b->pragma_id_].pragma_type_.compare("OMPSectionsDirective") == 0
                || sched_opt_[nested_b->pragma_id_].pragma_type_.compare("OMPSingleDirective") == 0)
                call_sections(nested_b);
            else if(sched_opt_[nested_b->pragma_id_].pragma_type_.compare("OMPBarrierDirective") == 0)
                call_barrier(nested_b);
            else {
                push_completed_job(nested_b, ForParameter(0, 1));
                return true;
            }
        }

    }
    return false;
}

void ThreadPool::call_sections(std::shared_ptr<NestedBase> nested_b){
    nested_b->callme(ForParameter(0, 1));
    
    int barriers_number = sched_opt_[nested_b->pragma_id_].barriers_.size();
    int barrier_id;
    for(int i = 0; i < barriers_number; i ++) {
        barrier_id = sched_opt_[nested_b->pragma_id_].barriers_[i];
        join(barrier_id);
    }
    push_completed_job(nested_b, ForParameter(0, 1));
}

void ThreadPool::call_parallel(std::shared_ptr<NestedBase> nested_b) {
    nested_b->callme(ForParameter(0, 1));

    if(sched_opt_[nested_b->pragma_id_].pragma_type_.compare("OMPParallelDirective") == 0) {
        int barriers_id = sched_opt_[nested_b->pragma_id_].barriers_[0];
        join(barriers_id);
            
        int barriers_number = sched_opt_[nested_b->pragma_id_].barriers_.size();
        for (int i = 1; i < barriers_number; i ++) {
            barriers_id = sched_opt_[nested_b->pragma_id_].barriers_[i];
            int thread_num = sched_opt_[barriers_id].threads_[0];
            std::thread::id t_id = threads_pool_[thread_num].get_id();
            join(barriers_id);
        }
    }
}

void ThreadPool::call_for(std::shared_ptr<NestedBase> nested_b) {
    int thread_number = sched_opt_[nested_b->pragma_id_].threads_.size();
    int thread_id;
    /* Get the integer id of the running thread */
    int my_id = thread_id_to_int_[std::this_thread::get_id()];
    for(int i = 0; i < thread_number; i ++) {
        thread_id = sched_opt_[nested_b->pragma_id_].threads_[i];
        if(thread_id != my_id) {
            push(nested_b->clone(), ForParameter(i, thread_number), thread_id);
        }
    }
    /* If a son and a father are on the same thread!!! */
    for(int i = 0; i < thread_number; i ++) {
        thread_id = sched_opt_[nested_b->pragma_id_].threads_[i];
        if(thread_id == my_id) {
            push_completed_job(nested_b->clone(), ForParameter(i, thread_number));
            nested_b->callme(ForParameter(i, thread_number));
        }
    }

    //if(sched_opt_[nested_b->pragma_id_].barriers_.size() > 0) {
        int barriers_id = sched_opt_[nested_b->pragma_id_].barriers_[0];
        join(barriers_id);
    //}
}

void ThreadPool::call_barrier(std::shared_ptr<NestedBase> nested_b) {
    int barriers_number = sched_opt_[nested_b->pragma_id_].barriers_.size();
    int barriers_id, threads_num;
    for (int i = 0; i < barriers_number; i ++) {
        barriers_id = sched_opt_[nested_b->pragma_id_].barriers_[i];
        join(barriers_id);
    }
}

/* Insert a job wich has the flag completed already setted. This is necessary in case a thread executes more
job consecutively */
void ThreadPool::push_completed_job(std::shared_ptr<NestedBase> nested_base,
                                     ForParameter for_param) {

    Jobid_t id = nested_base->pragma_id_;
        
    JobIn job_in(nested_base, for_param);
    job_in.job_id_ = id;
    job_in.job_completed_ = true;

    job_pop_mtx.lock();
    if(known_jobs_[id].size() == 0)
        known_jobs_[id].reserve(for_param.num_threads_);
    known_jobs_[id].push_back(std::move(job_in));
    job_pop_mtx.unlock();
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
    if(known_jobs_[id].size() == 0)
        known_jobs_[id].reserve(for_param.num_threads_);
    known_jobs_[id].push_back(std::move(job_in));

    JobQueue j_q(id, for_param.thread_id_);
    work_queue_[thread_id].push(j_q);
    
    job_pop_mtx.unlock();

}


void ThreadPool::push_termination_job(int thread_id) {

    JobQueue j_q(-1, 0);
    work_queue_[thread_id].push(j_q);
}


void ThreadPool::run(int me) {
    thread_id_to_int_[std::this_thread::get_id()] = me;
    while(true) {
        
        job_pop_mtx.lock();
        if(work_queue_[me].size() != 0) {

            JobQueue j_q = work_queue_[me].front();
            work_queue_[me].pop();
            job_pop_mtx.unlock();
            
            int pragma_id = j_q.j_id_;
            int thread_id = j_q.thread_id_;

            if(pragma_id != 0) {
                if(pragma_id == -1)
                    break;
                
                job_pop_mtx.lock();
                std::vector<JobIn>::iterator j_itr;
                for(j_itr = known_jobs_[pragma_id].begin(); j_itr != known_jobs_[pragma_id].end(); ++ j_itr) {
                    if(j_itr->job_.for_param_.thread_id_ == thread_id)
                        break;
                }
               
                job_pop_mtx.unlock();
                ForParameter for_param = j_itr->job_.for_param_;
                
                try {
                    j_itr->job_.nested_base_->callme(for_param);
                }catch(std::exception& e){
                    //known_jobs_[pragma_id][thread_id].terminated_with_exceptions_ = true;
                    std::cerr << "Pragma " << pragma_id << " terminated with exception: " << e.what() << std::endl;
                }


                if(j_itr->job_type_.compare("OMPTaskDirective") == 0
                    || j_itr->job_type_.compare("OMPSingleDirective") == 0
                    || j_itr->job_type_.compare("OMPSectionsDirective") == 0)
                {
                    int barriers_number = sched_opt_[pragma_id].barriers_.size();
                    int barrier_id;
                    for(int i = 0; i < barriers_number; i ++) {
                        barrier_id = sched_opt_[pragma_id].barriers_[i];
                        join(barrier_id);
                    }
                }

                job_end.lock();
                j_itr->job_completed_ = true;
                j_itr->done_cond_var_->notify_one();
                job_end.unlock();
            }
        }else {
            job_pop_mtx.unlock();
        }
    }
}


void ThreadPool::join(Jobid_t job_id) {

    /*for(int i = 0; i < known_jobs_[job_id].size(); i ++) {
        if(known_jobs_[job_id][i].job_completed_ != true) {
            std::unique_lock<std::mutex> lk(cond_var_mtx);
            known_jobs_[job_id][i].done_cond_var_->wait(lk);
        }
    }*/
    //std::mutex cond_var_mtx;
    
    std::vector<JobIn>::iterator j_itr;
    for(j_itr = known_jobs_[job_id].begin(); j_itr != known_jobs_[job_id].end(); ++ j_itr) {
        job_end.lock();
        if((*j_itr).job_completed_ != true){
            job_end.unlock();
            std::unique_lock<std::mutex> lk(cond_var_mtx[std::this_thread::get_id()]);
            j_itr->done_cond_var_->wait(lk);            
        }else{
            job_end.unlock();
        }
    }
    job_pop_mtx.lock();
    known_jobs_.erase(job_id);
    job_pop_mtx.unlock();
}


void ThreadPool::joinall() {
    /* Push termination job in the working queue */
    std::cout << "Joinall" << std::endl;
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

int timeval_subtract(timeval *result, timeval *x, timeval *y) {
  /* Perform the carry for the later subtraction by updating y. */
  if (x->tv_usec < y->tv_usec) {
    int nsec = (y->tv_usec - x->tv_usec) / 1000000 + 1;
    y->tv_usec -= 1000000 * nsec;
    y->tv_sec += nsec;
  }
  if (x->tv_usec - y->tv_usec > 1000000) {
    int nsec = (y->tv_usec - x->tv_usec) / 1000000;
    y->tv_usec += 1000000 * nsec;
    y->tv_sec -= nsec;
  }
}