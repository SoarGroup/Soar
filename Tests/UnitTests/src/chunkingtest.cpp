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
        CPPUNIT_TEST(testChunk4);
        CPPUNIT_TEST(testChunk5);
        CPPUNIT_TEST(testChunk6);
        CPPUNIT_TEST(testChunk7);
        CPPUNIT_TEST(testChunk8);
        CPPUNIT_TEST(testChunk9);
        CPPUNIT_TEST(testChunk10);
        CPPUNIT_TEST(testChunk11);
        CPPUNIT_TEST(testChunk12);
        CPPUNIT_TEST(testChunk13);
        CPPUNIT_TEST(testChunk14);
        CPPUNIT_TEST(testChunk15);
        CPPUNIT_TEST(testChunk16);
        CPPUNIT_TEST(testChunk17);
        CPPUNIT_TEST(testChunk18);
        CPPUNIT_TEST(testChunk19);
        CPPUNIT_TEST(testChunk20);
        CPPUNIT_TEST(testChunk21);
        CPPUNIT_TEST(testChunk22);
#endif
        CPPUNIT_TEST_SUITE_END();

    public:
        void setUp();       // Called before each function outlined by CPPUNIT_TEST
        void tearDown();    // Called after each function outlined by CPPUNIT_TEST

    protected:

        void source(const std::string& path);
        void build_and_check_chunk(const std::string& path, int64_t decisions, int64_t num_excised);

        void testChunk1();
        void testChunk2();
        void testChunk3();
        void testChunk4();
        void testChunk5();
        void testChunk6();
        void testChunk7();
        void testChunk8();
        void testChunk9();
        void testChunk10();
        void testChunk11();
        void testChunk12();
        void testChunk13();
        void testChunk14();
        void testChunk15();
        void testChunk16();
        void testChunk17();
        void testChunk18();
        void testChunk19();
        void testChunk20();
        void testChunk21();
        void testChunk22();

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

void ChunkTest::build_and_check_chunk(const std::string& path, int64_t decisions, int64_t num_excised)
{
    source(path.c_str());
    pAgent->RunSelf(decisions, sml::sml_DECISION);
    CPPUNIT_ASSERT_MESSAGE(pAgent->GetLastErrorDescription(), pAgent->GetLastCommandLineResult());
//    CPPUNIT_ASSERT(succeeded);
    {
        sml::ClientAnalyzedXML response;
        pAgent->ExecuteCommandLineXML((std::string("source test_agents/chunking-tests/expected/") + path).c_str(), &response);
        CPPUNIT_ASSERT(response.GetArgInt(sml::sml_Names::kParamExcisedProductionCount, -1) == num_excised);
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
    build_and_check_chunk("chunk1.soar", 4, 2);
}

void ChunkTest::testChunk2()
{
    build_and_check_chunk("chunk2.soar", 8, 1);
}

void ChunkTest::testChunk3()
{
    build_and_check_chunk("chunk3.soar", 8, 1);
}

void ChunkTest::testChunk4()
{
    build_and_check_chunk("chunk4.soar", 8, 1);
}

void ChunkTest::testChunk5()
{
    build_and_check_chunk("chunk5.soar", 8, 1);
}

void ChunkTest::testChunk6()
{
    build_and_check_chunk("chunk6.soar", 8, 1);
}

void ChunkTest::testChunk7()
{
    build_and_check_chunk("chunk7.soar", 8, 1);
}

void ChunkTest::testChunk8()
{
    build_and_check_chunk("chunk8.soar", 8, 1);
}

void ChunkTest::testChunk9()
{
    build_and_check_chunk("chunk9.soar", 8, 1);
}

void ChunkTest::testChunk10()
{
    build_and_check_chunk("chunk10.soar", 8, 1);
}

void ChunkTest::testChunk11()
{
    build_and_check_chunk("chunk11.soar", 8, 1);
}

void ChunkTest::testChunk12()
{
    build_and_check_chunk("chunk12.soar", 8, 1);
}

void ChunkTest::testChunk13()
{
    build_and_check_chunk("chunk13.soar", 8, 1);
}

void ChunkTest::testChunk14()
{
    build_and_check_chunk("chunk14.soar", 8, 1);
}

void ChunkTest::testChunk15()
{
    build_and_check_chunk("chunk15.soar", 8, 1);
}

void ChunkTest::testChunk16()
{
    build_and_check_chunk("chunk16.soar", 8, 1);
}

void ChunkTest::testChunk17()
{
    build_and_check_chunk("chunk17.soar", 8, 1);
}

void ChunkTest::testChunk18()
{
    build_and_check_chunk("chunk18.soar", 8, 1);
}

void ChunkTest::testChunk19()
{
    build_and_check_chunk("chunk19.soar", 8, 1);
}

void ChunkTest::testChunk20()
{
    build_and_check_chunk("chunk20.soar", 8, 1);
}

void ChunkTest::testChunk21()
{
    build_and_check_chunk("chunk21.soar", 8, 1);
}

void ChunkTest::testChunk22()
{
    build_and_check_chunk("chunk22.soar", 8, 1);
}
