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

        /* This agent is J Kirk's game learning agent.  It causes a bad crash around dc 346.  It seems to be
         * variablizing a list of results that is at least 400 elements long. */
        // CPPUNIT_TEST(GamesAgent_Sanity1);

//        CPPUNIT_TEST(SMem_Chunked_Query);
//        CPPUNIT_TEST(SMem_Chunked_Query2);
//        CPPUNIT_TEST(SMem_Chunk_Direct);
//        CPPUNIT_TEST(All_Test_Types);
//        CPPUNIT_TEST(BUNCPS_0);  // BUNCPS = Bottom-up Non-Chunky Problem Spaces
//        CPPUNIT_TEST(BUNCPS_1);  // (most came from problems found testing on Kirk's game learning agents)
//        CPPUNIT_TEST(BUNCPS_2);
//        CPPUNIT_TEST(BUNCPS_3);
//        CPPUNIT_TEST(BUNCPS_4);
//        CPPUNIT_TEST(BUNCPS_5);
//        CPPUNIT_TEST(BUNCPS_6_Four_Level);
//        CPPUNIT_TEST(BUNCPS_7_with_Constraints);
//        CPPUNIT_TEST(Chunk_Operator_Tie_Impasse);
//        CPPUNIT_TEST(Chunk_Operator_Tie_Item_Links);
//        CPPUNIT_TEST(Chunk_RL_Proposal);
//        CPPUNIT_TEST(Chunk_Superstate_Operator_Preference);
//        CPPUNIT_TEST(Chunked_Justification_with_extras);
//        CPPUNIT_TEST(Conflated_Constants);
//        CPPUNIT_TEST(Constraint_Prop_from_Base_Conds);
//        #ifndef SKIP_SLOW_TESTS
//            CPPUNIT_TEST(Demo_Arithmetic);
//        #endif
//        CPPUNIT_TEST(Demo_Blocks_World_Hierarchical);
//        CPPUNIT_TEST(Demo_Blocks_World_Hierarchical_Look_Ahead);
//        CPPUNIT_TEST(Demo_Blocks_World_Look_Ahead);
//        CPPUNIT_TEST(Demo_Blocks_World_Look_Ahead_State_Evaluation);
//        CPPUNIT_TEST(Demo_Blocks_World_Operator_Subgoaling);
//        CPPUNIT_TEST(Demo_Eight_Puzzle);
//        CPPUNIT_TEST(Demo_MaC_Planning);
//        CPPUNIT_TEST(Demo_RL_Unit);
//        CPPUNIT_TEST(Demo_ToH_Recursive);
//        CPPUNIT_TEST(Demo_Water_Jug_Hierarchy);
//        CPPUNIT_TEST(Demo_Water_Jug_Look_Ahead);
//        CPPUNIT_TEST(Demo_Water_Jug_Tie);
//        CPPUNIT_TEST(Disjunction_Merge);
//        CPPUNIT_TEST(Duplicates);
//        CPPUNIT_TEST(Faux_Operator);
        CPPUNIT_TEST(Faux_Smem_Operator_RHS);
//        CPPUNIT_TEST(Justification_RC_not_Ungrounded_STIs);
//        CPPUNIT_TEST(Justifications_Get_New_Identities);
//        CPPUNIT_TEST(Link_STM_to_LTM);
//        CPPUNIT_TEST(Literalization_of_NC_and_NCC);
//        CPPUNIT_TEST(Literalization_with_BT_Constraints);
//        CPPUNIT_TEST(Literalization_with_BT_Constraints2);
//        CPPUNIT_TEST(Literalization_with_Constraints);
//        CPPUNIT_TEST(Maintain_Instantiation_Specific_Identity);
//        CPPUNIT_TEST(NC_Disjunction);
//        CPPUNIT_TEST(NC_Simple_No_Exist);
//        CPPUNIT_TEST(NC_with_RC_and_Local_Variable);
//        CPPUNIT_TEST(NC_with_Relational_Constraint);
//        CPPUNIT_TEST(NCC_2_Conds_Simple_Literals);
//        CPPUNIT_TEST(NCC_Complex);
//        CPPUNIT_TEST(NCC_from_Backtrace);
//        CPPUNIT_TEST(NCC_Simple_Literals);
//        CPPUNIT_TEST(NCC_with_Relational_Constraint);
//        CPPUNIT_TEST(No_Topstate_Match);
//        CPPUNIT_TEST(Opaque_State_Barrier);
//        CPPUNIT_TEST(PRIMS_Sanity1);
//        CPPUNIT_TEST(PRIMS_Sanity2);
//        CPPUNIT_TEST(Promoted_STI);
//        CPPUNIT_TEST(Reorderer_Bad_Conjunction);
//        CPPUNIT_TEST(Repair_Unconnected_RHS_ID);
//        CPPUNIT_TEST(Repair_NOR_Temporal_Constraint);
//        CPPUNIT_TEST(Result_On_Operator);
//        CPPUNIT_TEST(Rete_Bug_Deep_vs_Deep);
//        CPPUNIT_TEST(Rete_Bug_Deep_vs_Top);
//        CPPUNIT_TEST(RHS_Math_Abs);
//        CPPUNIT_TEST(RHS_Math_Mixed);
//        CPPUNIT_TEST(RHS_Math);
//        CPPUNIT_TEST(RHS_Unbound_Multivalue);
//        CPPUNIT_TEST(RL_Variablization);
//        CPPUNIT_TEST(Simple_Constraint_Prop);
//        CPPUNIT_TEST(Simple_Literalization);
//        CPPUNIT_TEST(STI_Variablization_Same_Type);
//        CPPUNIT_TEST(STI_Variablization);
//        CPPUNIT_TEST(STI_with_referents);
//        CPPUNIT_TEST(Superstate_Identity_Opaque);
//        CPPUNIT_TEST(Chunk_All_Only_Except);   // bug 1145
//        CPPUNIT_TEST(Ungrounded_in_BT_Constraint);
//        CPPUNIT_TEST(Ungrounded_Mixed);
//        CPPUNIT_TEST(Ungrounded_Relational_Constraint);
//        CPPUNIT_TEST(Ungrounded_STI_Promotion);
//        CPPUNIT_TEST(Ungrounded_STIs);
//        CPPUNIT_TEST(Unify_Ambiguous_Output);
//        CPPUNIT_TEST(Unify_Children_Results);
//        CPPUNIT_TEST(Unify_through_Two_Traces_Four_Deep);
//        CPPUNIT_TEST(Vrblzd_Constraint_on_Ungrounded);

#endif
        CPPUNIT_TEST_SUITE_END();

    public:
        void setUp();       // Called before each function outlined by CPPUNIT_TEST
        void tearDown();    // Called after each function outlined by CPPUNIT_TEST

    protected:

        void source(const std::string& path);
        void check_chunk(const std::string& path, int64_t decisions, int64_t expected_chunks, bool directSourceChunks = false);
        void agent_command(const char* pCmd);
        void start_log(const char* path);
        void continue_log(const char* path);
        void close_log();
        void save_chunks(const char* path);
        void source_saved_chunks(const char* path);
        void Chunk_All_Only_Except();

        void All_Test_Types()                                  { check_chunk("All_Test_Types.soar", 4, 1); }
        void BUNCPS_0()                                        { check_chunk("BUNCPS_0.soar", 8, 1); }
        void BUNCPS_1()                                        { check_chunk("BUNCPS_1.soar", 8, 1); }
        void BUNCPS_2()                                        { check_chunk("BUNCPS_2.soar", 8, 1); }
        void BUNCPS_3()                                        { check_chunk("BUNCPS_3.soar", 8, 1); }
        void BUNCPS_4()                                        { check_chunk("BUNCPS_4.soar", 8, 1); }
        void BUNCPS_5()                                        { check_chunk("BUNCPS_5.soar", 8, 1); }
        void BUNCPS_6_Four_Level()                             { check_chunk("BUNCPS_6_Four_Level.soar", 8, 2); }
        void BUNCPS_7_with_Constraints()                       { check_chunk("BUNCPS_7_with_Constraints.soar", 8, 1); }
        void Chunk_Operator_Tie_Impasse()                      { check_chunk("Chunk_Operator_Tie_Impasse.soar", 6, 2); }
        void Chunk_Operator_Tie_Item_Links()                   { check_chunk("Chunk_Operator_Tie_Item_Links.soar", 6, 1); }
        void Chunk_RL_Proposal()                               { check_chunk("Chunk_RL_Proposal.soar", 8, 2); }
        void Chunk_Superstate_Operator_Preference()            { check_chunk("Chunk_Superstate_Operator_Preference.soar", 3, 1); }
        void Chunked_Justification_with_extras()               { check_chunk("STI_with_referents.soar", 8, 1); }
        void Conflated_Constants()                             { check_chunk("Conflated_Constants.soar", 8, 1); }
        void Constraint_Prop_from_Base_Conds()                 { check_chunk("Constraint_Prop_from_Base_Conds.soar", 8, 1); }
        void Demo_Arithmetic()                                 { check_chunk("Demo_Arithmetic.soar", 41424, 1); }
        void Demo_Blocks_World_Hierarchical_Look_Ahead()       { check_chunk("Demo_Blocks_World_Hierarchical_Look_Ahead.soar", 70, 4); }
        void Demo_Blocks_World_Hierarchical()                  { check_chunk("Demo_Blocks_World_Hierarchical.soar", 23, 16); }
        void Demo_Blocks_World_Look_Ahead_State_Evaluation()   { check_chunk("Demo_Blocks_World_Look_Ahead_State_Evaluation.soar", 37, 14); }
        void Demo_Blocks_World_Look_Ahead()                    { check_chunk("Demo_Blocks_World_Look_Ahead.soar", 37, 6); }
        void Demo_Blocks_World_Operator_Subgoaling()           { check_chunk("Demo_Blocks_World_Operator_Subgoaling.soar", 6, 1); }
        void Demo_Eight_Puzzle()                               { check_chunk("Demo_Eight_Puzzle.soar", 20, 6); }
        void Demo_MaC_Planning()                               { check_chunk("Demo_MaC_Planning.soar", 138, 58); }
        void Demo_RL_Unit()                                    { check_chunk("Demo_RL_Unit.soar", 26, 6); }
        void Demo_ToH_Recursive()                              { check_chunk("Demo_ToH_Recursive.soar", 23, 10); }
        void Demo_Water_Jug_Hierarchy()                        { check_chunk("Demo_Water_Jug_Hierarchy.soar", 600, 3); }
        void Demo_Water_Jug_Look_Ahead()                       { check_chunk("Demo_Water_Jug_Look_Ahead.soar", 600, 16); }
        void Demo_Water_Jug_Tie()                              { check_chunk("Demo_Water_Jug_Tie.soar", 600, 5); }
        void Disjunction_Merge()                               { check_chunk("Disjunction_Merge.soar", 5, 1); }
        void Duplicates()                                      { check_chunk("Duplicates.soar", 5, 2); }
        void Faux_Operator()                                   { check_chunk("Faux_Operator.soar", 8, 3); }
        void Faux_Smem_Operator_RHS()                          { check_chunk("Faux_Smem_Operator_RHS.soar", 8, 1, true); }
        void GamesAgent_Sanity1()                              { check_chunk("GamesAgent_Sanity1.soar", 4539, 14); }
        void Justification_RC_not_Ungrounded_STIs()            { check_chunk("Justification_RC_not_Ungrounded_STIs.soar", 8, 1); }
        void Justifications_Get_New_Identities()               { check_chunk("Justifications_Get_New_Identities.soar", 4, 1); }
        void Link_STM_to_LTM()                                 { check_chunk("Link_STM_to_LTM.soar", 6, 2); }
        void Literalization_of_NC_and_NCC()                    { check_chunk("Literalization_of_NC_and_NCC.soar", 8, 1); }
        void Literalization_with_BT_Constraints()              { check_chunk("Literalization_with_BT_Constraints.soar", 8, 1); }
        void Literalization_with_BT_Constraints2()             { check_chunk("Literalization_with_BT_Constraints2.soar", 8, 2); }
        void Literalization_with_Constraints()                 { check_chunk("Literalization_with_Constraints.soar", 8, 1); }
        void Maintain_Instantiation_Specific_Identity()        { check_chunk("Maintain_Instantiation_Specific_Identity.soar", 8, 1); }
        void NC_Disjunction()                                  { check_chunk("NC_Disjunction.soar", 8, 1); }
        void NC_Simple_No_Exist()                              { check_chunk("NC_Simple_No_Exist.soar", 8, 1); }
        void NC_with_RC_and_Local_Variable()                   { check_chunk("NC_with_RC_and_Local_Variable.soar", 8, 1); }
        void NC_with_Relational_Constraint()                   { check_chunk("NC_with_Relational_Constraint.soar", 8, 1); }
        void NCC_2_Conds_Simple_Literals()                     { check_chunk("NCC_2_Conds_Simple_Literals.soar", 8, 1); }
        void NCC_Complex()                                     { check_chunk("NCC_Complex.soar", 8, 1); }
        void NCC_from_Backtrace()                              { check_chunk("NCC_from_Backtrace.soar", 8, 1); }
        void NCC_Simple_Literals()                             { check_chunk("NCC_Simple_Literals.soar", 8, 1); }
        void NCC_with_Relational_Constraint()                  { check_chunk("NCC_with_Relational_Constraint.soar", 8, 1); }
        void No_Topstate_Match()                               { check_chunk("No_Topstate_Match.soar", 8, 1); }
        void Opaque_State_Barrier()                            { check_chunk("Opaque_State_Barrier.soar", 8, 1); }
        void PRIMS_Sanity1()                                   { check_chunk("PRIMS_Sanity1.soar", 795, 23); }
        void PRIMS_Sanity2()                                   { check_chunk("PRIMS_Sanity2.soar", 728, 22); }
        void Promoted_STI()                                    { check_chunk("Promoted_STI.soar", 8, 1); }
        void Reorderer_Bad_Conjunction()                       { check_chunk("Reorderer_Bad_Conjunction.soar", 8, 1); }
        void Repair_NOR_Temporal_Constraint()                  { check_chunk("Repair_NOR_Temporal_Constraint.soar", 8, 3); }
        void Repair_Unconnected_RHS_ID()                       { check_chunk("Repair_Unconnected_RHS_ID.soar", 8, 2); }
        void Result_On_Operator()                              { check_chunk("Result_On_Operator.soar", 8, 1); }
        void Rete_Bug_Deep_vs_Deep()                           { check_chunk("Rete_Bug_Deep_vs_Deep.soar", 8, 1); }
        void Rete_Bug_Deep_vs_Top()                            { check_chunk("Rete_Bug_Deep_vs_Top.soar", 8, 1); }
        void RHS_Math_Abs()                                    { check_chunk("RHS_Math_Abs.soar", 8, 2); }
        void RHS_Math_Mixed()                                  { check_chunk("RHS_Math_Mixed.soar", 8, 4); }
        void RHS_Math()                                        { check_chunk("RHS_Math.soar", 8, 1); }
        void RHS_Unbound_Multivalue()                          { check_chunk("RHS_Unbound_Multivalue.soar", 8, 1); }
        void RL_Variablization()                               { check_chunk("RL_Variablization.soar", 8, 5); }
        void Simple_Constraint_Prop()                          { check_chunk("Simple_Constraint_Prop.soar", 8, 1); }
        void Simple_Literalization()                           { check_chunk("Simple_Literalization.soar", 8, 1); }
        void SMem_Chunk_Direct()                               { check_chunk("SMem_Chunk_Direct.soar", 8, 1); }
        void SMem_Chunked_Query()                              { check_chunk("SMem_Chunked_Query.soar", 8, 1); }
        void SMem_Chunked_Query2()                             { check_chunk("SMem_Chunked_Query2.soar", 8, 1); }
        void STI_Variablization_Same_Type()                    { check_chunk("STI_Variablization_Same_Type.soar", 8, 1); }
        void STI_Variablization()                              { check_chunk("STI_Variablization.soar", 8, 1); }
        void STI_with_referents()                              { check_chunk("STI_with_referents.soar", 8, 1); }
        void Superstate_Identity_Opaque()                      { check_chunk("Superstate_Identity_Opaque.soar", 8, 1); }
        void Ungrounded_in_BT_Constraint()                     { check_chunk("Ungrounded_in_BT_Constraint.soar", 8, 2); }
        void Ungrounded_Mixed()                                { check_chunk("Ungrounded_Mixed.soar", 8, 1); }
        void Ungrounded_Relational_Constraint()                { check_chunk("Ungrounded_Relational_Constraint.soar", 8, 1); }
        void Ungrounded_STI_Promotion()                        { check_chunk("Ungrounded_STI_Promotion.soar", 8, 1); }
        void Ungrounded_STIs()                                 { check_chunk("Ungrounded_STIs.soar", 8, 1); }
        void Unify_Ambiguous_Output()                          { check_chunk("Unify_Ambiguous_Output.soar", 8, 1); }
        void Unify_Children_Results()                          { check_chunk("Unify_Children_Results.soar", 8, 1); }
        void Unify_through_Two_Traces_Four_Deep()              { check_chunk("Unify_through_Two_Traces_Four_Deep.soar", 8, 3); }
        void Vrblzd_Constraint_on_Ungrounded()                 { check_chunk("Vrblzd_Constraint_on_Ungrounded.soar", 8, 1); }

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

void ChunkTest::agent_command(const char* pCmd)
{
    sml::ClientAnalyzedXML response;
    pAgent->ExecuteCommandLineXML(pCmd, &response);
    CPPUNIT_ASSERT_MESSAGE(response.GetResultString(), pAgent->GetLastCommandLineResult());
}

void ChunkTest::start_log(const char* path)
{
    std::string lCmdName("output log ");
    lCmdName += path;
    lCmdName.resize(lCmdName.size() - 5);
    lCmdName += "_log.txt";
    #ifdef SAVE_LOG_FILES
        agent_command(lCmdName.c_str());
    #endif
}

void ChunkTest::continue_log(const char* path)
{
    std::string lCmdName("output log -A ");
    lCmdName += path;
    lCmdName.resize(lCmdName.size() - 5);
    lCmdName += "_log.txt";
    #ifdef SAVE_LOG_FILES
        agent_command(lCmdName.c_str());
    #endif
}

void ChunkTest::close_log()
{
    std::string lCmdName("output log -c");
    #ifdef SAVE_LOG_FILES
        agent_command(lCmdName.c_str());
    #endif
}

void ChunkTest::save_chunks(const char* path)
{
    std::string lCmdName;
    #ifdef SAVE_LOG_FILES
        lCmdName = "output command-to-file unit_test_chunks_";
        lCmdName += path;
        lCmdName.resize(lCmdName.size() - 5);
        lCmdName += ".soar print -frc";
    #else
        lCmdName = "output command-to-file unit_test_chunks.soar print -fcr";
    #endif
    agent_command(lCmdName.c_str());
}


void ChunkTest::source_saved_chunks(const char* path)
{
    std::string lCmdName;
    #ifdef SAVE_LOG_FILES
        lCmdName = "source unit_test_chunks_";
        lCmdName += path;
    #else
        lCmdName = "source unit_test_chunks.soar";
    #endif
    agent_command(lCmdName.c_str());
}

void ChunkTest::check_chunk(const std::string& path, int64_t decisions, int64_t expected_chunks, bool directSourceChunks)
{
    start_log(path.c_str());
    source(path.c_str());
    #ifdef TURN_EXPLAINER_ON
        agent_command("explain all on");
        agent_command("explain just on");
    #endif
    pAgent->RunSelf(decisions, sml::sml_DECISION);
    CPPUNIT_ASSERT_MESSAGE(pAgent->GetLastErrorDescription(), pAgent->GetLastCommandLineResult());
    if (!directSourceChunks)
    {
        close_log();
        save_chunks(path.c_str());
        tearDown();
        setUp();
        continue_log(path.c_str());
        source_saved_chunks(path.c_str());
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
            outStringStream << "--> Expecting to verify " << expected_chunks << " chunks: " << ignored << " matched but " << sourced << " did not.  ";
            std::string outString = outStringStream.str();
            std::cout << outString;
            #ifdef SAVE_LOG_FILES
                agent_command((std::string("output log --add |") + outString + std::string("|")).c_str());
            #endif
            throw CPPUnit_Assert_Failure(outStringStream.str());
        } else {
            #ifdef SAVE_LOG_FILES
                agent_command("output log -a Success!!!  All expected rules were learned!!!");
            #endif
        }
    }

    #ifdef INIT_AFTER_RUN
        #ifdef SAVE_LOG_FILES
            agent_command("output log -a Testing re-initialization of Soar for memory leaks and crashes.");
        #endif
        agent_command("soar init");
        agent_command("trace 0");
        agent_command("run 100");
        agent_command("trace 1");
        agent_command("soar init");
    #endif
    close_log();
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
    agent_command("output console off");
    agent_command("output callbacks on");
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


void ChunkTest::Chunk_All_Only_Except()
{
    source("Chunk_All_Only_Except.soar");
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
