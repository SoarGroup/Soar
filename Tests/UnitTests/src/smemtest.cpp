#include "portability.h"

#include "unittest.h"

#include "handlers.h"
#include "sml_Events.h"
#include "soar_instance.h"


//IMPORTANT:  DON'T USE THE VARIABLE success.  It is declared globally in another test suite and we don't own it here.

class SMemTest : public CPPUNIT_NS::TestCase
{
        CPPUNIT_TEST_SUITE(SMemTest);    // The name of this class
        
#ifdef DO_SMEM_TESTS

        CPPUNIT_TEST(testSimpleStore);
        CPPUNIT_TEST(testSimpleStoreMultivaluedAttribute);
        CPPUNIT_TEST(testSimpleCueBasedRetrieval);
        CPPUNIT_TEST(testSimpleNonCueBasedRetrieval);
        CPPUNIT_TEST(testSimpleNonCueBasedRetrievalOfNonExistingLTI);
        CPPUNIT_TEST(testTrivialMathQuery);
        CPPUNIT_TEST(testBadMathQuery);
        CPPUNIT_TEST(testSimpleFloat);
        CPPUNIT_TEST(testGreater);
        CPPUNIT_TEST(testGreaterOrEqual);
        CPPUNIT_TEST(testLess);
        CPPUNIT_TEST(testLessOrEqual);
        CPPUNIT_TEST(testLessWithNeg);
        CPPUNIT_TEST(testLessNoSolution);
        CPPUNIT_TEST(testMin);
        CPPUNIT_TEST(testMaxQuery);
        CPPUNIT_TEST(testMaxDoublePrecision);
        CPPUNIT_TEST(testMaxDoublePrecision_Irrational);
        CPPUNIT_TEST(testMaxMixedTypes);
        CPPUNIT_TEST(testMaxMultivalued);
        CPPUNIT_TEST(testNegStringFloat);
        CPPUNIT_TEST(testMaxNegQuery);
        CPPUNIT_TEST(testNegQuery);
        CPPUNIT_TEST(testNegQueryNoHash);
        CPPUNIT_TEST(testISupport);
        CPPUNIT_TEST(testISupportWithLearning);
        #ifndef SKIP_SLOW_TESTS
            CPPUNIT_TEST(testSmemArithmetic);
        #endif
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
        void testSimpleStoreMultivaluedAttribute();
        void testSimpleFloat();
        void testMaxDoublePrecision_Irrational();
        void testMaxDoublePrecision();
        void testSimpleNonCueBasedRetrievalOfNonExistingLTI();
        void testNegQuery();
        void testNegStringFloat();
        void testNegQueryNoHash();
        void testSmemArithmetic();
        void testISupport();
        void testISupportWithLearning();
        
        sml::Kernel* pKernel;
        sml::Agent* pAgent;
        bool succeeded;
};

CPPUNIT_TEST_SUITE_REGISTRATION(SMemTest);

void SMemTest::source(const std::string& path)
{
    pAgent->LoadProductions((std::string("test_agents/smemtests/") + path).c_str());
    CPPUNIT_ASSERT_MESSAGE(pAgent->GetLastErrorDescription(), pAgent->GetLastCommandLineResult());
}

void SMemTest::setUp()
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

void SMemTest::tearDown()
{
    pKernel->Shutdown();
    delete pKernel ;
    pKernel = 0;
    pAgent = 0;
}

void SMemTest::testSimpleCueBasedRetrieval()
{
    source("SimpleCueBasedRetrieval.soar");
    pAgent->RunSelf(3, sml::sml_DECISION);
    CPPUNIT_ASSERT(succeeded);
}

void SMemTest::testSimpleNonCueBasedRetrieval()
{
    source("SimpleNonCueBasedRetrieval.soar");
    pAgent->RunSelf(6, sml::sml_DECISION);
    CPPUNIT_ASSERT(succeeded);
}

void SMemTest::testSimpleStore()
{
    source("SimpleStore.soar");
    pAgent->RunSelf(3, sml::sml_DECISION);
    CPPUNIT_ASSERT(succeeded);
}

void SMemTest::testTrivialMathQuery()
{
    source("TrivialMathQuery.soar");
    pAgent->RunSelf(3, sml::sml_DECISION);
    CPPUNIT_ASSERT(succeeded);
}

void SMemTest::testBadMathQuery()
{
    source("BadMathQuery.soar");
    pAgent->RunSelf(3, sml::sml_DECISION);
    CPPUNIT_ASSERT(succeeded);
}

void SMemTest::testMaxQuery()
{
    source("Max.soar");
    pAgent->RunSelf(2, sml::sml_DECISION);
    CPPUNIT_ASSERT(succeeded);
}
void SMemTest::testMaxMixedTypes()
{
    source("MaxMixedTypes.soar");
    pAgent->RunSelf(2, sml::sml_DECISION);
    CPPUNIT_ASSERT(succeeded);
}
void SMemTest::testMaxMultivalued()
{
    source("MaxMultivalued.soar");
    pAgent->RunSelf(2, sml::sml_DECISION);
    CPPUNIT_ASSERT(succeeded);
}
void SMemTest::testMin()
{
    source("Min.soar");
    pAgent->RunSelf(2, sml::sml_DECISION);
    CPPUNIT_ASSERT(succeeded);
}
void SMemTest::testMaxNegQuery()
{
    source("MaxNegation.soar");
    pAgent->RunSelf(2, sml::sml_DECISION);
    CPPUNIT_ASSERT(succeeded);
}

void SMemTest::testGreater()
{
    source("Greater.soar");
    pAgent->RunSelf(2, sml::sml_DECISION);
    CPPUNIT_ASSERT(succeeded);
}
void SMemTest::testLess()
{
    source("Less.soar");
    pAgent->RunSelf(2, sml::sml_DECISION);
    CPPUNIT_ASSERT(succeeded);
}
void SMemTest::testGreaterOrEqual()
{
    source("GreaterOrEqual.soar");
    pAgent->RunSelf(2, sml::sml_DECISION);
    CPPUNIT_ASSERT(succeeded);
}
void SMemTest::testLessOrEqual()
{
    source("LessOrEqual.soar");
    pAgent->RunSelf(2, sml::sml_DECISION);
    CPPUNIT_ASSERT(succeeded);
}
void SMemTest::testLessWithNeg()
{
    source("LessWithNeg.soar");
    pAgent->RunSelf(2, sml::sml_DECISION);
    CPPUNIT_ASSERT(succeeded);
}
void SMemTest::testLessNoSolution()
{
    source("LessNoSolution.soar");
    pAgent->RunSelf(2, sml::sml_DECISION);
    CPPUNIT_ASSERT(succeeded);
}
void SMemTest::testSimpleStoreMultivaluedAttribute()
{
    source("SimpleStoreMultivaluedAttribute.soar");
    pAgent->RunSelf(3, sml::sml_DECISION);
    CPPUNIT_ASSERT(succeeded);
}
void SMemTest::testSimpleFloat()
{
    source("SimpleFloat.soar");
    pAgent->RunSelf(6, sml::sml_DECISION);
    CPPUNIT_ASSERT(succeeded);
}
void SMemTest::testMaxDoublePrecision_Irrational()
{
    source("MaxDoublePrecision-Irrational.soar");
    pAgent->RunSelf(6, sml::sml_DECISION);
    CPPUNIT_ASSERT(succeeded);
}
void SMemTest::testMaxDoublePrecision()
{
    source("MaxDoublePrecision.soar");
    pAgent->RunSelf(6, sml::sml_DECISION);
    CPPUNIT_ASSERT(succeeded);
}
void SMemTest::testSimpleNonCueBasedRetrievalOfNonExistingLTI()
{
    source("SimpleNonCueBasedRetrievalOfNonExistingLTI.soar");
    pAgent->RunSelf(3, sml::sml_DECISION);
    CPPUNIT_ASSERT_MESSAGE(pAgent->GetLastErrorDescription(), pAgent->GetLastCommandLineResult());
    CPPUNIT_ASSERT(succeeded);
}
void SMemTest::testNegQuery()
{
    source("NegQuery.soar");
    pAgent->RunSelf(250, sml::sml_DECISION);
    CPPUNIT_ASSERT(succeeded);
}
void SMemTest::testNegStringFloat()
{
    source("NegStringFloat.soar");
    pAgent->RunSelf(6, sml::sml_DECISION);
    CPPUNIT_ASSERT(succeeded);
}
void SMemTest::testNegQueryNoHash()
{
    source("NegQueryNoHash.soar");
    pAgent->RunSelf(2, sml::sml_DECISION);
    CPPUNIT_ASSERT(succeeded);
}
void SMemTest::testSmemArithmetic()
{
    source("arithmetic/arithmetic.soar") ;
    pAgent->ExecuteCommandLine("watch 0");
    pAgent->ExecuteCommandLine("srand 1080");

    pAgent->RunSelfForever();

    sml::ClientAnalyzedXML stats;
    pAgent->ExecuteCommandLineXML("stats", &stats);
    CPPUNIT_ASSERT(stats.GetArgInt(sml::sml_Names::kParamStatsCycleCountDecision, -1) == 46436);
}

void SMemTest::testISupport()
{
    source("smem-i-support.soar");
    pAgent->RunSelf(5, sml::sml_DECISION);
    CPPUNIT_ASSERT(succeeded);
}

void SMemTest::testISupportWithLearning()
{
    source("smem-i-support.soar");
    std::string result = pAgent->ExecuteCommandLine("chunk always") ;
    CPPUNIT_ASSERT(pAgent->GetLastCommandLineResult());
    pAgent->RunSelf(5, sml::sml_DECISION);
    CPPUNIT_ASSERT(succeeded);
}

