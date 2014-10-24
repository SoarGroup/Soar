#include "portability.h"

#include "unittest.h"

#include "handlers.h"
#include "kernel.h"
#include "sml_Events.h"


//IMPORTANT:  DON'T USE THE VARIABLE success.  It is declared globally in another test suite and we don't own it here.

class ChunkTest : public CPPUNIT_NS::TestCase
{
        CPPUNIT_TEST_SUITE(ChunkTest);   // The name of this class

#ifdef DO_CHUNKING_TESTS
        CPPUNIT_TEST(testChunk1);
        CPPUNIT_TEST(testChunk2);
        CPPUNIT_TEST(testChunk3);
#endif
        CPPUNIT_TEST_SUITE_END();

    public:
        void setUp();       // Called before each function outlined by CPPUNIT_TEST
        void tearDown();    // Called after each function outlined by CPPUNIT_TEST

    protected:

        void source(const std::string& path);
        void check_solution(const std::string& path);

        void testChunk1();
        void testChunk2();
        void testChunk3();

        sml::Kernel* pKernel;
        sml::Agent* pAgent;
        bool succeeded;
};

CPPUNIT_TEST_SUITE_REGISTRATION(ChunkTest);

void ChunkTest::source(const std::string& path)
{
    pAgent->LoadProductions((std::string("test_agents/chunking-tests/") + path).c_str());
    CPPUNIT_ASSERT_MESSAGE(pAgent->GetLastErrorDescription(), pAgent->GetLastCommandLineResult());
}

void ChunkTest::check_solution(const std::string& path)
{
    pAgent->LoadProductions((std::string("test_agents/chunking-tests/") + path).c_str());
    CPPUNIT_ASSERT_MESSAGE(pAgent->GetLastErrorDescription(), pAgent->GetLastCommandLineResult());
    {
        sml::ClientAnalyzedXML response;
        pAgent->ExecuteCommandLineXML((std::string("test_agents/chunking-tests/") + path).c_str(), &response);
        std::cout << "Ignored is " << response.GetArgInt(sml::sml_Names::kParamIgnoredProductionCount, -1) << std::endl;
        CPPUNIT_ASSERT(response.GetArgInt(sml::sml_Names::kParamIgnoredProductionCount, -1) == 1);
    }
}

void ChunkTest::setUp()
{
    pKernel = 0;
    pAgent = 0;
    pKernel = sml::Kernel::CreateKernelInNewThread() ;
    CPPUNIT_ASSERT(pKernel != NULL);
    CPPUNIT_ASSERT_MESSAGE(pKernel->GetLastErrorDescription(), !pKernel->HadError());

    pAgent = pKernel->CreateAgent("soar1");
    CPPUNIT_ASSERT(pAgent != NULL);

    succeeded = false;
    pKernel->AddRhsFunction("succeeded", Handlers::MySuccessHandler,  &succeeded) ;

    source("setup.soar");
}

void ChunkTest::tearDown()
{
    pKernel->Shutdown();
    delete pKernel ;
    pKernel = 0;
    pAgent = 0;
}

void ChunkTest::testChunk1()
{
    source("chunk1.soar");
    pAgent->RunSelf(4, sml::sml_DECISION);
    CPPUNIT_ASSERT(succeeded);
    check_solution("chunk1.soar");
}

void ChunkTest::testChunk2()
{
    source("chunk2.soar");
    pAgent->RunSelf(4, sml::sml_DECISION);
    CPPUNIT_ASSERT(succeeded);
}

void ChunkTest::testChunk3()
{
    source("chunk3.soar");
    pAgent->RunSelf(4, sml::sml_DECISION);
    CPPUNIT_ASSERT(succeeded);
}
