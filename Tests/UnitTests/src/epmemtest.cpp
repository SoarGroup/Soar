#include "portability.h"

#include "unittest.h"

#include "handlers.h"
#include "sml_Events.h"
#include "soar_instance.h"


//IMPORTANT:  DON'T USE THE VARIABLE success.  It is declared globally in another test suite and we don't own it here.

class EpmemTest : public CPPUNIT_NS::TestCase
{
        CPPUNIT_TEST_SUITE(EpmemTest);   // The name of this class

#ifdef DO_EPMEM_TESTS
        /* Fails because of new smem */
        CPPUNIT_TEST(testEpmemUnit);
        CPPUNIT_TEST(testHamiltonian);
        CPPUNIT_TEST(testSVS);
        CPPUNIT_TEST(testSVSHard);
#endif
        CPPUNIT_TEST_SUITE_END();

    public:
        void setUp();       // Called before each function outlined by CPPUNIT_TEST
        void tearDown();    // Called after each function outlined by CPPUNIT_TEST

    protected:

        void source(const std::string& path);

        void testEpmemUnit();
        void testHamiltonian();
        void testSVS();
        void testSVSHard();

        sml::Kernel* pKernel;
        sml::Agent* pAgent;
        bool succeeded;
};

CPPUNIT_TEST_SUITE_REGISTRATION(EpmemTest);

void EpmemTest::source(const std::string& path)
{
    pAgent->LoadProductions((std::string("test_agents/epmemtests/") + path).c_str());
    CPPUNIT_ASSERT_MESSAGE(pAgent->GetLastErrorDescription(), pAgent->GetLastCommandLineResult());
}

void EpmemTest::setUp()
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

void EpmemTest::tearDown()
{
    pKernel->Shutdown();
    delete pKernel ;
    pKernel = 0;
    pAgent = 0;
}

void EpmemTest::testEpmemUnit()
{
    source("epmem_unit.soar");
    pAgent->RunSelf(141, sml::sml_DECISION);
    CPPUNIT_ASSERT(succeeded);
}

void EpmemTest::testHamiltonian()
{
    source("hamiltonian.soar");
    pAgent->RunSelf(3, sml::sml_DECISION);
    CPPUNIT_ASSERT(succeeded);
}

void EpmemTest::testSVS()
{
    source("svs.soar");
    pAgent->RunSelf(3, sml::sml_DECISION);
    CPPUNIT_ASSERT(succeeded);
}

void EpmemTest::testSVSHard()
{
    source("svs_hard.soar");
    pAgent->RunSelf(3, sml::sml_DECISION);
    CPPUNIT_ASSERT(succeeded);
}
