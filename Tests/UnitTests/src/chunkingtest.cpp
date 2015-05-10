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
        CPPUNIT_TEST(RL_Variablization);
        CPPUNIT_TEST(STI_Variablization);
        CPPUNIT_TEST(STI_Variablization_Same_Type);
        CPPUNIT_TEST(Superstate_Identity_Opaque);
        CPPUNIT_TEST(RHS_Unbound_Multivalue);
        CPPUNIT_TEST(All_Test_Types);
        CPPUNIT_TEST(Conflated_Constants);
        CPPUNIT_TEST(Rete_Bug_Deep_vs_Top);
        CPPUNIT_TEST(Rete_Bug_Deep_vs_Deep);
        CPPUNIT_TEST(Ungrounded_STIs);
        CPPUNIT_TEST(Ungrounded_Mixed);
        CPPUNIT_TEST(Ungrounded_STI_Promotion);
        CPPUNIT_TEST(Ungrounded_Relational_Constraint);
        CPPUNIT_TEST(Ungrounded_in_BT_Constraint);
        CPPUNIT_TEST(Vrblzd_Constraint_on_Ungrounded);
        CPPUNIT_TEST(NC_Simple_No_Exist);
        CPPUNIT_TEST(NC_with_Relational_Constraint);
        CPPUNIT_TEST(NC_with_RC_and_Local_Variable);
        CPPUNIT_TEST(NCC_Simple_Literals);
        CPPUNIT_TEST(NCC_2_Conds_Simple_Literals);
        CPPUNIT_TEST(NCC_with_Relational_Constraint);
        CPPUNIT_TEST(NCC_Complex);
        CPPUNIT_TEST(NCC_from_Backtrace);
        CPPUNIT_TEST(Justification_RC_not_Ungrounded_STIs);
        CPPUNIT_TEST(Prohibit_Fake_Instantiation_LTIs);
        CPPUNIT_TEST(Maintain_Instantiation_Specific_Identity);
        CPPUNIT_TEST(Simple_Literalization);
        CPPUNIT_TEST(Literalization_of_NC_and_NCC);
        CPPUNIT_TEST(Literalization_with_Constraints);
        CPPUNIT_TEST(Simple_Constraint_Prop);
        CPPUNIT_TEST(Constraint_Prop_from_Base_Conds);
        CPPUNIT_TEST(BUNCPS_0);
        CPPUNIT_TEST(BUNCPS_1);
        CPPUNIT_TEST(BUNCPS_2);
        CPPUNIT_TEST(BUNCPS_3);
        CPPUNIT_TEST(BUNCPS_4);
        CPPUNIT_TEST(BUNCPS_5);
        CPPUNIT_TEST(BUNCPS_6_Four_Level);
        CPPUNIT_TEST(BUNCPS_7_with_Constraints);
//        CPPUNIT_TEST(testChunk40);
//        CPPUNIT_TEST(testChunk41);
//        CPPUNIT_TEST(testChunk42);
//        CPPUNIT_TEST(testChunk43);
//        CPPUNIT_TEST(testChunk44);
//        CPPUNIT_TEST(testChunk45);
//        CPPUNIT_TEST(testChunk46);
//        CPPUNIT_TEST(testChunk47);
//        CPPUNIT_TEST(testChunk48);
//        CPPUNIT_TEST(testChunk49);
#endif
        CPPUNIT_TEST_SUITE_END();

    public:
        void setUp();       // Called before each function outlined by CPPUNIT_TEST
        void tearDown();    // Called after each function outlined by CPPUNIT_TEST

    protected:

        void source(const std::string& path);
        void build_and_check_chunk(const std::string& path, int64_t decisions, int64_t expected_chunks);

        void STI_Variablization();
        void STI_Variablization_Same_Type();
        void All_Test_Types();
        void Conflated_Constants();
        void Ungrounded_Relational_Constraint();
        void Vrblzd_Constraint_on_Ungrounded();
        void Simple_Literalization();
        void Constraint_Prop_from_Base_Conds();
        void BUNCPS_7_with_Constraints();
        void Literalization_with_Constraints();
        void Ungrounded_in_BT_Constraint();
        void RHS_Unbound_Multivalue();
        void Rete_Bug_Deep_vs_Top();
        void Rete_Bug_Deep_vs_Deep();
        void Ungrounded_STIs();
        void Ungrounded_Mixed();
        void Ungrounded_STI_Promotion();
        void NC_with_RC_and_Local_Variable();
        void NCC_Simple_Literals();
        void NC_Simple_No_Exist();
        void NC_with_Relational_Constraint();
        void NCC_2_Conds_Simple_Literals();
        void NCC_with_Relational_Constraint();
        void NCC_Complex();
        void NCC_from_Backtrace();
        void RL_Variablization();
        void BUNCPS_0();
        void Prohibit_Fake_Instantiation_LTIs();
        void BUNCPS_1();
        void BUNCPS_2();
        void BUNCPS_3();
        void Maintain_Instantiation_Specific_Identity();
        void BUNCPS_4();
        void Justification_RC_not_Ungrounded_STIs();
        void BUNCPS_5();
        void BUNCPS_6_Four_Level();
        void Superstate_Identity_Opaque();
        void Simple_Constraint_Prop();
        void Literalization_of_NC_and_NCC();
//        void testChunk40();
//        void testChunk41();
//        void testChunk42();
//        void testChunk43();
//        void testChunk44();
//        void testChunk45();
//        void testChunk46();
//        void testChunk47();
//        void testChunk48();
//        void testChunk49();

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
        int sourced, excised, ignored;
        ignored = response.GetArgInt(sml::sml_Names::kParamIgnoredProductionCount, -1);
        if (ignored != expected_chunks)
        {
            sourced = response.GetArgInt(sml::sml_Names::kParamSourcedProductionCount, -1);
            excised = response.GetArgInt(sml::sml_Names::kParamExcisedProductionCount, -1);
            std::ostringstream outStringStream("");
            outStringStream << "--> Expected to ignore " << expected_chunks << ": Src = " << sourced << ", Exc = " << excised << ", Ign = " << ignored;
            throw CPPUnit_Assert_Failure(outStringStream.str());
        }
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

void ChunkTest::All_Test_Types()
{
/*
# Tests:
# - All relational test types with integers
# - Includes literal relational test and disjunction
# - RHS actions that are variablized
#-  RHS actions with literals that are the same symbols
#   as were variablized.
*/

    build_and_check_chunk("All_Test_Types.soar", 4, 1);
}

void ChunkTest::Ungrounded_Relational_Constraint()
{
    build_and_check_chunk("Ungrounded_Relational_Constraint.soar", 8, 1);
}

void ChunkTest::Vrblzd_Constraint_on_Ungrounded()
{
    build_and_check_chunk("Vrblzd_Constraint_on_Ungrounded.soar", 8, 1);
}

void ChunkTest::Literalization_with_Constraints()
{
    build_and_check_chunk("Literalization_with_Constraints.soar", 8, 1);
}

void ChunkTest::Conflated_Constants()
{
    build_and_check_chunk("Conflated_Constants.soar", 8, 1);
}

void ChunkTest::Superstate_Identity_Opaque()
{
    build_and_check_chunk("Superstate_Identity_Opaque.soar", 8, 1);
}

void ChunkTest::Ungrounded_in_BT_Constraint()
{
    build_and_check_chunk("Ungrounded_in_BT_Constraint.soar", 8, 2);
}

void ChunkTest::STI_Variablization()
{
    build_and_check_chunk("STI_Variablization.soar", 8, 1);
}

void ChunkTest::STI_Variablization_Same_Type()
{
    build_and_check_chunk("STI_Variablization_Same_Type.soar", 8, 1);
}

void ChunkTest::RHS_Unbound_Multivalue()
{
    build_and_check_chunk("chunk10.soar", 8, 2);
}

void ChunkTest::Rete_Bug_Deep_vs_Top()
{
    build_and_check_chunk("Rete_Bug_Deep_vs_Top.soar", 8, 1);
}

void ChunkTest::Rete_Bug_Deep_vs_Deep()
{
    build_and_check_chunk("Rete_Bug_Deep_vs_Deep.soar", 8, 1);
}

void ChunkTest::Ungrounded_STIs()
{
    build_and_check_chunk("Ungrounded_STIs.soar", 8, 1);
}

void ChunkTest::Ungrounded_Mixed()
{
    build_and_check_chunk("Ungrounded_Mixed.soar", 8, 1);
}

void ChunkTest::Ungrounded_STI_Promotion()
{
    build_and_check_chunk("Ungrounded_STI_Promotion.soar", 8, 1);
}

void ChunkTest::NC_with_RC_and_Local_Variable()
{
    build_and_check_chunk("NC_with_RC_and_Local_Variable.soar", 8, 1);
}

void ChunkTest::NCC_Simple_Literals()
{
    build_and_check_chunk("NCC_Simple_Literals.soar", 8, 1);
}

void ChunkTest::NC_Simple_No_Exist()
{
    build_and_check_chunk("NC_Simple_No_Exist.soar", 8, 1);
}

void ChunkTest::NC_with_Relational_Constraint()
{
    build_and_check_chunk("NC_with_Relational_Constraint.soar", 8, 1);
}

void ChunkTest::NCC_2_Conds_Simple_Literals()
{
    build_and_check_chunk("NCC_2_Conds_Simple_Literals.soar", 8, 1);
}

void ChunkTest::NCC_with_Relational_Constraint()
{
    build_and_check_chunk("NCC_with_Relational_Constraint.soar", 8, 1);
}

void ChunkTest::NCC_Complex()
{
    build_and_check_chunk("NCC_Complex.soar", 8, 1);
}

void ChunkTest::NCC_from_Backtrace()
{
    build_and_check_chunk("NCC_from_Backtrace.soar", 8, 1);
}

void ChunkTest::RL_Variablization()
{
    build_and_check_chunk("RL_Variablization.soar", 8, 5);
}

void ChunkTest::BUNCPS_0()
{
    build_and_check_chunk("BUNCPS_0.soar", 8, 1);
}

void ChunkTest::Prohibit_Fake_Instantiation_LTIs()
{
    build_and_check_chunk("Prohibit_Fake_Instantiation_LTIs.soar", 8, 1);
}

void ChunkTest::BUNCPS_1()
{
    build_and_check_chunk("BUNCPS_1.soar", 8, 1);
}

void ChunkTest::BUNCPS_2()
{
    build_and_check_chunk("BUNCPS_2.soar", 8, 1);
}

void ChunkTest::BUNCPS_3()
{
    build_and_check_chunk("BUNCPS_3.soar", 8, 1);
}

void ChunkTest::Maintain_Instantiation_Specific_Identity()
{
    build_and_check_chunk("Maintain_Instantiation_Specific_Identity.soar", 8, 1);
}

void ChunkTest::BUNCPS_4()
{
    build_and_check_chunk("BUNCPS_4.soar", 8, 1);
}

void ChunkTest::Justification_RC_not_Ungrounded_STIs()
{
    build_and_check_chunk("Justification_RC_not_Ungrounded_STIs.soar", 8, 1);
}

void ChunkTest::BUNCPS_5()
{
    build_and_check_chunk("BUNCPS_5.soar", 8, 1);
}

void ChunkTest::BUNCPS_6_Four_Level()
{
    build_and_check_chunk("BUNCPS_6_Four_Level.soar", 8, 2);
}

void ChunkTest::BUNCPS_7_with_Constraints()
{
    build_and_check_chunk("BUNCPS_7_with_Constraints.soar", 8, 1);
}

void ChunkTest::Simple_Literalization()
{
    /* Literalization and constraint maintenance */
    build_and_check_chunk("Simple_Literalization.soar", 8, 1);
}

void ChunkTest::Constraint_Prop_from_Base_Conds()
{
    /* Constraint maintenance from base conditions */
    build_and_check_chunk("Constraint_Prop_from_Base_Conds.soar", 8, 1);
}

void ChunkTest::Simple_Constraint_Prop()
{
    build_and_check_chunk("Simple_Constraint_Prop.soar", 8, 1);
}

void ChunkTest::Literalization_of_NC_and_NCC()
{
    build_and_check_chunk("Literalization_of_NC_and_NCC.soar", 8, 1);
}

//void ChunkTest::testChunk40()
//{
//    build_and_check_chunk("chunk40.soar", 8, 1);
//}
//
//void ChunkTest::testChunk41()
//{
//    build_and_check_chunk("chunk41.soar", 8, 1);
//}
//
//void ChunkTest::testChunk42()
//{
//    build_and_check_chunk("chunk42.soar", 8, 1);
//}
//
//void ChunkTest::testChunk43()
//{
//    build_and_check_chunk("chunk43.soar", 8, 1);
//}
//
//void ChunkTest::testChunk44()
//{
//    build_and_check_chunk("chunk44.soar", 8, 1);
//}
//
//void ChunkTest::testChunk45()
//{
//    build_and_check_chunk("chunk45.soar", 8, 1);
//}
//
//void ChunkTest::testChunk46()
//{
//    build_and_check_chunk("chunk46.soar", 8, 1);
//}
//
//void ChunkTest::testChunk47()
//{
//    build_and_check_chunk("chunk47.soar", 8, 1);
//}
//
//void ChunkTest::testChunk48()
//{
//    build_and_check_chunk("chunk48.soar", 8, 1);
//}
//
//void ChunkTest::testChunk49()
//{
//    build_and_check_chunk("chunk49.soar", 8, 1);
//}
