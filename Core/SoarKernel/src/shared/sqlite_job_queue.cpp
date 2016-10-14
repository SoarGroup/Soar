//
//  sqlite_job_queue.cpp
//  Soar-xcode
//
//  Created by Alex Turner on 10/3/16.
//  Copyright © 2016 University of Michigan – Soar Group. All rights reserved.
//

#include "sqlite_job_queue.hpp"

thread_local std::shared_ptr<SQLite::Database> sqlite_job_queue::db;

sqlite_job_queue::sqlite_job_queue(std::string statementURL)
: job_queue([this,statementURL]() {
    db = std::shared_ptr<SQLite::Database>(new SQLite::Database(statementURL, SQLite::OPEN_READWRITE | SQLite::OPEN_URI));
})
{}
