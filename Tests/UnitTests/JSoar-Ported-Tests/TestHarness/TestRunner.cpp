//
//  TestRunner.cpp
//  Soar-xcode
//
//  Created by Alex Turner on 6/16/15.
//  Copyright © 2015 University of Michigan – Soar Group. All rights reserved.
//

#include "TestRunner.hpp"

TestRunner::TestRunner(TestCategory* c, TestFunction* f, std::condition_variable* v)
: category(c), function(f), variable(v), kill(false), ready(false), done(false)
{}

void TestRunner::run()
{
	std::mutex mutex;
	std::unique_lock<std::mutex> lock(mutex);
	variable->wait(lock, [this]{ return ready == true; });

	category->runner = this;
	
	category->before();
	(*function)();
	category->after();
	
	done.store(true);
	variable->notify_all();
}
