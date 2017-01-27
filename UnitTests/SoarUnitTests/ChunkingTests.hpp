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
	
    TEST(SMem_Chunked_Query, -1);
    TEST(SMem_Chunked_Query2, -1);
    TEST(SMem_Chunk_Direct, -1);
    TEST(All_Test_Types, -1);
    TEST(BUNCPS_0, -1);  // BUNCPS = Bottom-up Non-Chunky Problem Spaces
    TEST(BUNCPS_1, -1);  // (most came from problems found testing on Kirk's game learning agents)
    TEST(BUNCPS_2, -1);
    TEST(BUNCPS_3, -1);
    TEST(BUNCPS_4, -1);
    TEST(BUNCPS_5, -1);
    TEST(BUNCPS_6_Four_Level, -1);
    TEST(BUNCPS_7_with_Constraints, -1);
    TEST(Chunk_Operator_Tie_Impasse, -1);
    TEST(Chunk_Operator_Tie_Item_Links, -1);
    TEST(Chunk_RL_Proposal, -1);
    TEST(Chunk_Superstate_Operator_Preference, -1);
    TEST(Chunked_Justification_with_extras, -1);
    TEST(Conflated_Constants, -1);
    TEST(Constraint_Prop_from_Base_Conds, -1);
    TEST(Constraint_Ungrounded, -1);
    TEST(Deep_Copy_Identity_Expansion, -1);
    TEST(Demo_Arithmetic, -1);
    TEST(Demo_Blocks_World_Hierarchical_Look_Ahead, -1);
    TEST(Demo_Blocks_World_Hierarchical, -1);
    TEST(Demo_Blocks_World_Look_Ahead_State_Evaluation, -1);
    TEST(Demo_Blocks_World_Look_Ahead, -1);
    TEST(Demo_Blocks_World_Operator_Subgoaling, -1);
    TEST(Demo_Eight_Puzzle, -1);
    TEST(Demo_MaC_Planning, -1);
    TEST(Demo_RL_Unit, -1);
    TEST(Demo_ToH_Recursive, -1);
    TEST(Demo_Water_Jug_Hierarchy, -1);
    TEST(Demo_Water_Jug_Look_Ahead, -1);
    TEST(Demo_Water_Jug_Tie, -1);
    TEST(Disjunction_Merge, -1);
    TEST(Duplicates, -1);
    TEST(Faux_Operator, -1);
    TEST(Faux_Smem_Operator_RHS, -1);
    TEST(Games_Nccx2_Long_Lived, -1);
    TEST(GamesAgent_Sanity1, -1);
    TEST(Justification_RC_not_Ungrounded_STIs, -1);
    TEST(Justifications_Get_New_Identities, -1);
    TEST(Link_STM_to_LTM, -1);
    TEST(Literalization_of_NC_and_NCC, -1);
    TEST(Literalization_with_BT_Constraints, -1);
    TEST(Literalization_with_BT_Constraints2, -1);
    TEST(Literalization_with_Constraints, -1);
    TEST(Maintain_Instantiation_Specific_Identity, -1);
    TEST(Max_Chunks, -1);
    TEST(Max_Dupes, -1);
    TEST(NC_Disjunction, -1);
    TEST(NC_Simple_No_Exist, -1);
    TEST(NC_with_RC_and_Local_Variable, -1);
    TEST(NC_with_Relational_Constraint, -1);
    TEST(NCC_2_Conds_Simple_Literals, -1);
    TEST(NCC_Complex, -1);
    TEST(NCC_from_Backtrace, -1);
    TEST(NCC_Simple_Literals, -1);
    TEST(NCC_with_Relational_Constraint, -1);
    TEST(No_Grounds, -1);
    TEST(No_Topstate_Match, -1);
    TEST(Opaque_State_Barrier, -1);
    TEST(Operator_Selection_Knowledge, -1);
    TEST(PRIMS_Sanity1, -1);
    TEST(PRIMS_Sanity2, -1);
    TEST(Promoted_STI, -1);
    TEST(Reorderer_Bad_Conjunction, -1);
    TEST(Repair_NOR_Temporal_Constraint, -1);
    TEST(Repair_Unconnected_RHS_ID, -1);
    TEST(Result_On_Operator, -1);
    TEST(Rete_Bug_Deep_vs_Deep, -1);
    TEST(Rete_Bug_Deep_vs_Top, -1);
    TEST(Rhs_Func_Literalization, -1);
    TEST(RHS_Math_Abs, -1);
    TEST(RHS_Math_Mixed, -1);
    TEST(RHS_Math, -1);
    TEST(RHS_Referent_Function, -1);
    TEST(RHS_Unbound_Multivalue, -1);
    TEST(RL_Variablization, -1);
    TEST(Simple_Constraint_Prop, -1);
    TEST(Simple_Literalization, -1);
    TEST(Singleton_Element_Types, -1);
    TEST(Singletons_Architectural, -1);
    TEST(Singletons, -1);
    TEST(STI_Variablization_Same_Type, -1);
    TEST(STI_Variablization, -1);
    TEST(STI_with_referents, -1);
    TEST(Superstate_Identity_Opaque, -1);
    TEST(Ungrounded_in_BT_Constraint, -1);
    TEST(Ungrounded_Mixed, -1);
    TEST(Ungrounded_Relational_Constraint, -1);
    TEST(Ungrounded_STI_Promotion, -1);
    TEST(Ungrounded_STIs, -1);
    TEST(Unify_Ambiguous_Output, -1);
    TEST(Unify_Children_Results, -1);
    TEST(Unify_through_Two_Traces_Four_Deep, -1);

    void All_Test_Types();
    void BUNCPS_0();
    void BUNCPS_1();
    void BUNCPS_2();
    void BUNCPS_3();
    void BUNCPS_4();
    void BUNCPS_5();
    void BUNCPS_6_Four_Level();
    void BUNCPS_7_with_Constraints();
    void Chunk_Operator_Tie_Impasse();
    void Chunk_Operator_Tie_Item_Links();
    void Chunk_RL_Proposal();
    void Chunk_Superstate_Operator_Preference();
    void Chunked_Justification_with_extras();
    void Conflated_Constants();
    void Constraint_Prop_from_Base_Conds();
    void Deep_Copy_Identity_Expansion();
    void Demo_Arithmetic();
    void Demo_Blocks_World_Hierarchical_Look_Ahead();
    void Demo_Blocks_World_Hierarchical();
    void Demo_Blocks_World_Look_Ahead_State_Evaluation();
    void Demo_Blocks_World_Look_Ahead();
    void Demo_Blocks_World_Operator_Subgoaling();
    void Demo_Eight_Puzzle();
    void Demo_MaC_Planning();
    void Demo_RL_Unit();
    void Demo_ToH_Recursive();
    void Demo_Water_Jug_Hierarchy();
    void Demo_Water_Jug_Look_Ahead();
    void Demo_Water_Jug_Tie();
    void Disjunction_Merge();
    void Duplicates();
    void Faux_Operator();
    void Faux_Smem_Operator_RHS();
    void GamesAgent_Sanity1();
    void Justification_RC_not_Ungrounded_STIs();
    void Justifications_Get_New_Identities();
    void Link_STM_to_LTM();
    void Literalization_of_NC_and_NCC();
    void Literalization_with_BT_Constraints();
    void Literalization_with_BT_Constraints2();
    void Literalization_with_Constraints();
    void Maintain_Instantiation_Specific_Identity();
    void NC_Disjunction();
    void NC_Simple_No_Exist();
    void NC_with_RC_and_Local_Variable();
    void NC_with_Relational_Constraint();
    void NCC_2_Conds_Simple_Literals();
    void NCC_Complex();
    void NCC_from_Backtrace();
    void NCC_Simple_Literals();
    void NCC_with_Relational_Constraint();
    void No_Topstate_Match();
    void Opaque_State_Barrier();
    void Operator_Selection_Knowledge();
    void PRIMS_Sanity1();
    void PRIMS_Sanity2();
    void Promoted_STI();
    void Reorderer_Bad_Conjunction();
    void Repair_NOR_Temporal_Constraint();
    void Repair_Unconnected_RHS_ID();
    void Result_On_Operator();
    void Rete_Bug_Deep_vs_Deep();
    void Rete_Bug_Deep_vs_Top();
    void RHS_Math_Abs();
    void RHS_Math_Mixed();
    void RHS_Math();
    void RHS_Referent_Function();
    void RHS_Unbound_Multivalue();
    void RL_Variablization();
    void Simple_Constraint_Prop();
    void Singleton_Element_Types();
    void Simple_Literalization();
    void Singletons();
    void Singletons_Architectural();
    void SMem_Chunk_Direct();
    void SMem_Chunked_Query();
    void SMem_Chunked_Query2();
    void STI_Variablization_Same_Type();
    void STI_Variablization();
    void STI_with_referents();
    void Superstate_Identity_Opaque();
    void Ungrounded_in_BT_Constraint();
    void Ungrounded_Mixed();
    void Ungrounded_Relational_Constraint();
    void Ungrounded_STI_Promotion();
    void Ungrounded_STIs();
    void Unify_Ambiguous_Output();
    void Unify_Children_Results();
    void Unify_through_Two_Traces_Four_Deep();
    void Constraint_Ungrounded();
    void Games_Nccx2_Long_Lived();
    void Max_Chunks();
    void Max_Dupes();
    void No_Grounds();
    void Rhs_Func_Literalization();

private:
    void source(const std::string& path);
    void check_chunk(const char* pTestName, int64_t decisions, int64_t expected_chunks, bool directSourceChunks = false);
    void verify_chunk(const char* pTestName, int64_t expected_chunks, bool directSourceChunks = false);
    void agent_command(const char* pCmd);
    void start_log(const char* path);
    void continue_log(const char* path);
    void close_log();
    void save_chunks(const char* path);
    void save_chunks_internal(const char* path);
    void source_saved_chunks(const char* path);
};

#endif /* ChunkingTests_hpp */
