//
//  TestCategory.hpp
//  Prototype-UnitTesting
//
//  Created by Alex Turner on 6/16/15.
//  Copyright © 2015 University of Michigan – Soar Group. All rights reserved.
//

#ifndef TestCategory_cpp
#define TestCategory_cpp

#include <cstdint>

#include <vector>
#include <functional>
#include <map>
#include <string>
#include <iostream>
#include <type_traits>

#include "TestHelpers.hpp"
#include "TestRunner.hpp"

class TestCategory
{
public:
	typedef std::tuple<std::function<void ()>, uint64_t, std::string> TestCategory_test;
	
public:
	virtual ~TestCategory() {}
	
	std::vector<TestCategory_test> m_TestCategory_tests;

	const std::string getCategoryName() { return TestHelpers::demangle(typeid(*this).name()); /*m_categoryName;*/ }
	const std::vector<TestCategory_test> getTests() { return m_TestCategory_tests; }
	
	TestRunner* runner;
		
	class Test
	{
	public:
		Test(std::string testName, std::function<void ()> testFunction, uint64_t timeoutMs, std::vector<TestCategory_test>& tests)
		{
            tests.push_back(std::make_tuple(testFunction, timeoutMs == -1 ? 60000 : timeoutMs, testName));
		}
	};

	virtual void before() {};
	
	// if caught, you are not allowed to throw more exceptions which, if done, will be ignored.
	virtual void after(bool caught) {};
};

#endif /* TestCategory_cpp */
