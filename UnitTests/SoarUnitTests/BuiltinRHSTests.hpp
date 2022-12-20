//  Copyright © 2022 University of Michigan – Soar Group. All rights reserved.

#ifndef BuiltinRHSTests_cpp
#define BuiltinRHSTests_cpp

#include "FunctionalTestHarness.hpp"

class BuiltinRHSTests : public FunctionalTestHarness
{
public:
    TEST_CATEGORY(BuiltinRHSTests);

    TEST(testString, -1)
    void testString();
};

#endif /* BuiltinRHSTests_cpp */
