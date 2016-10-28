//
//  job_queue.cpp
//  Soar-xcode
//
//  Created by Alex Turner on 10/3/16.
//  Copyright © 2016 University of Michigan – Soar Group. All rights reserved.
//

#include "job_queue.hpp"

job_queue::job::job() {}

job_queue::job::job(std::function<void()> e, std::function<void()> c)
: execution(e),
completionCallback(c),
complete(false)
{}

void job_queue::job::wait()
{
    while (!this->complete)
    {
        std::this_thread::yield();
    }
}

job_queue::job& job_queue::job::operator=(const job& b)
{
    this->execution = b.execution;
    this->complete = b.complete.load();

    return *this;
}

job_queue::job_queue(std::function<void ()> threadInitializer)
: killThreads(false)
{
    unsigned concurentThreadsSupported = std::thread::hardware_concurrency();

    if (concurentThreadsSupported == 0)
        concurentThreadsSupported = 1;

    threadInitializer();

    std::function<void()> jobProcessor = [this,threadInitializer]()
    {
        threadInitializer();

        while (!killThreads)
        {
            std::shared_ptr<job> job;

            try {
                std::lock_guard<std::mutex> lock(mutex);

                if (jobQueue.size() > 0)
                {
                    job = jobQueue.front();
                    jobQueue.pop_front();
                }
            } catch(...) {}

            if (job == nullptr)
            {
                std::this_thread::yield();
                continue;
            }

            job->execution();
            job->complete = true;
            job->completionCallback();
        }
    };

    for (unsigned i = 0;i < concurentThreadsSupported;++i)
        threads.push_back(new std::thread(jobProcessor));
}

job_queue::~job_queue()
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

std::shared_ptr<job_queue::job> job_queue::post(std::function<void()> e, std::function<void()> c)
{
    std::lock_guard<std::mutex> lock(mutex);

    std::shared_ptr<job> job(new class job(e, c));
    jobQueue.push_back(job);
    
    return job;
}
