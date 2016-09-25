//
//  smem_job_queue.hpp
//  Soar-xcode
//
//  Created by Alex Turner on 9/25/16.
//  Copyright © 2016 University of Michigan – Soar Group. All rights reserved.
//

#ifndef smem_job_queue_hpp
#define smem_job_queue_hpp

#include <functional>
#include <atomic>
#include <vector>
#include <memory>
#include <mutex>
#include <thread>
#include <list>

class SMem_JobQueue
{
public:
    class SMem_Job;
private:
    std::atomic<bool> killThreads;

    std::mutex mutex;

    std::vector<std::thread*> threads;

    std::list<std::shared_ptr<SMem_Job>> jobQueue;

public:
    class SMem_Job
    {
    private:
        friend class SMem_JobQueue;

        SMem_Job() {}

        SMem_Job(std::function<void(void*)> e, void* a, std::function<void()> c)
        : execution(e),
        completionCallback(c)
        {
            argument = a;
        }

    public:
        std::function<void(void*)> execution;
        std::function<void()> completionCallback;

        void* argument;

        std::atomic<bool> complete;

        SMem_Job& operator=(const SMem_Job& b)
        {
            this->execution = b.execution;
            this->argument = b.argument;
            this->complete = b.complete.load();

            return *this;
        }
    };

    SMem_JobQueue()
    : killThreads(false)
    {
        unsigned concurentThreadsSupported = std::thread::hardware_concurrency();

        if (concurentThreadsSupported == 0)
            concurentThreadsSupported = 1;

        std::function<void()> jobProcessor = [this]()
        {
            while (!killThreads)
            {
                std::shared_ptr<SMem_Job> job;

                try {
                    std::lock_guard<std::mutex> lock(mutex);

                    if (jobQueue.size() > 0)
                    {
                        job = jobQueue.front();
                        jobQueue.pop_front();
                    }
                } catch(...) {}

                job->execution(job->argument);
                job->complete = true;
                job->completionCallback();
            }
        };

        threads.push_back(new std::thread(jobProcessor));
    }

    ~SMem_JobQueue()
    {
        killThreads = true;

        bool killed = false;
        while (!killed)
        {
            std::this_thread::yield();

            killed = true;
            for (auto& threadPtr : threads)
            {
                if (!threadPtr->joinable())
                {
                    killed = false;
                    break;
                }
            }
        }

        for (auto& threadPtr : threads)
        {
            threadPtr->join();
        }
    }

    std::shared_ptr<SMem_Job> add(std::function<void()> e, std::function<void()> c = []{})
    {
        return add([e](void*){ e(); }, nullptr, c);
    }

    std::shared_ptr<SMem_Job> add(std::function<void(void*)> e, void* argument = nullptr, std::function<void()> c = []{})
    {
        std::lock_guard<std::mutex> lock(mutex);

        std::shared_ptr<SMem_Job> job(new SMem_Job(e, argument, c));
        jobQueue.push_back(job);

        return job;
    }
};

#endif /* smem_job_queue_hpp */
