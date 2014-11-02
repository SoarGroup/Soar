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
        CPPUNIT_TEST(testChunk23);
        CPPUNIT_TEST(testChunk24);
        CPPUNIT_TEST(testChunk25);
        CPPUNIT_TEST(testChunk26);
        CPPUNIT_TEST(testChunk27);
#endif
        CPPUNIT_TEST_SUITE_END();

    public:
        void setUp();       // Called before each function outlined by CPPUNIT_TEST
        void tearDown();    // Called after each function outlined by CPPUNIT_TEST

    protected:

        void source(const std::string& path);
        void build_and_check_chunk(const std::string& path, int64_t decisions, int64_t expected_chunks);

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
        void testChunk23();
        void testChunk24();
        void testChunk25();
        void testChunk26();
        void testChunk27();

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

void ChunkTest::build_and_check_chunk(const std::string& path, int64_t decisions, int64_t expected_chunks)
{
    source(path.c_str());
    pAgent->RunSelf(decisions, sml::sml_DECISION);
    CPPUNIT_ASSERT_MESSAGE(pAgent->GetLastErrorDescription(), pAgent->GetLastCommandLineResult());
//    CPPUNIT_ASSERT(succeeded);
    {
        sml::ClientAnalyzedXML response;
        pAgent->ExecuteCommandLineXML((std::string("source test_agents/chunking-tests/expected/") + path).c_str(), &response);
        int excised, ignored;
        excised = response.GetArgInt(sml::sml_Names::kParamExcisedProductionCount, -1);
        ignored = response.GetArgInt(sml::sml_Names::kParamIgnoredProductionCount, -1);
        CPPUNIT_ASSERT((excised + ignored) == expected_chunks);
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
    source("setup.soar");
    build_and_check_chunk("chunk1.soar", 4, 2);
}

void ChunkTest::testChunk2()
{
    source("setup.soar");
    build_and_check_chunk("chunk2.soar", 8, 1);
}

void ChunkTest::testChunk3()
{
    source("setup.soar");
    build_and_check_chunk("chunk3.soar", 8, 1);
}

void ChunkTest::testChunk4()
{
    source("setup.soar");
    build_and_check_chunk("chunk4.soar", 8, 1);
}

void ChunkTest::testChunk5()
{
    source("setup.soar");
    build_and_check_chunk("chunk5.soar", 8, 1);
}

void ChunkTest::testChunk6()
{
    source("setup.soar");
    build_and_check_chunk("chunk6.soar", 8, 1);
}

void ChunkTest::testChunk7()
{
    source("setup.soar");
    build_and_check_chunk("chunk7.soar", 8, 3);
}

void ChunkTest::testChunk8()
{
    source("setup.soar");
    build_and_check_chunk("chunk8.soar", 8, 1);
}

void ChunkTest::testChunk9()
{
    source("setup.soar");
    build_and_check_chunk("chunk9.soar", 8, 2);
}

void ChunkTest::testChunk10()
{
    source("setup.soar");
    build_and_check_chunk("chunk10.soar", 8, 2);
}

void ChunkTest::testChunk11()
{
    source("setup.soar");
    build_and_check_chunk("chunk11.soar", 8, 1);
}

void ChunkTest::testChunk12()
{
    source("setup.soar");
    build_and_check_chunk("chunk12.soar", 8, 2);
}

void ChunkTest::testChunk13()
{
    source("setup.soar");
    build_and_check_chunk("chunk13.soar", 8, 1);
}

void ChunkTest::testChunk14()
{
    source("setup.soar");
    build_and_check_chunk("chunk14.soar", 8, 1);
}

void ChunkTest::testChunk15()
{
    source("setup.soar");
    build_and_check_chunk("chunk15.soar", 8, 1);
}

void ChunkTest::testChunk16()
{
    source("setup.soar");
    build_and_check_chunk("chunk16.soar", 8, 1);
}

void ChunkTest::testChunk17()
{
    source("setup.soar");
    build_and_check_chunk("chunk17.soar", 8, 1);
}

void ChunkTest::testChunk18()
{
    source("setup.soar");
    build_and_check_chunk("chunk18.soar", 8, 1);
}

void ChunkTest::testChunk19()
{
    source("setup.soar");
    build_and_check_chunk("chunk19.soar", 8, 1);
}

void ChunkTest::testChunk20()
{
    source("setup.soar");
    build_and_check_chunk("chunk20.soar", 8, 1);
}

void ChunkTest::testChunk21()
{
    source("setup.soar");
    build_and_check_chunk("chunk21.soar", 8, 1);
}

void ChunkTest::testChunk22()
{
    source("setup.soar");
    build_and_check_chunk("chunk22.soar", 8, 1);
}

void ChunkTest::testChunk23()
{
    source("setup.soar");
    build_and_check_chunk("chunk23.soar", 8, 1);
}

void ChunkTest::testChunk24()
{
    source("setup.soar");
    build_and_check_chunk("chunk24.soar", 8, 1);
}

void ChunkTest::testChunk25()
{
    source("setup.soar");
    build_and_check_chunk("chunk25.soar", 8, 1);
}

void ChunkTest::testChunk26()
{
    build_and_check_chunk("chunk26.soar", 8, 0);
}

void ChunkTest::testChunk27()
{
    build_and_check_chunk("chunk27.soar", 8, 1);
}
