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

    thread_local static SQLite::Database db;
};

class sqlite_thread_guard
{
    SQLite::Statement statement;

public:
    sqlite_thread_guard(const SQLite::Statement& s) : statement(sqlite_job_queue::db, s) {}

    SQLite::Statement& operator*() { return statement; }
    SQLite::Statement* operator->() { return &statement; }

    sqlite_thread_guard(const sqlite_thread_guard&) = delete;
    sqlite_thread_guard& operator=(const sqlite_thread_guard&) = delete;

    sqlite_thread_guard(sqlite_thread_guard&&) = default;
    sqlite_thread_guard& operator=(sqlite_thread_guard&&) = default;
};

#endif /* sqlite_job_queue_hpp */
