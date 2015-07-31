//
//  ChunkingTests.cpp
//  Soar-xcode
//
//  Created by Alex Turner on 7/29/15.
//  Copyright © 2015 University of Michigan – Soar Group. All rights reserved.
//

#include "ChunkingTests.hpp"
#include "sml_ClientAnalyzedXML.h"
#include "SoarHelper.hpp"

void ChunkingTests::build_and_check_chunk(const std::string& name, int decisions, int expected_chunks)
{
	runTest(name, decisions);
	
	sml::ClientAnalyzedXML response;
	
	std::string sourceName = this->getCategoryName() + "_" + name + "_expected" + ".soar";
	
	std::string path = SoarHelper::GetResource(sourceName);
	assertNonZeroSize_msg("Could not find test file '" + sourceName + "'", path);
	
	agent->ExecuteCommandLineXML(std::string("source \"" + path + "\"").c_str(), &response);
	int sourced, excised, ignored;
	ignored = response.GetArgInt(sml::sml_Names::kParamIgnoredProductionCount, -1);
	sourced = response.GetArgInt(sml::sml_Names::kParamSourcedProductionCount, -1);
	excised = response.GetArgInt(sml::sml_Names::kParamExcisedProductionCount, -1);
	
	std::ostringstream outStringStream("");
	outStringStream << "--> Expected to ignore " << expected_chunks << ": Src = " << sourced << ", Exc = " << excised << ", Ign = " << ignored;
	
	assertEquals_msg(ignored, expected_chunks, outStringStream.str());
}

void ChunkingTests::setUp()
{
	FunctionalTestHarness::setUp();
}

void ChunkingTests::tearDown(bool caught)
{
	FunctionalTestHarness::tearDown(caught);
}

void ChunkingTests::testAll_Test_Types()
{
	/*
	 # Tests:
	 # - All relational test types with integers
	 # - Includes literal relational test and disjunction
	 # - RHS actions that are variablized
	 #-  RHS actions with literals that are the same symbols
	 #   as were variablized.
	 */
	
	build_and_check_chunk("testAll_Test_Types", 5, 1);
}

void ChunkingTests::testUngrounded_Relational_Constraint()
{
	build_and_check_chunk("testUngrounded_Relational_Constraint", 3, 1);
}

void ChunkingTests::testVrblzd_Constraint_on_Ungrounded()
{
	build_and_check_chunk("testVrblzd_Constraint_on_Ungrounded", 4, 1);
}

void ChunkingTests::testLiteralization_with_Constraints()
{
	build_and_check_chunk("testLiteralization_with_Constraints", 3, 1);
}

void ChunkingTests::testConflated_Constants()
{
	build_and_check_chunk("testConflated_Constants", 3, 1);
}

void ChunkingTests::testSuperstate_Identity_Opaque()
{
	build_and_check_chunk("testSuperstate_Identity_Opaque", 2, 1);
}

void ChunkingTests::testUngrounded_in_BT_Constraint()
{
	build_and_check_chunk("testUngrounded_in_BT_Constraint", 3, 2);
}

void ChunkingTests::testSTI_Variablization()
{
	build_and_check_chunk("testSTI_Variablization", 3, 1);
}

void ChunkingTests::testSTI_Variablization_Same_Type()
{
	build_and_check_chunk("testSTI_Variablization_Same_Type", 3, 1);
}

void ChunkingTests::testRHS_Unbound_Multivalue()
{
	build_and_check_chunk("testRHS_Unbound_Multivalue", 3, 2);
}

void ChunkingTests::testRete_Bug_Deep_vs_Top()
{
	build_and_check_chunk("testRete_Bug_Deep_vs_Top", 3, 1);
}

void ChunkingTests::testRete_Bug_Deep_vs_Deep()
{
	build_and_check_chunk("testRete_Bug_Deep_vs_Deep", 3, 1);
}

void ChunkingTests::testUngrounded_STIs()
{
	build_and_check_chunk("testUngrounded_STIs", 2, 1);
}

void ChunkingTests::testUngrounded_Mixed()
{
	build_and_check_chunk("testUngrounded_Mixed", 3, 1);
}

void ChunkingTests::testUngrounded_STI_Promotion()
{
	build_and_check_chunk("testUngrounded_STI_Promotion", 3, 1);
}

void ChunkingTests::testNC_with_RC_and_Local_Variable()
{
	build_and_check_chunk("testNC_with_RC_and_Local_Variable", 3, 1);
}

void ChunkingTests::testNCC_Simple_Literals()
{
	build_and_check_chunk("testNCC_Simple_Literals", 3, 1);
}

void ChunkingTests::testNC_Simple_No_Exist()
{
	build_and_check_chunk("testNC_Simple_No_Exist", 3, 1);
}

void ChunkingTests::testNC_with_Relational_Constraint()
{
	build_and_check_chunk("testNC_with_Relational_Constraint", 3, 1);
}

void ChunkingTests::testNCC_2_Conds_Simple_Literals()
{
	build_and_check_chunk("testNCC_2_Conds_Simple_Literals", 3, 1);
}

void ChunkingTests::testNCC_with_Relational_Constraint()
{
	build_and_check_chunk("testNCC_with_Relational_Constraint", 3, 1);
}

void ChunkingTests::testNCC_Complex()
{
	build_and_check_chunk("testNCC_Complex", 3, 1);
}

void ChunkingTests::testNCC_from_Backtrace()
{
	build_and_check_chunk("testNCC_from_Backtrace", 3, 1);
}

void ChunkingTests::testRL_Variablization()
{
	build_and_check_chunk("testRL_Variablization", 2, 5);
}

void ChunkingTests::testBUNCPS_0()
{
	build_and_check_chunk("testBUNCPS_0", 5, 1);
}

void ChunkingTests::testProhibit_Fake_Instantiation_LTIs()
{
	build_and_check_chunk("testProhibit_Fake_Instantiation_LTIs", 6, 1);
}

void ChunkingTests::testBUNCPS_1()
{
	build_and_check_chunk("testBUNCPS_1", 5, 1);
}

void ChunkingTests::testBUNCPS_2()
{
	build_and_check_chunk("testBUNCPS_2", 5, 1);
}

void ChunkingTests::testBUNCPS_3()
{
	build_and_check_chunk("testBUNCPS_3", 4, 1);
}

void ChunkingTests::testMaintain_Instantiation_Specific_Identity()
{
	build_and_check_chunk("testMaintain_Instantiation_Specific_Identity", 2, 1);
}

void ChunkingTests::testBUNCPS_4()
{
	build_and_check_chunk("testBUNCPS_4", 5, 1);
}

void ChunkingTests::testJustification_RC_not_Ungrounded_STIs()
{
	build_and_check_chunk("testJustification_RC_not_Ungrounded_STIs", 6, 1);
}

void ChunkingTests::testBUNCPS_5()
{
	build_and_check_chunk("testBUNCPS_5", 6, 1);
}

void ChunkingTests::testBUNCPS_6_Four_Level()
{
	build_and_check_chunk("testBUNCPS_6_Four_Level", 4, 2);
}

void ChunkingTests::testBUNCPS_7_with_Constraints()
{
	build_and_check_chunk("testBUNCPS_7_with_Constraints", 4, 1);
}

void ChunkingTests::testSimple_Literalization()
{
	/* Literalization and constraint maintenance */
	build_and_check_chunk("testSimple_Literalization", 3, 1);
}

void ChunkingTests::testConstraint_Prop_from_Base_Conds()
{
	/* Constraint maintenance from base conditions */
	build_and_check_chunk("testConstraint_Prop_from_Base_Conds", 3, 1);
}

void ChunkingTests::testSimple_Constraint_Prop()
{
	build_and_check_chunk("testSimple_Constraint_Prop", 3, 1);
}

void ChunkingTests::testLiteralization_of_NC_and_NCC()
{
	build_and_check_chunk("testLiteralization_of_NC_and_NCC", 3, 1);
}

void ChunkingTests::testLiteralization_with_BT_Constraints()
{
	build_and_check_chunk("testLiteralization_with_BT_Constraints", 3, 1);
}

void ChunkingTests::testLiteralization_with_BT_Constraints2()
{
	build_and_check_chunk("testLiteralization_with_BT_Constraints2", 3, 2);
}

void ChunkingTests::testUnify_through_Two_Traces_Four_Deep()
{
	build_and_check_chunk("testUnify_through_Two_Traces_Four_Deep", 3, 1);
}

void ChunkingTests::testChunk43()
{
	build_and_check_chunk("testchunk43", 3, 1);
}

void ChunkingTests::testChunk44()
{
	build_and_check_chunk("testchunk44", 2, 1);
}

void ChunkingTests::testChunk45()
{
	build_and_check_chunk("testchunk45", 3, 1);
}

void ChunkingTests::testChunk46()
{
	build_and_check_chunk("testchunk46", 3, 1);
}

void ChunkingTests::testChunk47()
{
	build_and_check_chunk("testchunk47", 3, 1);
}

void ChunkingTests::testChunk48()
{
	build_and_check_chunk("testchunk48", 3, 1);
}

void ChunkingTests::testChunk49()
{
	build_and_check_chunk("testchunk49", 3, 1);
}
