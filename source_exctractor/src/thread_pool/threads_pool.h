#include <string>
#include <thread>
#include <vector>
#include <mutex> 
#include <map>
#include <math.h>
#include <iostream>
#include <condition_variable>
#include <queue>
#include <memory>
#include <exception>

#include "xml_creator/tinyxml2.h"

int chartoint(const char *cc);
int chartoint(char *cc);


class ForParameter {
public:
    const int thread_id_;
    const int num_threads_;
    ForParameter(int thread_id, int num_threads) : thread_id_(thread_id), num_threads_(num_threads) {}
};


class NestedBase { 
public: 

    NestedBase(int pragma_id) : pragma_id_(pragma_id) {}
    
    int pragma_id_;
    std::queue<std::shared_ptr<NestedBase>> todo_job_;

    void launch_todo_job() {
        while(todo_job_.size() != 0) {
            todo_job_.front()->callme(ForParameter(0, 1));
            todo_job_.pop();
        }
    }
    
    virtual void callme(ForParameter for_param) = 0;
    virtual std::shared_ptr<NestedBase> clone() const = 0;
};


class ThreadPool {
public:
    typedef int Jobid_t;

    struct Job
    {
        std::shared_ptr<NestedBase> nested_base_;
        ForParameter for_param_;
        Job(std::shared_ptr<NestedBase> nested_base, ForParameter for_param)
            : nested_base_(nested_base), for_param_(for_param) {}
    };

    /* Launches the threads */
    void init(int pool_size);

    /* Called by the task to be put in the job queue */
    bool call(std::shared_ptr<NestedBase> nested_base);
    void call_sections(std::shared_ptr<NestedBase> nested_b);
    void call_parallel(std::shared_ptr<NestedBase> nested_b);
    void call_for(std::shared_ptr<NestedBase> nested_b);
    void call_barrier(std::shared_ptr<NestedBase> nested_b);

    /* Push a job in the job queue */
    void push(std::shared_ptr<NestedBase> nested_base, ForParameter for_param, int thread_id);
    void push_completed_job(std::shared_ptr<NestedBase> nested_base, ForParameter for_param);
    void push_termination_job(int thread_id);

    /* Pause a thread till the job[job_id] complete */ 
    void join(Jobid_t job_id, std::thread::id caller_thread_id);

    void joinall();

    static ThreadPool* getInstance(std::string file_name);

    /* Map the thread::id to an integer going from 0 to num_thread - 1 */
    std::map<std::thread::thread::id, int> thread_id_to_int_;

    ~ThreadPool() { joinall(); }

private:
    struct ScheduleOptions {
        int pragma_id_;
        int caller_id_;
        /* In case of a parallel for, specify to the job which part of the for to execute */
        int thread_id_;
        /* Idicates the pragma type: parallel, task, ... */
        std::string pragma_type_;
        /* Indicates the threads that have to run the task */
        std::vector<int> threads_;
        /* List of pragma_id_ to wait before completing the task */
        std::vector<int> barriers_;
    };

    struct JobIn {
        Job job_;
        /* ID of the job = pragma line number */
        Jobid_t job_id_;
        Jobid_t pragma_id_;
        /* Pragma type, e.g. OMPParallelDirective, OMPTaskDirective, ... */
        std::string job_type_;
        /* Fix the bug where a thread waits for another thread which already nofied to have compleated */ 
        bool job_completed_ = false;

        bool terminated_with_exceptions_ = false;

        std::unique_ptr<std::condition_variable> done_cond_var_;

        std::vector<int> barriers_;

        JobIn(std::shared_ptr<NestedBase> nested_base, ForParameter for_param) 
                : job_(nested_base, for_param), job_completed_(false) {}

    };

    struct JobQueue {
        Jobid_t j_id_;
        int thread_id_;
        std::thread::id caller_thread_id_;
        JobQueue(Jobid_t j_id, int thread_id, std::thread::id caller_thread_id) 
            : j_id_(j_id), thread_id_(thread_id), caller_thread_id_(caller_thread_id) {}
    };

    ThreadPool(std::string file_name);
    
    void run(int id);

    std::map<int, ScheduleOptions> sched_opt_;

    std::vector<std::thread> threads_pool_; // not thread safe
    
    /* Job queue for each thread */
    std::map<int, std::queue<JobQueue>> work_queue_;
    
    /* For each pragma the list of jobs executing that pragma, e.g. in case of parallel for */
    typedef std::pair<Jobid_t, std::thread::id> JobKey;
    std::map<JobKey, std::vector<JobIn>> known_jobs_;

    /* Mutex used by std::condition_variable to synchronize jobs execution */
    std::mutex cond_var_mtx; 
    std::mutex job_pop_mtx;
};

