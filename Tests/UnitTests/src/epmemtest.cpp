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
        CPPUNIT_TEST(testEpmemUnit_1);
        CPPUNIT_TEST(testEpmemUnit_2);
        CPPUNIT_TEST(testEpmemUnit_3);
        CPPUNIT_TEST(testEpmemUnit_4);
        CPPUNIT_TEST(testEpmemUnit_5);
        CPPUNIT_TEST(testEpmemUnit_6);
        CPPUNIT_TEST(testEpmemUnit_7);
        CPPUNIT_TEST(testEpmemUnit_8);
        CPPUNIT_TEST(testEpmemUnit_9);
        CPPUNIT_TEST(testEpmemUnit_10);
        CPPUNIT_TEST(testEpmemUnit_11);
        CPPUNIT_TEST(testEpmemUnit_12);
        CPPUNIT_TEST(testEpmemUnit_13);
        /* In Soar 9.6.0, this test should fail.  It was testing whether epmem could
         * determine that an LTI should only match after it was promoted to an LTI.
         * Epmem no longer tracks promotion time, so this should indeed return an
         * earlier episode.  Keeping around because it might make the basis of a good
         * test for epmem's new interaction with smem */
//        CPPUNIT_TEST(testEpmemUnit_14);
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

        void testEpmemUnit_1();
        void testEpmemUnit_2();
        void testEpmemUnit_3();
        void testEpmemUnit_4();
        void testEpmemUnit_5();
        void testEpmemUnit_6();
        void testEpmemUnit_7();
        void testEpmemUnit_8();
        void testEpmemUnit_9();
        void testEpmemUnit_10();
        void testEpmemUnit_11();
        void testEpmemUnit_12();
        void testEpmemUnit_13();
        void testEpmemUnit_14();
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
    #ifdef CONFIGURE_SOAR_FOR_UNIT_TESTS
    configure_for_unit_tests();
    #endif

    pAgent = pKernel->CreateAgent("soar1");
    CPPUNIT_ASSERT(pAgent != NULL);

    succeeded = false;
    pKernel->AddRhsFunction("succeeded", Handlers::MySuccessHandler,  &succeeded) ;
}

void EpmemTest::tearDown()
{
    #ifdef INIT_AFTER_RUN
    {
        sml::ClientAnalyzedXML response;
        pAgent->ExecuteCommandLineXML("soar init", &response);
        CPPUNIT_ASSERT_MESSAGE(response.GetResultString(), pAgent->GetLastCommandLineResult());
    }
    #endif
    pKernel->Shutdown();
    delete pKernel ;
    pKernel = 0;
    pAgent = 0;
}

void EpmemTest::testEpmemUnit_1()
{
    source("epmem_unit_test_1.soar");
    pAgent->RunSelf(141, sml::sml_DECISION);
    CPPUNIT_ASSERT(succeeded);
}

void EpmemTest::testEpmemUnit_2()
{
    source("epmem_unit_test_2.soar");
    pAgent->RunSelf(141, sml::sml_DECISION);
    CPPUNIT_ASSERT(succeeded);
}

void EpmemTest::testEpmemUnit_3()
{
    source("epmem_unit_test_3.soar");
    pAgent->RunSelf(141, sml::sml_DECISION);
    CPPUNIT_ASSERT(succeeded);
}

void EpmemTest::testEpmemUnit_4()
{
    source("epmem_unit_test_4.soar");
    pAgent->RunSelf(141, sml::sml_DECISION);
    CPPUNIT_ASSERT(succeeded);
}

void EpmemTest::testEpmemUnit_5()
{
    source("epmem_unit_test_5.soar");
    pAgent->RunSelf(141, sml::sml_DECISION);
    CPPUNIT_ASSERT(succeeded);
}

void EpmemTest::testEpmemUnit_6()
{
    source("epmem_unit_test_6.soar");
    pAgent->RunSelf(141, sml::sml_DECISION);
    CPPUNIT_ASSERT(succeeded);
}

void EpmemTest::testEpmemUnit_7()
{
    source("epmem_unit_test_7.soar");
    pAgent->RunSelf(141, sml::sml_DECISION);
    CPPUNIT_ASSERT(succeeded);
}

void EpmemTest::testEpmemUnit_8()
{
    source("epmem_unit_test_8.soar");
    pAgent->RunSelf(141, sml::sml_DECISION);
    CPPUNIT_ASSERT(succeeded);
}

void EpmemTest::testEpmemUnit_9()
{
    source("epmem_unit_test_9.soar");
    pAgent->RunSelf(141, sml::sml_DECISION);
    CPPUNIT_ASSERT(succeeded);
}

void EpmemTest::testEpmemUnit_10()
{
    source("epmem_unit_test_10.soar");
    pAgent->RunSelf(141, sml::sml_DECISION);
    CPPUNIT_ASSERT(succeeded);
}

void EpmemTest::testEpmemUnit_11()
{
    source("epmem_unit_test_11.soar");
    pAgent->RunSelf(141, sml::sml_DECISION);
    CPPUNIT_ASSERT(succeeded);
}

void EpmemTest::testEpmemUnit_12()
{
    source("epmem_unit_test_12.soar");
    pAgent->RunSelf(141, sml::sml_DECISION);
    CPPUNIT_ASSERT(succeeded);
}

void EpmemTest::testEpmemUnit_13()
{
    source("epmem_unit_test_13.soar");
    pAgent->RunSelf(141, sml::sml_DECISION);
    CPPUNIT_ASSERT(succeeded);
}

void EpmemTest::testEpmemUnit_14()
{
    source("epmem_unit_test_14.soar");
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
