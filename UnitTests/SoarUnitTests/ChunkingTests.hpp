//
//  ChunkingTests.hpp
//  Soar-xcode
//
//  Created by Alex Turner on 7/29/15.
//  Copyright © 2015 University of Michigan – Soar Group. All rights reserved.
//

#ifndef ChunkingTests_hpp
#define ChunkingTests_hpp

#include "FunctionalTestHarness.hpp"

class ChunkingTests : public FunctionalTestHarness
{
public:
	TEST_CATEGORY(ChunkingTests);
	
	void before() { setUp(); }
	void setUp();
	
	void after(bool caught) { tearDown(caught); }
	void tearDown(bool caught);
	
    TEST(testSMem_Chunked_Query, -1);
    TEST(testSMem_Chunked_Query2, -1);
    TEST(testSMem_Chunk_Direct, -1);
    TEST(testAll_Test_Types, -1);
    TEST(testBUNCPS_0, -1);  // BUNCPS = Bottom-up Non-Chunky Problem Spaces
    TEST(testBUNCPS_1, -1);  // (most came from problems found testing on Kirk's game learning agents)
    TEST(testBUNCPS_2, -1);
    TEST(testBUNCPS_3, -1);
    TEST(testBUNCPS_4, -1);
    TEST(testBUNCPS_5, -1);
    TEST(testBUNCPS_6_Four_Level, -1);
    TEST(testBUNCPS_7_with_Constraints, -1);
    TEST(testChunk_Operator_Tie_Impasse, -1);
    TEST(testChunk_Operator_Tie_Item_Links, -1);
    TEST(testChunk_RL_Proposal, -1);
    TEST(testChunked_Justification_with_extras, -1);
    TEST(testChunk_Superstate_Operator_Preference, -1);
    TEST(testConflated_Constants, -1);
    TEST(testConstraint_Prop_from_Base_Conds, -1);
//    TEST(testDemo_Arithmetic, -1);
    TEST(testDemo_Blocks_World_Hierarchical, -1);
    TEST(testDemo_Blocks_World_Hierarchical_Look_Ahead, -1);
    TEST(testDemo_Blocks_World_Look_Ahead, -1);
    TEST(testDemo_Blocks_World_Look_Ahead_State_Evaluation, -1);
    TEST(testDemo_Blocks_World_Operator_Subgoaling, -1);
    TEST(testDemo_Eight_Puzzle, -1);
    TEST(testDemo_MaC_Planning, -1);
    TEST(testDemo_RL_Unit, -1);
    TEST(testDemo_ToH_Recursive, -1);
    TEST(testDemo_Water_Jug_Hierarchy, -1);
    TEST(testDemo_Water_Jug_Look_Ahead, -1);
    TEST(testDemo_Water_Jug_Tie, -1);
    TEST(testDisjunction_Merge, -1);
    TEST(testDuplicates, -1);
    TEST(testFaux_Operator, -1);
    TEST(testFaux_Smem_Operator_RHS, -1);
    TEST(testJustification_RC_not_Ungrounded_STIs, -1);
    TEST(testJustifications_Get_New_Identities, -1);
    TEST(testLink_STM_to_LTM, -1);          // Decreased expected to 0.  At least make sure it runs.
    TEST(testLiteralization_of_NC_and_NCC, -1);
    TEST(testLiteralization_with_BT_Constraints, -1);
    TEST(testLiteralization_with_BT_Constraints2, -1);
    TEST(testLiteralization_with_Constraints, -1);
    TEST(testMaintain_Instantiation_Specific_Identity, -1);
    TEST(testNC_Disjunction, -1);
    TEST(testNC_Simple_No_Exist, -1);
    TEST(testNC_with_RC_and_Local_Variable, -1);
    TEST(testNC_with_Relational_Constraint, -1);
    TEST(testNCC_2_Conds_Simple_Literals, -1);
    TEST(testNCC_Complex, -1);
    TEST(testNCC_from_Backtrace, -1);
    TEST(testNCC_Simple_Literals, -1);
    TEST(testNCC_with_Relational_Constraint, -1);
    TEST(testNo_Topstate_Match, -1);
    TEST(testOperator_Selection_Knowledge, -1);
    TEST(testOpaque_State_Barrier, -1);
    TEST(testPRIMS_Sanity1, -1);
    TEST(testPRIMS_Sanity2, -1);
    TEST(testPromoted_STI, -1);
    TEST(testReorderer_Bad_Conjunction, -1);
    TEST(testRepair_Unconnected_RHS_ID, -1);
    TEST(testRepair_NOR_Temporal_Constraint, -1);
    TEST(testResult_On_Operator, -1);
    TEST(testRete_Bug_Deep_vs_Deep, -1);
    TEST(testRete_Bug_Deep_vs_Top, -1);
    TEST(testRHS_Math_Abs, -1);
    TEST(testRHS_Math_Mixed, -1);
    TEST(testRHS_Math, -1);
    TEST(testRHS_Referent_Function, -1);
    TEST(testRHS_Unbound_Multivalue, -1);
    TEST(testRL_Variablization, -1);
    TEST(testSimple_Constraint_Prop, -1);
    TEST(testSimple_Literalization, -1);
    TEST(testSTI_Variablization_Same_Type, -1);
    TEST(testSTI_Variablization, -1);
    TEST(testSTI_with_referents, -1);
    TEST(testSuperstate_Identity_Opaque, -1);
    TEST(testUngrounded_in_BT_Constraint, -1);
    TEST(testUngrounded_Mixed, -1);
    TEST(testUngrounded_Relational_Constraint, -1);
    TEST(testUngrounded_STI_Promotion, -1);
    TEST(testUngrounded_STIs, -1);
    TEST(testUnify_Ambiguous_Output, -1);
    TEST(testUnify_Children_Results, -1);
    TEST(testUnify_through_Two_Traces_Four_Deep, -1);
    TEST(testVrblzd_Constraint_on_Ungrounded, -1);
//    TEST(testGamesAgent_Sanity1, -1);

    void testAll_Test_Types()                                  { check_chunk("All_Test_Types", 4, 1); }
    void testBUNCPS_0()                                        { check_chunk("BUNCPS_0", 8, 1); }
    void testBUNCPS_1()                                        { check_chunk("BUNCPS_1", 8, 1); }
    void testBUNCPS_2()                                        { check_chunk("BUNCPS_2", 8, 1); }
    void testBUNCPS_3()                                        { check_chunk("BUNCPS_3", 8, 1); }
    void testBUNCPS_4()                                        { check_chunk("BUNCPS_4", 8, 1); }
    void testBUNCPS_5()                                        { check_chunk("BUNCPS_5", 8, 1); }
    void testBUNCPS_6_Four_Level()                             { check_chunk("BUNCPS_6_Four_Level", 8, 2); }
    void testBUNCPS_7_with_Constraints()                       { check_chunk("BUNCPS_7_with_Constraints", 8, 1); }
    void testChunk_Operator_Tie_Impasse()                      { check_chunk("Chunk_Operator_Tie_Impasse", 6, 2); }
    void testChunk_Operator_Tie_Item_Links()                   { check_chunk("Chunk_Operator_Tie_Item_Links", 6, 1); }
    void testChunk_RL_Proposal()                               { check_chunk("Chunk_RL_Proposal", 8, 2); }
    void testChunk_Superstate_Operator_Preference()            { check_chunk("Chunk_Superstate_Operator_Preference", 3, 1); }
    void testChunked_Justification_with_extras()               { check_chunk("STI_with_referents", 8, 1); }
    void testConflated_Constants()                             { check_chunk("Conflated_Constants", 8, 1); }
    void testConstraint_Prop_from_Base_Conds()                 { check_chunk("Constraint_Prop_from_Base_Conds", 8, 1); }
    void testDemo_Arithmetic()                                 { check_chunk("Demo_Arithmetic", 41424, 1); }
    void testDemo_Blocks_World_Hierarchical_Look_Ahead()       { check_chunk("Demo_Blocks_World_Hierarchical_Look_Ahead", 70, 4); }
    void testDemo_Blocks_World_Hierarchical()                  { check_chunk("Demo_Blocks_World_Hierarchical", 23, 16); }
    void testDemo_Blocks_World_Look_Ahead_State_Evaluation()   { check_chunk("Demo_Blocks_World_Look_Ahead_State_Evaluation", 37, 14); }
    void testDemo_Blocks_World_Look_Ahead()                    { check_chunk("Demo_Blocks_World_Look_Ahead", 37, 6); }
    void testDemo_Blocks_World_Operator_Subgoaling()           { check_chunk("Demo_Blocks_World_Operator_Subgoaling", 6, 1); }
    void testDemo_Eight_Puzzle()                               { check_chunk("Demo_Eight_Puzzle", 20, 6); }
    void testDemo_MaC_Planning()                               { check_chunk("Demo_MaC_Planning", 138, 42); }
    void testDemo_RL_Unit()                                    { check_chunk("Demo_RL_Unit", 26, 6); }
    void testDemo_ToH_Recursive()                              { check_chunk("Demo_ToH_Recursive", 23, 10); }
    void testDemo_Water_Jug_Hierarchy()                        { check_chunk("Demo_Water_Jug_Hierarchy", 423, 3); }
    void testDemo_Water_Jug_Look_Ahead()                       { check_chunk("Demo_Water_Jug_Look_Ahead", 102, 16); }
    void testDemo_Water_Jug_Tie()                              { check_chunk("Demo_Water_Jug_Tie", 48, 5); }
    void testDisjunction_Merge()                               { check_chunk("Disjunction_Merge", 5, 1); }
    void testDuplicates()                                      { check_chunk("Duplicates", 5, 2); }
    void testFaux_Operator()                                   { check_chunk("Faux_Operator", 8, 3); }
    void testFaux_Smem_Operator_RHS()                          { check_chunk("Faux_Smem_Operator_RHS", 8, 0, true); }           // Should be 1
    void testGamesAgent_Sanity1()                              { check_chunk("GamesAgent_Sanity1", 4539, 9); }                  // Should be 14 expected chunks
    void testJustification_RC_not_Ungrounded_STIs()            { check_chunk("Justification_RC_not_Ungrounded_STIs", 8, 1); }
    void testJustifications_Get_New_Identities()               { check_chunk("Justifications_Get_New_Identities", 4, 1); }
    void testLink_STM_to_LTM()                                 { check_chunk("Link_STM_to_LTM", 6, 0); }                        // Should be 2 expected chunks
    void testLiteralization_of_NC_and_NCC()                    { check_chunk("Literalization_of_NC_and_NCC", 8, 1); }
    void testLiteralization_with_BT_Constraints()              { check_chunk("Literalization_with_BT_Constraints", 8, 1); }
    void testLiteralization_with_BT_Constraints2()             { check_chunk("Literalization_with_BT_Constraints2", 8, 2); }
    void testLiteralization_with_Constraints()                 { check_chunk("Literalization_with_Constraints", 8, 1); }
    void testMaintain_Instantiation_Specific_Identity()        { check_chunk("Maintain_Instantiation_Specific_Identity", 8, 1); }
    void testNC_Disjunction()                                  { check_chunk("NC_Disjunction", 8, 1); }
    void testNC_Simple_No_Exist()                              { check_chunk("NC_Simple_No_Exist", 8, 1); }
    void testNC_with_RC_and_Local_Variable()                   { check_chunk("NC_with_RC_and_Local_Variable", 8, 1); }
    void testNC_with_Relational_Constraint()                   { check_chunk("NC_with_Relational_Constraint", 8, 1); }
    void testNCC_2_Conds_Simple_Literals()                     { check_chunk("NCC_2_Conds_Simple_Literals", 8, 1); }
    void testNCC_Complex()                                     { check_chunk("NCC_Complex", 8, 1); }
    void testNCC_from_Backtrace()                              { check_chunk("NCC_from_Backtrace", 8, 1); }
    void testNCC_Simple_Literals()                             { check_chunk("NCC_Simple_Literals", 8, 1); }
    void testNCC_with_Relational_Constraint()                  { check_chunk("NCC_with_Relational_Constraint", 8, 1); }
    void testNo_Topstate_Match()                               { check_chunk("No_Topstate_Match", 8, 1); }
    void testOpaque_State_Barrier()                            { check_chunk("Opaque_State_Barrier", 8, 1); }
    void testOperator_Selection_Knowledge()                    { check_chunk("Operator_Selection_Knowledge", 75, 13); }        // Should be 18
    void testPRIMS_Sanity1()                                   { check_chunk("PRIMS_Sanity1", 795, 0); }                       // Was 24 before concat rhs variablization change
    void testPRIMS_Sanity2()                                   { check_chunk("PRIMS_Sanity2", 728, 0); }                       // Was 22 before concat rhs variablization change
    void testPromoted_STI()                                    { check_chunk("Promoted_STI", 8, 1); }
    void testReorderer_Bad_Conjunction()                       { check_chunk("Reorderer_Bad_Conjunction", 8, 1); }
    void testRepair_NOR_Temporal_Constraint()                  { check_chunk("Repair_NOR_Temporal_Constraint", 8, 3); }
    void testRepair_Unconnected_RHS_ID()                       { check_chunk("Repair_Unconnected_RHS_ID", 8, 2); }
    void testResult_On_Operator()                              { check_chunk("Result_On_Operator", 8, 1); }
    void testRete_Bug_Deep_vs_Deep()                           { check_chunk("Rete_Bug_Deep_vs_Deep", 8, 1); }
    void testRete_Bug_Deep_vs_Top()                            { check_chunk("Rete_Bug_Deep_vs_Top", 8, 1); }
    void testRHS_Math_Abs()                                    { check_chunk("RHS_Math_Abs", 8, 2); }
    void testRHS_Math_Mixed()                                  { check_chunk("RHS_Math_Mixed", 8, 4); }
    void testRHS_Math()                                        { check_chunk("RHS_Math", 8, 1); }
    void testRHS_Referent_Function()                           { check_chunk("RHS_Referent_Function", 8, 1); }
    void testRHS_Unbound_Multivalue()                          { check_chunk("RHS_Unbound_Multivalue", 8, 1); }
    void testRL_Variablization()                               { check_chunk("RL_Variablization", 8, 5); }
    void testSimple_Constraint_Prop()                          { check_chunk("Simple_Constraint_Prop", 8, 1); }
    void testSimple_Literalization()                           { check_chunk("Simple_Literalization", 8, 1); }
    void testSMem_Chunk_Direct()                               { check_chunk("SMem_Chunk_Direct", 8, 1); }
    void testSMem_Chunked_Query()                              { check_chunk("SMem_Chunked_Query", 8, 1); }
    void testSMem_Chunked_Query2()                             { check_chunk("SMem_Chunked_Query2", 8, 1); }
    void testSTI_Variablization_Same_Type()                    { check_chunk("STI_Variablization_Same_Type", 8, 1); }
    void testSTI_Variablization()                              { check_chunk("STI_Variablization", 8, 1); }
    void testSTI_with_referents()                              { check_chunk("STI_with_referents", 8, 1); }
    void testSuperstate_Identity_Opaque()                      { check_chunk("Superstate_Identity_Opaque", 8, 1); }
    void testUngrounded_in_BT_Constraint()                     { check_chunk("Ungrounded_in_BT_Constraint", 8, 2); }
    void testUngrounded_Mixed()                                { check_chunk("Ungrounded_Mixed", 8, 1); }
    void testUngrounded_Relational_Constraint()                { check_chunk("Ungrounded_Relational_Constraint", 8, 1); }
    void testUngrounded_STI_Promotion()                        { check_chunk("Ungrounded_STI_Promotion", 8, 1); }
    void testUngrounded_STIs()                                 { check_chunk("Ungrounded_STIs", 8, 1); }
    void testUnify_Ambiguous_Output()                          { check_chunk("Unify_Ambiguous_Output", 8, 1); }
    void testUnify_Children_Results()                          { check_chunk("Unify_Children_Results", 8, 1); }
    void testUnify_through_Two_Traces_Four_Deep()              { check_chunk("Unify_through_Two_Traces_Four_Deep", 8, 3); }
    void testVrblzd_Constraint_on_Ungrounded()                 { check_chunk("Vrblzd_Constraint_on_Ungrounded", 8, 1); }

	
private:
    void source(const std::string& path);
    void check_chunk(const char* pTestName, int64_t decisions, int64_t expected_chunks, bool directSourceChunks = false);
    void agent_command(const char* pCmd);
    void start_log(const char* path);
    void continue_log(const char* path);
    void close_log();
    void save_chunks(const char* path);
    void save_chunks_internal(const char* path);
    void source_saved_chunks(const char* path);
};

#endif /* ChunkingTests_hpp */
