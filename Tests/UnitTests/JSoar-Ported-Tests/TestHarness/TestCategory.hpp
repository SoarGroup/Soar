//
//  TestCategory.hpp
//  Soar-xcode
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

#include "TestHelpers.hpp"

class TestRunner;

class TestCategory
{
public:
	typedef std::tuple<TestFunction*, uint64_t, std::string> TestCategory_test;

private:
	TestCategory();
	
	std::string m_categoryName;
	
public:
	std::vector<TestCategory_test> m_TestCategory_tests;

	const std::string getCategoryName() { return m_categoryName; }
	const std::vector<TestCategory_test> getTests() { return m_TestCategory_tests; }
	
	TestRunner* runner;
		
	class Test
	{
	public:
		Test(std::string testName, TestFunction* testFunction, uint64_t timeoutMs, std::vector<TestCategory_test>& tests)
		{
			tests.push_back(std::make_tuple(testFunction, timeoutMs, testName));
		}
	};
	
	TestCategory(std::string testCategoryName) : m_categoryName(testCategoryName) {}
		
	virtual void before() {};
	
	// if caught, you are not allowed to throw more exceptions which, if done, will cause the unit testing framework to crash.
	virtual void after(bool caught) {};
};

#endif /* TestCategory_cpp */
