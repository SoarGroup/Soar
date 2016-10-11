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

#endif /* guard_hpp */
