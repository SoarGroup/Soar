//
//  guard.hpp
//  Soar-xcode
//
//  Created by Alex Turner on 10/4/16.
//  Copyright © 2016 University of Michigan – Soar Group. All rights reserved.
//

#ifndef guard_hpp
#define guard_hpp

#include <functional>
#include "Statement.h"
#include "job_queue.hpp"

class guard
{
    std::function<void()> execute;

public:
    guard(std::function<void()> e) : execute(e) {}
    ~guard() { execute(); }
};

class reset_guard
{
    SQLite::Statement& to_reset;

public:
    reset_guard(SQLite::Statement& s) : to_reset(s) {}
    ~reset_guard() { to_reset.reset(); }
};

class wait_guard
{
    std::shared_ptr<job_queue::job> job;

public:
    wait_guard(std::shared_ptr<job_queue::job> j) : job(j) {}
    ~wait_guard() { job->wait(); }
};


#endif /* guard_hpp */
