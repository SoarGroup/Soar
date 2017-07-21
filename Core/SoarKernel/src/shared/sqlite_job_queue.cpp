//
//  sqlite_job_queue.cpp
//  Soar-xcode
//
//  Created by Alex Turner on 10/3/16.
//  Copyright © 2016 University of Michigan – Soar Group. All rights reserved.
//

#include "sqlite_job_queue.hpp"
#include "semantic_memory.h"

thread_local SQLite::Database sqlite_job_queue::db(SMem_Manager::memoryDatabasePath, SMem_Manager::sqlite3Flags, SMem_Manager::sqlite3Timeout);

sqlite_job_queue::sqlite_job_queue(std::string statementURL)
: job_queue([this,statementURL]() {
    sqlite_job_queue::db.reconnect(statementURL, SMem_Manager::sqlite3Flags, SMem_Manager::sqlite3Timeout);
},
            [this,statementURL]() {
    sqlite_job_queue::db.disconnect();
})
{}

