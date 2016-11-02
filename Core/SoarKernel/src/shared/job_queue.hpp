//
//  job_queue_hpp.cpp
//  Soar-xcode
//
//  Created by Alex Turner on 9/25/16.
//  Copyright © 2016 University of Michigan – Soar Group. All rights reserved.
//

#ifndef job_queue_hpp
#define job_queue_hpp

#include <functional>
#include <atomic>
#include <vector>
#include <memory>
#include <mutex>
#include <thread>
#include <list>

class job_queue
{
public:
    class job;
private:
    std::atomic<bool> killThreads;

    std::mutex mutex;

    std::vector<std::thread*> threads;

    std::list<std::shared_ptr<job>> jobQueue;

public:
    class job
    {
    private:
        friend class job_queue;

        job();
        job(std::function<void()> e, std::function<void()> c);
    public:
        std::function<void()> execution;
        std::function<void()> completionCallback;

        std::atomic<bool> complete;

        job& operator=(const job& b);

        void wait();

        std::exception_ptr exception;
    };
    
    job_queue(std::function<void ()> threadInitializer = [](){});
    ~job_queue();

    std::shared_ptr<job> post(std::function<void()> e, std::function<void()> c = []{});
};

#endif /* job_queue_hpp */
