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

void ChunkingTests::setUp()
{
    configure_for_unit_tests();

	FunctionalTestHarness::setUp();
}

void ChunkingTests::tearDown(bool caught)
{
	FunctionalTestHarness::tearDown(caught);
}

void ChunkingTests::build_and_check_chunk(const std::string& name, int decisions, int expected_chunks)
{
    runTestSetup(name);
    agent->RunSelf(decisions, sml::sml_DECISION);

    sml::ClientAnalyzedXML response;

    std::string sourceName = this->getCategoryName() + "_" + name + "_expected.soar";

    std::string path = SoarHelper::GetResource(sourceName);

    if (path.size() == 0)
    {
        sourceName = name + "_expected.soar";
        path = SoarHelper::GetResource(sourceName);
    }

    if (path.size() == 0 && name.find("test") == 0)
    {
        sourceName = name.substr(std::string("test").size(), -1) + "_expected.soar";
        path = SoarHelper::GetResource(sourceName);
    }

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

void ChunkingTests::testLearn()
{
    runTestSetup("testLearn");

    agent->ExecuteCommandLine("chunk all-except");

    kernel->RunAllAgentsForever();
    {
        sml::ClientAnalyzedXML response;
        agent->ExecuteCommandLineXML("stats", &response);
        assertTrue(response.GetArgInt(sml::sml_Names::kParamStatsCycleCountDecision, -1) == 3);
        assertTrue(response.GetArgInt(sml::sml_Names::kParamStatsCycleCountElaboration, -1) == 5);
    }

    // learning is off, same behavior expected
    agent->ExecuteCommandLine("init");
    kernel->RunAllAgentsForever();
    {
        sml::ClientAnalyzedXML response;
        agent->ExecuteCommandLineXML("stats", &response);
        assertTrue(response.GetArgInt(sml::sml_Names::kParamStatsCycleCountDecision, -1) == 3);
        assertTrue(response.GetArgInt(sml::sml_Names::kParamStatsCycleCountElaboration, -1) == 5);
    }

    // turn learn except on
    agent->ExecuteCommandLine("init");
    agent->ExecuteCommandLine("chunk all-except");
    kernel->RunAllAgentsForever();
    {
        sml::ClientAnalyzedXML response;
        agent->ExecuteCommandLineXML("stats", &response);
        assertTrue(response.GetArgInt(sml::sml_Names::kParamStatsCycleCountDecision, -1) == 3);
        assertTrue(response.GetArgInt(sml::sml_Names::kParamStatsCycleCountElaboration, -1) == 5);
    }

    // don't learn is active so same result expected
    agent->ExecuteCommandLine("init");
    kernel->RunAllAgentsForever();
    {
        sml::ClientAnalyzedXML response;
        agent->ExecuteCommandLineXML("stats", &response);
        assertTrue(response.GetArgInt(sml::sml_Names::kParamStatsCycleCountDecision, -1) == 3);
        assertTrue(response.GetArgInt(sml::sml_Names::kParamStatsCycleCountElaboration, -1) == 5);
    }

    // get rid of dont learn
    agent->ExecuteCommandLine("init");
    agent->ExecuteCommandLine("excise dont*learn");
    kernel->RunAllAgentsForever();
    {
        sml::ClientAnalyzedXML response;
        agent->ExecuteCommandLineXML("stats", &response);
        assertTrue(response.GetArgInt(sml::sml_Names::kParamStatsCycleCountDecision, -1) == 3);
        assertTrue(response.GetArgInt(sml::sml_Names::kParamStatsCycleCountElaboration, -1) == 5);
    }

    // expect improvement
    agent->ExecuteCommandLine("init");
    kernel->RunAllAgentsForever();
    {
        sml::ClientAnalyzedXML response;
        agent->ExecuteCommandLineXML("stats", &response);
        assertTrue(response.GetArgInt(sml::sml_Names::kParamStatsCycleCountDecision, -1) == 1);
        assertTrue(response.GetArgInt(sml::sml_Names::kParamStatsCycleCountElaboration, -1) == 3);
    }

    // go to only mode
    agent->ExecuteCommandLine("init");
    agent->ExecuteCommandLine("excise -c");
    agent->ExecuteCommandLine("chunk only");
    kernel->RunAllAgentsForever();
    {
        sml::ClientAnalyzedXML response;
        agent->ExecuteCommandLineXML("stats", &response);
        assertTrue(response.GetArgInt(sml::sml_Names::kParamStatsCycleCountDecision, -1) == 3);
        assertTrue(response.GetArgInt(sml::sml_Names::kParamStatsCycleCountElaboration, -1) == 5);
    }

    // force learn is active, expect improvement
    agent->ExecuteCommandLine("init");
    kernel->RunAllAgentsForever();
    {
        sml::ClientAnalyzedXML response;
        agent->ExecuteCommandLineXML("stats", &response);
        assertTrue(response.GetArgInt(sml::sml_Names::kParamStatsCycleCountDecision, -1) == 1);
        assertTrue(response.GetArgInt(sml::sml_Names::kParamStatsCycleCountElaboration, -1) == 3);
    }

    // get rid of chunk and force learn
    agent->ExecuteCommandLine("init");
    agent->ExecuteCommandLine("excise -c");
    agent->ExecuteCommandLine("excise force*learn");
    kernel->RunAllAgentsForever();
    {
        sml::ClientAnalyzedXML response;
        agent->ExecuteCommandLineXML("stats", &response);
        assertTrue(response.GetArgInt(sml::sml_Names::kParamStatsCycleCountDecision, -1) == 3);
        assertTrue(response.GetArgInt(sml::sml_Names::kParamStatsCycleCountElaboration, -1) == 5);
    }

    // expect no improvement
    agent->ExecuteCommandLine("init");
    kernel->RunAllAgentsForever();
    {
        sml::ClientAnalyzedXML response;
        agent->ExecuteCommandLineXML("stats", &response);
        assertTrue(response.GetArgInt(sml::sml_Names::kParamStatsCycleCountDecision, -1) == 3);
        assertTrue(response.GetArgInt(sml::sml_Names::kParamStatsCycleCountElaboration, -1) == 5);
    }
}

void ChunkingTests::testAllTestTypes()
{
    /*
     # Tests:
     # - All relational test types with integers
     # - Includes literal relational test and disjunction
     # - RHS actions that are variablized
     #-  RHS actions with literals that are the same symbols
     #   as were variablized.
     */

    build_and_check_chunk("testAll_Test_Types", 8, 1);
}

void ChunkingTests::testUngroundedRelationalConstraint()
{
    build_and_check_chunk("testUngrounded_Relational_Constraint", 8, 1);
}

void ChunkingTests::testVrblzdConstraintonUngrounded()
{
    build_and_check_chunk("testVrblzd_Constraint_on_Ungrounded", 8, 1);
}

void ChunkingTests::testLiteralizationwithConstraints()
{
    build_and_check_chunk("testLiteralization_with_Constraints", 8, 1);
}

void ChunkingTests::testConflatedConstants()
{
    build_and_check_chunk("testConflated_Constants", 8, 1);
}

void ChunkingTests::testSuperstateIdentityOpaque()
{
    build_and_check_chunk("testSuperstate_Identity_Opaque", 8, 1);
}

void ChunkingTests::testUngroundedinBTConstraint()
{
    build_and_check_chunk("testUngrounded_in_BT_Constraint", 8, 2);
}

void ChunkingTests::testSTIVariablization()
{
    build_and_check_chunk("testSTI_Variablization", 8, 1);
}

void ChunkingTests::testSTIVariablizationSameType()
{
    build_and_check_chunk("testSTI_Variablization_Same_Type", 8, 1);
}

void ChunkingTests::testRHSUnboundMultivalue()
{
    build_and_check_chunk("testRHS_Unbound_Multivalue", 8, 1);
}

void ChunkingTests::testReteBugDeepvsTop()
{
    build_and_check_chunk("testRete_Bug_Deep_vs_Top", 8, 1);
}

void ChunkingTests::testReteBugDeepvsDeep()
{
    build_and_check_chunk("testRete_Bug_Deep_vs_Deep", 8, 1);
}

void ChunkingTests::testUngroundedSTIs()
{
    build_and_check_chunk("testUngrounded_STIs", 8, 1);
}

void ChunkingTests::testUngroundedMixed()
{
    build_and_check_chunk("testUngrounded_Mixed", 8, 1);
}

void ChunkingTests::testUngroundedSTIPromotion()
{
    build_and_check_chunk("testUngrounded_STI_Promotion", 8, 1);
}

void ChunkingTests::testNCwithRCandLocalVariable()
{
    build_and_check_chunk("testNC_with_RC_and_Local_Variable", 8, 1);
}

void ChunkingTests::testNCCSimpleLiterals()
{
    build_and_check_chunk("testNCC_Simple_Literals", 8, 1);
}

void ChunkingTests::testNCSimpleNoExist()
{
    build_and_check_chunk("testNC_Simple_No_Exist", 8, 1);
}

void ChunkingTests::testNCwithRelationalConstraint()
{
    build_and_check_chunk("testNC_with_Relational_Constraint", 8, 1);
}

void ChunkingTests::testNCC2CondsSimpleLiterals()
{
    build_and_check_chunk("testNCC_2_Conds_Simple_Literals", 8, 1);
}

void ChunkingTests::testNCCwithRelationalConstraint()
{
    build_and_check_chunk("testNCC_with_Relational_Constraint", 8, 1);
}

void ChunkingTests::testNCCComplex()
{
    build_and_check_chunk("testNCC_Complex", 8, 1);
}

void ChunkingTests::testNCCfromBacktrace()
{
    build_and_check_chunk("testNCC_from_Backtrace", 8, 1);
}

void ChunkingTests::testRLVariablization()
{
    build_and_check_chunk("testRL_Variablization", 8, 5);
}

void ChunkingTests::testBUNCPS0()
{
    build_and_check_chunk("testBUNCPS_0", 8, 1);
}

void ChunkingTests::testBUNCPS1()
{
    build_and_check_chunk("testBUNCPS_1", 8, 1);
}

void ChunkingTests::testBUNCPS2()
{
    build_and_check_chunk("testBUNCPS_2", 8, 1);
}

void ChunkingTests::testBUNCPS3()
{
    build_and_check_chunk("testBUNCPS_3", 8, 1);
}

void ChunkingTests::testMaintainInstantiationSpecificIdentity()
{
    build_and_check_chunk("testMaintain_Instantiation_Specific_Identity", 8, 1);
}

void ChunkingTests::testBUNCPS4()
{
    build_and_check_chunk("testBUNCPS_4", 8, 1);
}

void ChunkingTests::testJustificationRCnotUngroundedSTIs()
{
    build_and_check_chunk("testJustification_RC_not_Ungrounded_STIs", 8, 1);
}

void ChunkingTests::testBUNCPS5()
{
    build_and_check_chunk("testBUNCPS_5", 8, 1);
}

void ChunkingTests::testBUNCPS6FourLevel()
{
    build_and_check_chunk("testBUNCPS_6_Four_Level", 8, 2);
}

void ChunkingTests::testBUNCPS7withConstraints()
{
    build_and_check_chunk("testBUNCPS_7_with_Constraints", 8, 1);
}

void ChunkingTests::testSimpleLiteralization()
{
    /* Literalization and constraint maintenance */
    build_and_check_chunk("testSimple_Literalization", 8, 1);
}

void ChunkingTests::testConstraintPropfromBaseConds()
{
    /* Constraint maintenance from base conditions */
    build_and_check_chunk("testConstraint_Prop_from_Base_Conds", 8, 1);
}

void ChunkingTests::testSimpleConstraintProp()
{
    build_and_check_chunk("testSimple_Constraint_Prop", 8, 1);
}

void ChunkingTests::testLiteralizationofNCandNCC()
{
    build_and_check_chunk("testLiteralization_of_NC_and_NCC", 8, 1);
}

void ChunkingTests::testLiteralizationwithBTConstraints()
{
    build_and_check_chunk("testLiteralization_with_BT_Constraints", 8, 1);
}

void ChunkingTests::testLiteralizationwithBTConstraints2()
{
    build_and_check_chunk("testLiteralization_with_BT_Constraints2", 8, 2);
}

void ChunkingTests::testUnifythroughTwoTracesFourDeep()
{
    build_and_check_chunk("testUnify_through_Two_Traces_Four_Deep", 8, 1);
}

void ChunkingTests::testSTIwithreferents()
{
    build_and_check_chunk("testSTI_with_referents", 8, 1);
}
void ChunkingTests::testChunkedJustificationwithextras()
{
    build_and_check_chunk("testSTI_with_referents", 8, 1);
}
void ChunkingTests::testNoTopstateMatch()
{
    build_and_check_chunk("testNo_Topstate_Match", 8, 1);
}

void ChunkingTests::testRepairNORTemporalConstraint()
{
    // Change to 2 chunks expected after rule repair fixed
    build_and_check_chunk("testRepair_NOR_Temporal_Constraint", 8, 3);
}

void ChunkingTests::testRHSMath()
{
    build_and_check_chunk("testRHS_Math", 8, 2);
}

void ChunkingTests::testUngroundedLTI()
{
    build_and_check_chunk("testUngrounded_LTI", 8, 2);
}

void ChunkingTests::testPromotedSTI()
{
    build_and_check_chunk("testPromoted_STI", 8, 1);
}

void ChunkingTests::testChunkRLProposal()
{
    build_and_check_chunk("testChunk_RL_Proposal", 8, 1);
}
void ChunkingTests::testNCDisjunction()
{
    build_and_check_chunk("testNC_Disjunction", 8, 1);
}
void ChunkingTests::testRHSMathMixed()
{
    build_and_check_chunk("testRHS_Math_Mixed", 8, 4);
}
void ChunkingTests::testRHSMathAbs()
{
    build_and_check_chunk("testRHS_Math_Abs", 8, 3);
}
void ChunkingTests::testReordererBadConjunction()
{
    build_and_check_chunk("testReorderer_Bad_Conjunction", 8, 1);
}
void ChunkingTests::testOpaqueStateBarrier()
{
    build_and_check_chunk("testOpaque_State_Barrier", 8, 1);
}
void ChunkingTests::testUnifyAmbiguousOutput()
{
    build_and_check_chunk("testUnify_Ambiguous_Output", 8, 1);
}
void ChunkingTests::testFauxSmemOperatorRHS()
{
    build_and_check_chunk("testFaux_Smem_Operator_RHS", 8, 1);
}
void ChunkingTests::testFauxOperator()
{
    build_and_check_chunk("testFaux_Operator", 8, 3);
}
void ChunkingTests::testSmemChunkDirect()
{
    build_and_check_chunk("testSmem_Chunk_Direct", 8, 1);
}
void ChunkingTests::testSMemChunkedQuery()
{
    /* Re-ordered doesn't seem to reorder these rules the same on different platforms, so
     * it's not detecting them as duplicates.  Setting expected to ignore to 0 until we
     * re-do the re-orderer or come up with a different solution. */
    build_and_check_chunk("testSMem_Chunked_Query", 8, 1);
}
void ChunkingTests::testResultOnOperator()
{
    build_and_check_chunk("testResult_On_Operator", 8, 1);
}
void ChunkingTests::testUnifyChildrenResults()
{
    build_and_check_chunk("testUnify_Children_Results", 8, 1);
}
void ChunkingTests::testBlocksWorldHierarchical()
{
    build_and_check_chunk("testBlocks_World_Hierarchical", 83, 16);
}
