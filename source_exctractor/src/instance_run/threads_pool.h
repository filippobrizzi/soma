

class ThreadPool {
public:
    typedef int jobid_t;

    struct Job
    {
        std::shared_ptr<NestedBase> nested_base_;
        ForParameter for_param_;
        Job(std::shared_ptr<NestedBase> nested_base, ForParameter for_param)
            : nested_base_(nested_base), for_param_(for_param) {}
    };


    void init(int pool_size);

    jobid_t push(Job job);

    bool join(jobid_t);

    void joinall();

    void call(std::shared_ptr<NestedBase> nested_base);

    ~ThreadPool() { joinall(); }


private:
    struct ScheduleOptions {
        int pragma_id_;
        /* Indicates the threads that have to run the task */
        std::vector<int> threads_
        /* List of pragma_id_ to wait before completing the task */
        std::vector<int> barriers_;
    };

    struct JobIn {
        Job job_;
        jobid_t job_id_;
        bool terminated_with_exceptions_;
        std::unique_ptr<std::condition_variable> done_cond_var_;
    };

    void run(int id);

    std::map<int, std::Queue<JobIn>> work_queue_; //NB thread safe
    
    std::map<int, ScheduleOptions> sched_opt_;
    
    std::vector<std::thread> threads_pool_; // not thread safe
    std::map<jobid_t, JobIn> known_jobs_; // thread safe
    
    std::mutex mtx; // common
};

void ThreadPool::push(std::shared_ptr<NestedBase> nested_base, ForParameter for_param, int thread_id) {
    jobid_t id = nested_base.pragma_id_;
        
    JobIn job_in;

    job_in.job_ = Job(nested_base, for_param);
    job_in.job_id_ = id;
    job_in.done_cond_var_ = std::unique<std::condition_variable>(new std::condition_variable());
    known_jobs_[id] = job_in;
    work_queue_[id].push_back(job_in_);
    
    return id;
}

void ThreadPool::call(std::shared_ptr<NestedBase> nested_base) {
    int thread_number = sched_opt_[nested_base->pragma_id_].threads_.size();
    int thread_id;
    for(int i = 0; i < thread_number; i ++) {
        thread_id = sched_opt_[nested_base->pragma_id_].threads_.at(i);
        push(nested_base, ForParameter(i, thread_number), thread_id);
    }

    int barriers_number = sched_opt_[nested_b->pragma_id_]).barriers_.size();
    int barriers_id = sched_opt_[nested_b->pragma_id_].barriers_.at(i);
    for (int i = 0; i < barriers_number; i ++) {
        join(barriers_id);
        known_jobs_.erase(barriers_id);
    }

}


void ThreadPool::init(int pool_size)
{
    threads_pool_.reserve(pool_size);
    for(int i = 0; i < pool_size; i++) {
        threads_pool_.push_back(std::thread(&ThreadPool::run,this, i));
    }
}

void ThreadPool::run(int me) {
        while(true) {
            JobIn job_in = work_queue_[me].front();
                    // TODO: exceptions here: terminatedwithexceptions
            try {
                job_in.job_.nested_base_(job_in.job_.for_param_);
            } catch(std::exception e)
            
            job_in.done_cond_var_.notify_one();
        }
}

void ThreadPool::join(jobid_t job_id) {
    
    std::unique_lock<std::mutex> lk(mtx);
    known_jobs_[job_id].done_cond_var_.wait(lk);
}

void ThreadPool::joinall() {
        work_queue_.clear();
        for(int i = 0; i < threads_pool_.size(); i++)
                threads_pool_[i].join();
}