//
//  TestRunner.cpp
//  Prototype-UnitTesting
//
//  Created by Alex Turner on 6/16/15.
//  Copyright © 2015 University of Michigan – Soar Group. All rights reserved.
//

#include "TestRunner.hpp"

TestRunner::TestRunner(TestCategory* c, TestFunction* f, std::condition_variable_any* v)
: category(c), function(f), variable(v), kill(false), ready(false), done(false), failed(false)
{}

void TestRunner::run()
{
	std::mutex mutex;
	std::unique_lock<std::mutex> lock(mutex);
	variable->wait(lock, [this]{ return ready == true; });

	category->runner = this;
	
	
	try {
		category->before();
		(*function)();
		category->after(false);
	} catch (std::exception& e) {
		failed = true;
		failureMessage = e.what();
		category->after(true);
	}
	
	done.store(true);
	variable->notify_all();
}
