#include "portability.h"

#include "unittest.h"

#include "handlers.h"
#include "kernel.h"
#include "sml_Events.h"


//IMPORTANT:  DON'T USE THE VARIABLE success.  It is declared globally in another test suite and we don't own it here.

class SMemMathTest : public CPPUNIT_NS::TestCase
{
        CPPUNIT_TEST_SUITE(SMemMathTest);    // The name of this class
        
#ifdef DO_SMEM_MATH_TESTS
        CPPUNIT_TEST(testSimpleCueBasedRetrieval);
        CPPUNIT_TEST(testSimpleNonCueBasedRetrieval);
        CPPUNIT_TEST(testSimpleStore);
        CPPUNIT_TEST(testTrivialMathQuery);
        CPPUNIT_TEST(testBadMathQuery);
        CPPUNIT_TEST(testMaxQuery);
        CPPUNIT_TEST(testMaxMixedTypes);
        CPPUNIT_TEST(testMaxMultivalued);
        CPPUNIT_TEST(testMin);
        CPPUNIT_TEST(testMaxNegQuery);
        CPPUNIT_TEST(testGreater);
        CPPUNIT_TEST(testLess);
        CPPUNIT_TEST(testGreaterOrEqual);
        CPPUNIT_TEST(testLessOrEqual);
        CPPUNIT_TEST(testLessWithNeg);
        CPPUNIT_TEST(testLessNoSolution);
        CPPUNIT_TEST(testMirroring);
        CPPUNIT_TEST(testMergeAdd);
        CPPUNIT_TEST(testMergeNone);
        CPPUNIT_TEST(testSimpleStoreMultivaluedAttribute);
        CPPUNIT_TEST(testSimpleFloat);
        CPPUNIT_TEST(testMaxDoublePrecision_Irrational);
        CPPUNIT_TEST(testMaxDoublePrecision);
        CPPUNIT_TEST(testSimpleNonCueBasedRetrievalOfNonExistingLTI);
        CPPUNIT_TEST(testNegQuery);
        CPPUNIT_TEST(testNegStringFloat);
        CPPUNIT_TEST(testNegQueryNoHash);
#endif
        CPPUNIT_TEST_SUITE_END();
        
    public:
        void setUp();       // Called before each function outlined by CPPUNIT_TEST
        void tearDown();    // Called after each function outlined by CPPUNIT_TEST
        
    protected:
    
        void source(const std::string& path);
        
        void testSimpleCueBasedRetrieval();
        void testSimpleNonCueBasedRetrieval();
        void testSimpleStore();
        void testTrivialMathQuery();
        void testBadMathQuery();
        void testMaxQuery();
        void testMaxMixedTypes();
        void testMaxMultivalued();
        void testMin();
        void testGreater();
        void testLess();
        void testGreaterOrEqual();
        void testLessOrEqual();
        void testMaxNegQuery();
        void testLessWithNeg();
        void testLessNoSolution();
        void testMirroring();
        void testMergeAdd();
        void testMergeNone();
        void testSimpleStoreMultivaluedAttribute();
        void testSimpleFloat();
        void testMaxDoublePrecision_Irrational();
        void testMaxDoublePrecision();
        void testSimpleNonCueBasedRetrievalOfNonExistingLTI();
        void testNegQuery();
        void testNegStringFloat();
        void testNegQueryNoHash();
        
        sml::Kernel* pKernel;
        sml::Agent* pAgent;
        bool succeeded;
};

CPPUNIT_TEST_SUITE_REGISTRATION(SMemMathTest);

void SMemMathTest::source(const std::string& path)
{
    pAgent->LoadProductions((std::string("test_agents/smem-math-tests/") + path).c_str());
    CPPUNIT_ASSERT_MESSAGE(pAgent->GetLastErrorDescription(), pAgent->GetLastCommandLineResult());
}

void SMemMathTest::setUp()
{
    pKernel = 0;
    pAgent = 0;
    pKernel = sml::Kernel::CreateKernelInNewThread() ;
    CPPUNIT_ASSERT(pKernel != NULL);
    CPPUNIT_ASSERT_MESSAGE(pKernel->GetLastErrorDescription(), !pKernel->HadError());
    
    /* Sets Soar's output settings to what the unit tests expect.  Prevents
     * debug trace code from being output and causing some tests to appear to fail. */
    configure_for_unit_tests();

    pAgent = pKernel->CreateAgent("soar1");
    CPPUNIT_ASSERT(pAgent != NULL);
    
    succeeded = false;
    pKernel->AddRhsFunction("succeeded", Handlers::MySuccessHandler,  &succeeded) ;
}

void SMemMathTest::tearDown()
{
    pKernel->Shutdown();
    delete pKernel ;
    pKernel = 0;
    pAgent = 0;
}

void SMemMathTest::testSimpleCueBasedRetrieval()
{
    source("SMemFunctionalTests_testSimpleCueBasedRetrieval.soar");
    pAgent->RunSelf(3, sml::sml_DECISION);
    CPPUNIT_ASSERT(succeeded);
}

void SMemMathTest::testSimpleNonCueBasedRetrieval()
{
    source("SMemFunctionalTests_testSimpleNonCueBasedRetrieval.soar");
    pAgent->RunSelf(6, sml::sml_DECISION);
    CPPUNIT_ASSERT(succeeded);
}

void SMemMathTest::testSimpleStore()
{
    source("SMemFunctionalTests_testSimpleStore.soar");
    pAgent->RunSelf(3, sml::sml_DECISION);
    CPPUNIT_ASSERT(succeeded);
}

void SMemMathTest::testTrivialMathQuery()
{
    source("SMemFunctionalTests_testTrivialMathQuery.soar");
    pAgent->RunSelf(3, sml::sml_DECISION);
    CPPUNIT_ASSERT(succeeded);
}

void SMemMathTest::testBadMathQuery()
{
    source("SMemFunctionalTests_testBadMathQuery.soar");
    pAgent->RunSelf(3, sml::sml_DECISION);
    CPPUNIT_ASSERT(succeeded);
}

void SMemMathTest::testMaxQuery()
{
    source("SMemFunctionalTests_testMax.soar");
    pAgent->RunSelf(2, sml::sml_DECISION);
    CPPUNIT_ASSERT(succeeded);
}
void SMemMathTest::testMaxMixedTypes()
{
    source("SMemFunctionalTests_testMaxMixedTypes.soar");
    pAgent->RunSelf(2, sml::sml_DECISION);
    CPPUNIT_ASSERT(succeeded);
}
void SMemMathTest::testMaxMultivalued()
{
    source("SMemFunctionalTests_testMaxMultivalued.soar");
    pAgent->RunSelf(2, sml::sml_DECISION);
    CPPUNIT_ASSERT(succeeded);
}
void SMemMathTest::testMin()
{
    source("SMemFunctionalTests_testMin.soar");
    pAgent->RunSelf(2, sml::sml_DECISION);
    CPPUNIT_ASSERT(succeeded);
}
void SMemMathTest::testMaxNegQuery()
{
    source("SMemFunctionalTests_testMaxNegation.soar");
    pAgent->RunSelf(2, sml::sml_DECISION);
    CPPUNIT_ASSERT(succeeded);
}

void SMemMathTest::testGreater()
{
    source("SMemFunctionalTests_testGreater.soar");
    pAgent->RunSelf(2, sml::sml_DECISION);
    CPPUNIT_ASSERT(succeeded);
}
void SMemMathTest::testLess()
{
    source("SMemFunctionalTests_testLess.soar");
    pAgent->RunSelf(2, sml::sml_DECISION);
    CPPUNIT_ASSERT(succeeded);
}
void SMemMathTest::testGreaterOrEqual()
{
    source("SMemFunctionalTests_testGreaterOrEqual.soar");
    pAgent->RunSelf(2, sml::sml_DECISION);
    CPPUNIT_ASSERT(succeeded);
}
void SMemMathTest::testLessOrEqual()
{
    source("SMemFunctionalTests_testLessOrEqual.soar");
    pAgent->RunSelf(2, sml::sml_DECISION);
    CPPUNIT_ASSERT(succeeded);
}
void SMemMathTest::testLessWithNeg()
{
    source("SMemFunctionalTests_testLessWithNeg.soar");
    pAgent->RunSelf(2, sml::sml_DECISION);
    CPPUNIT_ASSERT(succeeded);
}
void SMemMathTest::testLessNoSolution()
{
    source("SMemFunctionalTests_testLessNoSolution.soar");
    pAgent->RunSelf(2, sml::sml_DECISION);
    CPPUNIT_ASSERT(succeeded);
}
void SMemMathTest::testMirroring()
{
    source("SMemFunctionalTests_testMirroring.soar");
    pAgent->RunSelf(5, sml::sml_DECISION);
    CPPUNIT_ASSERT(succeeded);
}
void SMemMathTest::testMergeAdd()
{
    source("SMemFunctionalTests_testMergeAdd.soar");
    pAgent->RunSelf(5, sml::sml_DECISION);
    CPPUNIT_ASSERT(succeeded);
}
void SMemMathTest::testMergeNone()
{
    source("SMemFunctionalTests_testMergeNone.soar");
    pAgent->RunSelf(5, sml::sml_DECISION);
    CPPUNIT_ASSERT(succeeded);
}

void SMemMathTest::testSimpleStoreMultivaluedAttribute()
{
    source("SMemFunctionalTests_testLess.soar");
    pAgent->RunSelf(2, sml::sml_DECISION);
    CPPUNIT_ASSERT(succeeded);
}
void SMemMathTest::testSimpleFloat()
{
    source("SMemFunctionalTests_testLessWithNeg.soar");
    pAgent->RunSelf(2, sml::sml_DECISION);
    CPPUNIT_ASSERT(succeeded);
}
void SMemMathTest::testMaxDoublePrecision_Irrational()
{
    source("SMemFunctionalTests_testLessNoSolution.soar");
    pAgent->RunSelf(2, sml::sml_DECISION);
    CPPUNIT_ASSERT(succeeded);
}
void SMemMathTest::testMaxDoublePrecision()
{
    source("SMemFunctionalTests_testMirroring.soar");
    pAgent->RunSelf(5, sml::sml_DECISION);
    CPPUNIT_ASSERT(succeeded);
}
void SMemMathTest::testSimpleNonCueBasedRetrievalOfNonExistingLTI()
{
    source("SMemFunctionalTests_testMergeAdd.soar");
    pAgent->RunSelf(5, sml::sml_DECISION);
    CPPUNIT_ASSERT(succeeded);
}
void SMemMathTest::testNegQuery()
{
    source("SMemFunctionalTests_testMergeNone.soar");
    pAgent->RunSelf(5, sml::sml_DECISION);
    CPPUNIT_ASSERT(succeeded);
}
void SMemMathTest::testNegStringFloat()
{
    source("SMemFunctionalTests_testNegStringFloat.soar");
    pAgent->RunSelf(6, sml::sml_DECISION);
    CPPUNIT_ASSERT(succeeded);
}
void SMemMathTest::testNegQueryNoHash()
{
    source("SMemFunctionalTests_testNegQueryNoHash.soar");
    pAgent->RunSelf(2, sml::sml_DECISION);
    CPPUNIT_ASSERT(succeeded);
}

