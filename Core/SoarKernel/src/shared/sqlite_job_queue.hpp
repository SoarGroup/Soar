//
//  sqlite_job_queue.hpp
//  Soar-xcode
//
//  Created by Alex Turner on 9/25/16.
//  Copyright © 2016 University of Michigan – Soar Group. All rights reserved.
//

#ifndef sqlite_job_queue_hpp
#define sqlite_job_queue_hpp

#include "job_queue.hpp"

#include "Database.h"

#include <memory>

class sqlite_job_queue : public job_queue
{
public:
    sqlite_job_queue(std::string statementURL);

    thread_local static std::shared_ptr<SQLite::Database> db;
};

#endif /* sqlite_job_queue_hpp */
