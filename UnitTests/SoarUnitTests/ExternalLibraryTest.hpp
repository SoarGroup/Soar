#ifndef ExternalLibraryTest_hpp
#define ExternalLibraryTest_hpp

#include "FunctionalTestHarness.hpp"

class ExternalLibraryTest : public FunctionalTestHarness
{
public:
    TEST_CATEGORY(ExternalLibraryTest);

    TEST(testLoadLibrary, -1)
    void testLoadLibrary();
};

#endif
