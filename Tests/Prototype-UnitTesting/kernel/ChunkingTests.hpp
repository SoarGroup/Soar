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
	
	TEST(testSTI_Variablization,-1);
	void testSTI_Variablization();
	
	TEST(testSTI_Variablization_Same_Type,-1);
	void testSTI_Variablization_Same_Type();
	
	TEST(testSuperstate_Identity_Opaque,-1);
	void testSuperstate_Identity_Opaque();
	
	TEST(testRHS_Unbound_Multivalue,-1);
	void testRHS_Unbound_Multivalue();
	
	TEST(testAll_Test_Types,-1);
	void testAll_Test_Types();
	
	TEST(testConflated_Constants,-1);
	void testConflated_Constants();
	
	TEST(testRete_Bug_Deep_vs_Top,-1);
	void testRete_Bug_Deep_vs_Top();
	
	TEST(testRete_Bug_Deep_vs_Deep,-1);
	void testRete_Bug_Deep_vs_Deep();
	
	TEST(testUngrounded_STIs,-1);
	void testUngrounded_STIs();
	
	TEST(testUngrounded_Mixed,-1);
	void testUngrounded_Mixed();
	
	TEST(testUngrounded_STI_Promotion,-1);
	void testUngrounded_STI_Promotion();
	
	TEST(testUngrounded_Relational_Constraint,-1);
	void testUngrounded_Relational_Constraint();
	
	TEST(testUngrounded_in_BT_Constraint,-1);
	void testUngrounded_in_BT_Constraint();
	
	TEST(testVrblzd_Constraint_on_Ungrounded,-1);
	void testVrblzd_Constraint_on_Ungrounded();
	
	TEST(testNC_Simple_No_Exist,-1);
	void testNC_Simple_No_Exist();
	
	TEST(testNC_with_Relational_Constraint,-1);
	void testNC_with_Relational_Constraint();
	
	TEST(testNC_with_RC_and_Local_Variable,-1);
	void testNC_with_RC_and_Local_Variable();
	
	TEST(testNCC_Simple_Literals,-1);
	void testNCC_Simple_Literals();
	
	TEST(testNCC_2_Conds_Simple_Literals,-1);
	void testNCC_2_Conds_Simple_Literals();
	
	TEST(testNCC_with_Relational_Constraint,-1);
	void testNCC_with_Relational_Constraint();
	
	TEST(testNCC_Complex,-1);
	void testNCC_Complex();
	
	TEST(testNCC_from_Backtrace,-1);
	void testNCC_from_Backtrace();
	
	TEST(testJustification_RC_not_Ungrounded_STIs,-1);
	void testJustification_RC_not_Ungrounded_STIs();
	
	TEST(testProhibit_Fake_Instantiation_LTIs,-1);
	void testProhibit_Fake_Instantiation_LTIs();
	
	TEST(testMaintain_Instantiation_Specific_Identity,-1);
	void testMaintain_Instantiation_Specific_Identity();
	
	TEST(testSimple_Constraint_Prop,-1);
	void testSimple_Constraint_Prop();
	
	TEST(testConstraint_Prop_from_Base_Conds,-1);
	void testConstraint_Prop_from_Base_Conds();
	
	TEST(testSimple_Literalization,-1);
	void testSimple_Literalization();
	
	TEST(testLiteralization_of_NC_and_NCC,-1);
	void testLiteralization_of_NC_and_NCC();
	
	TEST(testLiteralization_with_Constraints,-1);
	void testLiteralization_with_Constraints();
	
	TEST(testLiteralization_with_BT_Constraints,-1);
	void testLiteralization_with_BT_Constraints();
	
	TEST(testBUNCPS_0,-1);
	void testBUNCPS_0();
	
	TEST(testBUNCPS_1,-1);
	void testBUNCPS_1();
	
	TEST(testBUNCPS_2,-1);
	void testBUNCPS_2();
	
	TEST(testBUNCPS_3,-1);
	void testBUNCPS_3();
	
	TEST(testBUNCPS_4,-1);
	void testBUNCPS_4();
	
	TEST(testBUNCPS_5,-1);
	void testBUNCPS_5();
	
	TEST(testBUNCPS_6_Four_Level,-1);
	void testBUNCPS_6_Four_Level();
	
	TEST(testBUNCPS_7_with_Constraints,-1);
	void testBUNCPS_7_with_Constraints();
	
	TEST(testLiteralization_with_BT_Constraints2,-1);
	void testLiteralization_with_BT_Constraints2();
	
	TEST(testUnify_through_Two_Traces_Four_Deep,-1);
	void testUnify_through_Two_Traces_Four_Deep();
	
	TEST(testRL_Variablization,-1);
	void testRL_Variablization();
	
private:
	void build_and_check_chunk(const std::string& path, int decisions, int expected_chunks);
};

#endif /* ChunkingTests_hpp */
