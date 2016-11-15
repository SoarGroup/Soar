//
//  job_queue.cpp
//  Soar-xcode
//
//  Created by Alex Turner on 10/3/16.
//  Copyright © 2016 University of Michigan – Soar Group. All rights reserved.
//

#include "job_queue.hpp"

#include <iostream>
#include <functional>
#include <type_traits>
#include <utility>
#include <thread>
#include <future>

job_queue::job::job() {}

job_queue::job::job(std::function<void()> e, std::function<void()> c)
: execution(e),
completionCallback(c),
complete(false),
exception(nullptr)
{}

void job_queue::job::wait()
{
    while (!this->complete)
    {
        std::this_thread::yield();
    }

    if (exception != nullptr)
        std::rethrow_exception(exception);
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
            std::function<void()> task;

            {
                std::unique_lock<std::mutex> lock(mutex);
                if (jobQueue.empty())
                {
                    condition.wait(lock);
                    continue;
                }

                task = std::move(jobQueue.front());
                jobQueue.pop_front();
            }

            task();
        }
    };

    for (unsigned i = 0;i < concurentThreadsSupported;++i)
        threads.emplace_back(jobProcessor);
}

job_queue::~job_queue()
{
    killThreads = true;
    condition.notify_all();

    for (auto& t : threads)
        t.join();

    threads.clear();
}
