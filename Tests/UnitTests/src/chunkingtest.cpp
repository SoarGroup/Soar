#include "portability.h"

#include "unittest.h"

#include "handlers.h"
#include "sml_Events.h"
#include "soar_instance.h"


//IMPORTANT:  DON'T USE THE VARIABLE success.  It is declared globally in another test suite and we don't own it here.

class ChunkTest : public CPPUNIT_NS::TestCase
{
        CPPUNIT_TEST_SUITE(ChunkTest);   // The name of this class

#ifdef DO_CHUNKING_TESTS

        CPPUNIT_TEST(SMem_Chunked_Query);
        CPPUNIT_TEST(SMem_Chunked_Query2);
        CPPUNIT_TEST(SMem_Chunk_Direct);
        CPPUNIT_TEST(All_Test_Types);
        CPPUNIT_TEST(BUNCPS_0);  // BUNCPS = Bottom-up Non-Chunky Problem Spaces
        CPPUNIT_TEST(BUNCPS_1);  // (most came from problems found testing on Kirk's game learning agents)
        CPPUNIT_TEST(BUNCPS_2);
        CPPUNIT_TEST(BUNCPS_3);
        CPPUNIT_TEST(BUNCPS_4);
        CPPUNIT_TEST(BUNCPS_5);
        CPPUNIT_TEST(BUNCPS_6_Four_Level);
        CPPUNIT_TEST(BUNCPS_7_with_Constraints);
        CPPUNIT_TEST(Chunk_Operator_Tie_Impasse);
        CPPUNIT_TEST(Chunk_Operator_Tie_Item_Links);
        CPPUNIT_TEST(Chunk_RL_Proposal);
        CPPUNIT_TEST(Chunk_Superstate_Operator_Preference);
        CPPUNIT_TEST(Chunked_Justification_with_extras);
        CPPUNIT_TEST(Conflated_Constants);
        CPPUNIT_TEST(Constraint_Prop_from_Base_Conds);
        CPPUNIT_TEST(Demo_Blocks_World_Hierarchical);
        CPPUNIT_TEST(Demo_Blocks_World_Look_Ahead);
        CPPUNIT_TEST(Demo_Blocks_World_Look_Ahead_State_Evaluation);
        CPPUNIT_TEST(Demo_Eight_Puzzle);
        CPPUNIT_TEST(Demo_RL_Unit);
        CPPUNIT_TEST(Demo_Water_Jug_Hierarchy);
        CPPUNIT_TEST(Demo_Water_Jug_Look_Ahead);
        CPPUNIT_TEST(Demo_Water_Jug_Tie);
        CPPUNIT_TEST(Disjunction_Merge);
        CPPUNIT_TEST(Faux_Operator);
        CPPUNIT_TEST(Faux_Smem_Operator_RHS);
        CPPUNIT_TEST(Justification_RC_not_Ungrounded_STIs);
        CPPUNIT_TEST(Justifications_Get_New_Identities);
        CPPUNIT_TEST(Literalization_of_NC_and_NCC);
        CPPUNIT_TEST(Literalization_with_BT_Constraints);
        CPPUNIT_TEST(Literalization_with_BT_Constraints2);
        CPPUNIT_TEST(Literalization_with_Constraints);
        CPPUNIT_TEST(Maintain_Instantiation_Specific_Identity);
        CPPUNIT_TEST(NC_Disjunction);
        CPPUNIT_TEST(NC_Simple_No_Exist);
        CPPUNIT_TEST(NC_with_RC_and_Local_Variable);
        CPPUNIT_TEST(NC_with_Relational_Constraint);
        CPPUNIT_TEST(NCC_2_Conds_Simple_Literals);
        CPPUNIT_TEST(NCC_Complex);
        CPPUNIT_TEST(NCC_from_Backtrace);
        CPPUNIT_TEST(NCC_Simple_Literals);
        CPPUNIT_TEST(NCC_with_Relational_Constraint);
        CPPUNIT_TEST(No_Topstate_Match);
        CPPUNIT_TEST(Opaque_State_Barrier);
        CPPUNIT_TEST(Promoted_STI);
        CPPUNIT_TEST(Reorderer_Bad_Conjunction);
        CPPUNIT_TEST(Repair_Unconnected_RHS_ID);
        CPPUNIT_TEST(Repair_NOR_Temporal_Constraint);
        CPPUNIT_TEST(Result_On_Operator);
        CPPUNIT_TEST(Rete_Bug_Deep_vs_Deep);
        CPPUNIT_TEST(Rete_Bug_Deep_vs_Top);
        CPPUNIT_TEST(RHS_Math_Abs);
        CPPUNIT_TEST(RHS_Math_Mixed);
        CPPUNIT_TEST(RHS_Math);
        CPPUNIT_TEST(RHS_Unbound_Multivalue);
        CPPUNIT_TEST(RL_Variablization);
        CPPUNIT_TEST(Simple_Constraint_Prop);
        CPPUNIT_TEST(Simple_Literalization);
        CPPUNIT_TEST(STI_Variablization_Same_Type);
        CPPUNIT_TEST(STI_Variablization);
        CPPUNIT_TEST(STI_with_referents);
        CPPUNIT_TEST(Superstate_Identity_Opaque);
        CPPUNIT_TEST(testLearn);   // bug 1145
        CPPUNIT_TEST(Ungrounded_in_BT_Constraint);
        CPPUNIT_TEST(Ungrounded_Mixed);
        CPPUNIT_TEST(Ungrounded_Relational_Constraint);
        CPPUNIT_TEST(Ungrounded_STI_Promotion);
        CPPUNIT_TEST(Ungrounded_STIs);
        CPPUNIT_TEST(Unify_Ambiguous_Output);
        CPPUNIT_TEST(Unify_Children_Results);
        CPPUNIT_TEST(Unify_through_Two_Traces_Four_Deep);
        CPPUNIT_TEST(Vrblzd_Constraint_on_Ungrounded);
        #ifndef SKIP_SLOW_TESTS
                CPPUNIT_TEST(Demo_Arithmetic);
        #endif

#endif
        CPPUNIT_TEST_SUITE_END();

    public:
        void setUp();       // Called before each function outlined by CPPUNIT_TEST
        void tearDown();    // Called after each function outlined by CPPUNIT_TEST

    protected:

        void source(const std::string& path);
        void build_and_check_chunk(const std::string& path, int64_t decisions, int64_t expected_chunks);
        void build_and_check_chunk_clean(const std::string& path, int64_t decisions, int64_t expected_chunks);
        void STI_Variablization();
        void STI_Variablization_Same_Type();
        void All_Test_Types();
        void Conflated_Constants();
        void Chunk_Operator_Tie_Impasse();
        void Chunk_Operator_Tie_Item_Links();
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
        void Literalization_with_BT_Constraints();
        void Literalization_with_BT_Constraints2();
        void Unify_through_Two_Traces_Four_Deep();
        void STI_with_referents();
        void Chunked_Justification_with_extras();
        void No_Topstate_Match();
        void Repair_NOR_Temporal_Constraint();
        void RHS_Math();
        void Repair_Unconnected_RHS_ID();
        void Promoted_STI();
        void Chunk_RL_Proposal();
        void NC_Disjunction();
        void RHS_Math_Mixed();
        void RHS_Math_Abs();
        void Reorderer_Bad_Conjunction();
        void Opaque_State_Barrier();
        void Unify_Ambiguous_Output();
        void Faux_Smem_Operator_RHS();
        void Faux_Operator();
        void SMem_Chunk_Direct();
        void SMem_Chunked_Query();
        void Result_On_Operator();
        void Unify_Children_Results();
        void Demo_Blocks_World_Hierarchical();
        void testLearn();
        void SMem_Chunked_Query2();
        void Demo_Arithmetic();
        void Demo_Blocks_World_Look_Ahead();
        void Demo_Blocks_World_Look_Ahead_State_Evaluation();
        void Demo_Eight_Puzzle();
        void Demo_RL_Unit();
        void Demo_Water_Jug_Hierarchy();
        void Demo_Water_Jug_Look_Ahead();
        void Demo_Water_Jug_Tie();
        void Disjunction_Merge();
        void Chunk_Superstate_Operator_Preference();
        void Justifications_Get_New_Identities();

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
#ifdef TURN_EXPLAINER_ON
    {
        sml::ClientAnalyzedXML response;
        pAgent->ExecuteCommandLineXML("explain all on", &response);
        CPPUNIT_ASSERT_MESSAGE(response.GetResultString(), pAgent->GetLastCommandLineResult());
    }
    {
        sml::ClientAnalyzedXML response;
        pAgent->ExecuteCommandLineXML("explain just on", &response);
        CPPUNIT_ASSERT_MESSAGE(response.GetResultString(), pAgent->GetLastCommandLineResult());
    }
#endif
    pAgent->RunSelf(decisions, sml::sml_DECISION);
    CPPUNIT_ASSERT_MESSAGE(pAgent->GetLastErrorDescription(), pAgent->GetLastCommandLineResult());
    //    CPPUNIT_ASSERT(succeeded);

    {
        sml::ClientAnalyzedXML response;
        pAgent->ExecuteCommandLineXML((std::string("source test_agents/chunking-tests/expected/") + path).c_str(), &response);
        CPPUNIT_ASSERT_MESSAGE(response.GetResultString(), pAgent->GetLastCommandLineResult());

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
#ifdef INIT_AFTER_RUN
    {
        sml::ClientAnalyzedXML response;
        pAgent->ExecuteCommandLineXML("soar init", &response);
        CPPUNIT_ASSERT_MESSAGE(response.GetResultString(), pAgent->GetLastCommandLineResult());
    }
    {
        sml::ClientAnalyzedXML response;
        pAgent->ExecuteCommandLineXML("run 100", &response);
        CPPUNIT_ASSERT_MESSAGE(response.GetResultString(), pAgent->GetLastCommandLineResult());
    }
    {
        sml::ClientAnalyzedXML response;
        pAgent->ExecuteCommandLineXML("soar init", &response);
        CPPUNIT_ASSERT_MESSAGE(response.GetResultString(), pAgent->GetLastCommandLineResult());
    }
#endif
}

void ChunkTest::build_and_check_chunk_clean(const std::string& path, int64_t decisions, int64_t expected_chunks)
{
    source(path.c_str());
    pAgent->RunSelf(decisions, sml::sml_DECISION);
    CPPUNIT_ASSERT_MESSAGE(pAgent->GetLastErrorDescription(), pAgent->GetLastCommandLineResult());
    {
        sml::ClientAnalyzedXML response;
        pAgent->ExecuteCommandLineXML("output command-to-file unit_test_chunks.soar print -fc", &response);
        CPPUNIT_ASSERT_MESSAGE(response.GetResultString(), pAgent->GetLastCommandLineResult());
    }
    tearDown();
    setUp();
    {
        sml::ClientAnalyzedXML response;
        pAgent->ExecuteCommandLineXML("source unit_test_chunks.soar", &response);
        CPPUNIT_ASSERT_MESSAGE(response.GetResultString(), pAgent->GetLastCommandLineResult());
    }
    {
        sml::ClientAnalyzedXML response;
        pAgent->ExecuteCommandLineXML((std::string("source test_agents/chunking-tests/expected/") + path).c_str(), &response);
        CPPUNIT_ASSERT_MESSAGE(response.GetResultString(), pAgent->GetLastCommandLineResult());

        int sourced, excised, ignored;
        ignored = response.GetArgInt(sml::sml_Names::kParamIgnoredProductionCount, -1);
        if (ignored != expected_chunks)
        {
            sourced = response.GetArgInt(sml::sml_Names::kParamSourcedProductionCount, -1);
            excised = response.GetArgInt(sml::sml_Names::kParamExcisedProductionCount, -1);
            std::ostringstream outStringStream("");
            outStringStream << "--> Expected to ignore " << expected_chunks << ": Src = " << sourced << ", Exc = " << excised << ", Ign = " << ignored;
//            std::cout << outStringStream.str() << "\n";
//            sml::ClientAnalyzedXML response2;
//            pAgent->ExecuteCommandLineXML("output command-to-file failed.soar print -f", &response2);
//            pAgent->ExecuteCommandLineXML("saverules", &response2);

            throw CPPUnit_Assert_Failure(outStringStream.str());
        }
    }
    #ifdef INIT_AFTER_RUN
    {
        sml::ClientAnalyzedXML response;
        pAgent->ExecuteCommandLineXML("soar init", &response);
        CPPUNIT_ASSERT_MESSAGE(response.GetResultString(), pAgent->GetLastCommandLineResult());
    }
    {
        sml::ClientAnalyzedXML response;
        pAgent->ExecuteCommandLineXML("run 100", &response);
        CPPUNIT_ASSERT_MESSAGE(response.GetResultString(), pAgent->GetLastCommandLineResult());
    }
    {
        sml::ClientAnalyzedXML response;
        pAgent->ExecuteCommandLineXML("soar init", &response);
        CPPUNIT_ASSERT_MESSAGE(response.GetResultString(), pAgent->GetLastCommandLineResult());
    }
    #endif
}

void ChunkTest::setUp()
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

    /* The following may not be necessary.  May have been something I was using to print
     * some debug trace messages while the unit tests were running.   */
    #ifndef CONFIGURE_SOAR_FOR_UNIT_TESTS
    {
        sml::ClientAnalyzedXML response;
        pAgent->ExecuteCommandLineXML("output console off", &response);
        CPPUNIT_ASSERT_MESSAGE(response.GetResultString(), pAgent->GetLastCommandLineResult());
    }
    {
        sml::ClientAnalyzedXML response;
        pAgent->ExecuteCommandLineXML("output callbacks on", &response);
        CPPUNIT_ASSERT_MESSAGE(response.GetResultString(), pAgent->GetLastCommandLineResult());
    }
    #endif

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


void ChunkTest::testLearn()
{
    source("testLearn.soar");
    pAgent->ExecuteCommandLine("chunk except");
    pKernel->RunAllAgentsForever();
    {
        sml::ClientAnalyzedXML response;
        pAgent->ExecuteCommandLineXML("stats", &response);
        CPPUNIT_ASSERT(response.GetArgInt(sml::sml_Names::kParamStatsCycleCountDecision, -1) == 3);
        CPPUNIT_ASSERT(response.GetArgInt(sml::sml_Names::kParamStatsCycleCountElaboration, -1) == 5);
    }

    // learning is off, same behavior expected
    pAgent->ExecuteCommandLine("init");
    pKernel->RunAllAgentsForever();
    {
        sml::ClientAnalyzedXML response;
        pAgent->ExecuteCommandLineXML("stats", &response);
        CPPUNIT_ASSERT(response.GetArgInt(sml::sml_Names::kParamStatsCycleCountDecision, -1) == 3);
        CPPUNIT_ASSERT(response.GetArgInt(sml::sml_Names::kParamStatsCycleCountElaboration, -1) == 5);
    }

    // turn learn except on
    pAgent->ExecuteCommandLine("init");
    pAgent->ExecuteCommandLine("chunk except");
    pKernel->RunAllAgentsForever();
    {
        sml::ClientAnalyzedXML response;
        pAgent->ExecuteCommandLineXML("stats", &response);
        CPPUNIT_ASSERT(response.GetArgInt(sml::sml_Names::kParamStatsCycleCountDecision, -1) == 3);
        CPPUNIT_ASSERT(response.GetArgInt(sml::sml_Names::kParamStatsCycleCountElaboration, -1) == 5);
    }

    // don't learn is active so same result expected
    pAgent->ExecuteCommandLine("init");
    pKernel->RunAllAgentsForever();
    {
        sml::ClientAnalyzedXML response;
        pAgent->ExecuteCommandLineXML("stats", &response);
        CPPUNIT_ASSERT(response.GetArgInt(sml::sml_Names::kParamStatsCycleCountDecision, -1) == 3);
        CPPUNIT_ASSERT(response.GetArgInt(sml::sml_Names::kParamStatsCycleCountElaboration, -1) == 5);
    }

    // get rid of dont learn
    pAgent->ExecuteCommandLine("init");
    pAgent->ExecuteCommandLine("excise dont*learn");
    pKernel->RunAllAgentsForever();
    {
        sml::ClientAnalyzedXML response;
        pAgent->ExecuteCommandLineXML("stats", &response);
        CPPUNIT_ASSERT(response.GetArgInt(sml::sml_Names::kParamStatsCycleCountDecision, -1) == 3);
        CPPUNIT_ASSERT(response.GetArgInt(sml::sml_Names::kParamStatsCycleCountElaboration, -1) == 5);
    }

    // expect improvement
    pAgent->ExecuteCommandLine("init");
    pKernel->RunAllAgentsForever();
    {
        sml::ClientAnalyzedXML response;
        pAgent->ExecuteCommandLineXML("stats", &response);
        CPPUNIT_ASSERT(response.GetArgInt(sml::sml_Names::kParamStatsCycleCountDecision, -1) == 1);
        CPPUNIT_ASSERT(response.GetArgInt(sml::sml_Names::kParamStatsCycleCountElaboration, -1) == 3);
    }

    // go to only mode
    pAgent->ExecuteCommandLine("init");
    pAgent->ExecuteCommandLine("excise -c");
    pAgent->ExecuteCommandLine("chunk only");
    pKernel->RunAllAgentsForever();
    {
        sml::ClientAnalyzedXML response;
        pAgent->ExecuteCommandLineXML("stats", &response);
        CPPUNIT_ASSERT(response.GetArgInt(sml::sml_Names::kParamStatsCycleCountDecision, -1) == 3);
        CPPUNIT_ASSERT(response.GetArgInt(sml::sml_Names::kParamStatsCycleCountElaboration, -1) == 5);
    }

    // force learn is active, expect improvement
    pAgent->ExecuteCommandLine("init");
    pKernel->RunAllAgentsForever();
    {
        sml::ClientAnalyzedXML response;
        pAgent->ExecuteCommandLineXML("stats", &response);
        CPPUNIT_ASSERT(response.GetArgInt(sml::sml_Names::kParamStatsCycleCountDecision, -1) == 1);
        CPPUNIT_ASSERT(response.GetArgInt(sml::sml_Names::kParamStatsCycleCountElaboration, -1) == 3);
    }

    // get rid of chunk and force learn
    pAgent->ExecuteCommandLine("init");
    pAgent->ExecuteCommandLine("excise -c");
    pAgent->ExecuteCommandLine("excise force*learn");
    pKernel->RunAllAgentsForever();
    {
        sml::ClientAnalyzedXML response;
        pAgent->ExecuteCommandLineXML("stats", &response);
        CPPUNIT_ASSERT(response.GetArgInt(sml::sml_Names::kParamStatsCycleCountDecision, -1) == 3);
        CPPUNIT_ASSERT(response.GetArgInt(sml::sml_Names::kParamStatsCycleCountElaboration, -1) == 5);
    }

    // expect no improvement
    pAgent->ExecuteCommandLine("init");
    pKernel->RunAllAgentsForever();
    {
        sml::ClientAnalyzedXML response;
        pAgent->ExecuteCommandLineXML("stats", &response);
        CPPUNIT_ASSERT(response.GetArgInt(sml::sml_Names::kParamStatsCycleCountDecision, -1) == 3);
        CPPUNIT_ASSERT(response.GetArgInt(sml::sml_Names::kParamStatsCycleCountElaboration, -1) == 5);
    }
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
    build_and_check_chunk("RHS_Unbound_Multivalue.soar", 8, 1);
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

void ChunkTest::Literalization_with_BT_Constraints()
{
    build_and_check_chunk("Literalization_with_BT_Constraints.soar", 8, 1);
}

void ChunkTest::Literalization_with_BT_Constraints2()
{
    build_and_check_chunk("Literalization_with_BT_Constraints2.soar", 8, 2);
}

void ChunkTest::Unify_through_Two_Traces_Four_Deep()
{
    build_and_check_chunk_clean("Unify_through_Two_Traces_Four_Deep.soar", 8, 3);
}

void ChunkTest::STI_with_referents()
{
    build_and_check_chunk("STI_with_referents.soar", 8, 1);
}
void ChunkTest::Chunked_Justification_with_extras()
{
    build_and_check_chunk("STI_with_referents.soar", 8, 1);
}
void ChunkTest::No_Topstate_Match()
{
    build_and_check_chunk("No_Topstate_Match.soar", 8, 1);
}

void ChunkTest::Repair_NOR_Temporal_Constraint()
{
    // Change to 2 chunks expected after rule repair fixed
    build_and_check_chunk("Repair_NOR_Temporal_Constraint.soar", 8, 3);
}

void ChunkTest::RHS_Math()
{
    build_and_check_chunk("RHS_Math.soar", 8, 2);
}

void ChunkTest::Repair_Unconnected_RHS_ID()
{
    build_and_check_chunk("Repair_Unconnected_RHS_ID.soar", 8, 2);
}

void ChunkTest::Promoted_STI()
{
    build_and_check_chunk("Promoted_STI.soar", 8, 1);
}

void ChunkTest::Chunk_RL_Proposal()
{
    build_and_check_chunk("Chunk_RL_Proposal.soar", 8, 1);
}
void ChunkTest::NC_Disjunction()
{
    build_and_check_chunk("NC_Disjunction.soar", 8, 1);
}
void ChunkTest::RHS_Math_Mixed()
{
    build_and_check_chunk("RHS_Math_Mixed.soar", 8, 4);
}
void ChunkTest::RHS_Math_Abs()
{
    build_and_check_chunk("RHS_Math_Abs.soar", 8, 2);
}
void ChunkTest::Reorderer_Bad_Conjunction()
{
    build_and_check_chunk("Reorderer_Bad_Conjunction.soar", 8, 1);
}
void ChunkTest::Opaque_State_Barrier()
{
    build_and_check_chunk("Opaque_State_Barrier.soar", 8, 1);
}
void ChunkTest::Unify_Ambiguous_Output()
{
    build_and_check_chunk("Unify_Ambiguous_Output.soar", 8, 1);
}
void ChunkTest::Faux_Smem_Operator_RHS()
{
    build_and_check_chunk("Faux_Smem_Operator_RHS.soar", 8, 1);
}
void ChunkTest::Faux_Operator()
{
    build_and_check_chunk("Faux_Operator.soar", 8, 3);
}
void ChunkTest::SMem_Chunk_Direct()
{
    build_and_check_chunk_clean("SMem_Chunk_Direct.soar", 8, 1);
}
void ChunkTest::SMem_Chunked_Query()
{
    build_and_check_chunk_clean("SMem_Chunked_Query.soar", 8, 1);
}
void ChunkTest::Result_On_Operator()
{
    build_and_check_chunk("Result_On_Operator.soar", 8, 1);
}
void ChunkTest::Unify_Children_Results()
{
    build_and_check_chunk("Unify_Children_Results.soar", 8, 1);
}
void ChunkTest::Demo_Blocks_World_Hierarchical()
{
    build_and_check_chunk("Demo_Blocks_World_Hierarchical.soar", 23, 16);
}
void ChunkTest::Chunk_Operator_Tie_Impasse()
{
    build_and_check_chunk("Chunk_Operator_Tie_Impasse.soar", 6, 2);
}
void ChunkTest::Chunk_Operator_Tie_Item_Links()
{
    build_and_check_chunk("Chunk_Operator_Tie_Item_Links.soar", 6, 1);
}
void ChunkTest::SMem_Chunked_Query2()
{
    build_and_check_chunk_clean("SMem_Chunked_Query2.soar", 8, 1);
}

void ChunkTest::Demo_Arithmetic()
{
    build_and_check_chunk("Demo_Arithmetic.soar", 41424, 1);
}

void ChunkTest::Demo_Blocks_World_Look_Ahead()
{
    build_and_check_chunk_clean("Demo_Blocks_World_Look_Ahead.soar", 37, 6);
}

void ChunkTest::Demo_Blocks_World_Look_Ahead_State_Evaluation()
{
    build_and_check_chunk_clean("Demo_Blocks_World_Look_Ahead_State_Evaluation.soar", 37, 14);
}

void ChunkTest::Demo_Eight_Puzzle()
{
    build_and_check_chunk_clean("Demo_Eight_Puzzle.soar", 20, 6);
}

void ChunkTest::Demo_RL_Unit()
{
    build_and_check_chunk("Demo_RL_Unit.soar", 26, 6);
}

void ChunkTest::Demo_Water_Jug_Hierarchy()
{
    build_and_check_chunk("Demo_Water_Jug_Hierarchy.soar", 600, 3);
}

void ChunkTest::Demo_Water_Jug_Look_Ahead()
{
    build_and_check_chunk_clean("Demo_Water_Jug_Look_Ahead.soar", 600, 16);
}

void ChunkTest::Demo_Water_Jug_Tie()
{
    build_and_check_chunk("Demo_Water_Jug_Tie.soar", 600, 5);
}

void ChunkTest::Chunk_Superstate_Operator_Preference()
{
    build_and_check_chunk_clean("Chunk_Superstate_Operator_Preference.soar", 3, 1);
}

void ChunkTest::Disjunction_Merge()
{
    build_and_check_chunk_clean("Disjunction_Merge.soar", 5, 1);
}

void ChunkTest::Justifications_Get_New_Identities()
{
    build_and_check_chunk_clean("Justifications_Get_New_Identities.soar", 4, 1);
}
