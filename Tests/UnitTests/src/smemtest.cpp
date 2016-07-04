#include "portability.h"

#include "unittest.h"

#include "handlers.h"
#include "kernel.h"
#include "sml_Events.h"
#include "soar_instance.h"


//IMPORTANT:  DON'T USE THE VARIABLE success.  It is declared globally in another test suite and we don't own it here.

class SMemTest : public CPPUNIT_NS::TestCase
{
        CPPUNIT_TEST_SUITE(SMemTest);    // The name of this class

#ifdef DO_SMEM_TESTS
        CPPUNIT_TEST(testISupport);
        CPPUNIT_TEST(testISupportWithLearning);
#endif
        CPPUNIT_TEST_SUITE_END();

    public:
        void setUp();       // Called before each function outlined by CPPUNIT_TEST
        void tearDown();    // Called after each function outlined by CPPUNIT_TEST

    protected:

        void source(const std::string& path);

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

void SMemTest::testISupport()
{
    source("smem-i-support.soar");
    pAgent->RunSelf(10, sml::sml_DECISION);
    CPPUNIT_ASSERT(succeeded);
}

void SMemTest::testISupportWithLearning()
{
    source("smem-i-support.soar");
    std::string result = pAgent->ExecuteCommandLine("learn -e") ;
    CPPUNIT_ASSERT(pAgent->GetLastCommandLineResult());
    pAgent->RunSelf(10, sml::sml_DECISION);
    CPPUNIT_ASSERT(succeeded);
}
