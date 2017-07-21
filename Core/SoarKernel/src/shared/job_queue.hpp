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
#include <deque>
#include <future>

class job_queue
{
public:
    class job;
private:
    std::atomic<bool> killThreads;
    std::vector<std::thread> threads;


    std::mutex mutex;
    std::condition_variable condition;

    std::deque<std::function<void()>> jobQueue;

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
    
    explicit job_queue(std::function<void ()> threadInitializer = [](){}, std::function<void ()> threadDestructor = [](){});
    ~job_queue();

    template<class returnType>
    std::future<returnType> post(std::packaged_task<returnType()>& pt)
    {
        struct movePT {
            std::packaged_task<returnType()> pt;
        };

        auto temp = new movePT({std::move(pt)});
        auto jobQueueInvoker = [temp](){
            temp->pt();
            delete temp;
        };

        std::unique_lock<std::mutex> lock(mutex);

        auto rt = temp->pt.get_future();
        jobQueue.push_back(jobQueueInvoker);

        condition.notify_one();
        
        return rt;
    }
};

#endif /* job_queue_hpp */
