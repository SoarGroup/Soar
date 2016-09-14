//
//  ChunkingTests.hpp
//  Soar-xcode
//
//  Created by Alex Turner on 7/29/15.
//  Copyright © 2015 University of Michigan – Soar Group. All rights reserved.
//

#ifndef Chunkingtestshpp
#define Chunkingtestshpp

#include "FunctionalTestHarness.hpp"

class ChunkingTests : public FunctionalTestHarness
{
public:
	TEST_CATEGORY(ChunkingTests);
	
	void before() { setUp(); }
	void setUp();
	
	void after(bool caught) { tearDown(caught); }
	void tearDown(bool caught);
	
    TEST(testAllTestTypes,-1)
	void testAllTestTypes();

    TEST(testBlocksWorldHierarchical,-1)
	void testBlocksWorldHierarchical();

    TEST(testBUNCPS0,-1)
	void testBUNCPS0();

    TEST(testBUNCPS1,-1)
	void testBUNCPS1();

    TEST(testBUNCPS2,-1)
	void testBUNCPS2();

    TEST(testBUNCPS3,-1)
	void testBUNCPS3();

    TEST(testBUNCPS4,-1)
	void testBUNCPS4();

    TEST(testBUNCPS5,-1)
	void testBUNCPS5();

    TEST(testBUNCPS6FourLevel,-1)
	void testBUNCPS6FourLevel();

    TEST(testBUNCPS7withConstraints,-1)
	void testBUNCPS7withConstraints();

    TEST(testChunkRLProposal,-1)
	void testChunkRLProposal();

    TEST(testChunkedJustificationwithextras,-1)
	void testChunkedJustificationwithextras();

    TEST(testConflatedConstants,-1)
	void testConflatedConstants();

    TEST(testConstraintPropfromBaseConds,-1)
	void testConstraintPropfromBaseConds();

    TEST(testFauxOperator,-1)
	void testFauxOperator();

    /* Fails but only when run from on certain Jenkins machines*/
    TEST(testFauxSmemOperatorRHS,-1)
	void testFauxSmemOperatorRHS();

    TEST(testJustificationRCnotUngroundedSTIs,-1)
	void testJustificationRCnotUngroundedSTIs();

    TEST(testLiteralizationofNCandNCC,-1)
	void testLiteralizationofNCandNCC();

    TEST(testLiteralizationwithBTConstraints,-1)
	void testLiteralizationwithBTConstraints();

    TEST(testLiteralizationwithBTConstraints2,-1)
	void testLiteralizationwithBTConstraints2();

    TEST(testLiteralizationwithConstraints,-1)
	void testLiteralizationwithConstraints();

    TEST(testMaintainInstantiationSpecificIdentity,-1)
	void testMaintainInstantiationSpecificIdentity();

    TEST(testNCDisjunction,-1)
	void testNCDisjunction();

    TEST(testNCSimpleNoExist,-1)
	void testNCSimpleNoExist();

    TEST(testNCwithRCandLocalVariable,-1)
	void testNCwithRCandLocalVariable();

    TEST(testNCwithRelationalConstraint,-1)
	void testNCwithRelationalConstraint();

    TEST(testNCC2CondsSimpleLiterals,-1)
	void testNCC2CondsSimpleLiterals();

    TEST(testNCCComplex,-1)
	void testNCCComplex();

    TEST(testNCCfromBacktrace,-1)
	void testNCCfromBacktrace();

    TEST(testNCCSimpleLiterals,-1)
	void testNCCSimpleLiterals();

    TEST(testNCCwithRelationalConstraint,-1)
	void testNCCwithRelationalConstraint();

    TEST(testNoTopstateMatch,-1)
	void testNoTopstateMatch();

    TEST(testOpaqueStateBarrier,-1)
	void testOpaqueStateBarrier();

    TEST(testPromotedSTI,-1)
	void testPromotedSTI();

    TEST(testReordererBadConjunction,-1)
	void testReordererBadConjunction();

    TEST(testRepairNORTemporalConstraint,-1)
	void testRepairNORTemporalConstraint();

    TEST(testResultOnOperator,-1)
	void testResultOnOperator();

    TEST(testReteBugDeepvsDeep,-1)
	void testReteBugDeepvsDeep();

    TEST(testReteBugDeepvsTop,-1)
	void testReteBugDeepvsTop();

    TEST(testRHSMathAbs,-1)
	void testRHSMathAbs();

    TEST(testRHSMathMixed,-1)
	void testRHSMathMixed();

    TEST(testRHSMath,-1)
	void testRHSMath();

    TEST(testRHSUnboundMultivalue,-1)
	void testRHSUnboundMultivalue();

    TEST(testRLVariablization,-1)
	void testRLVariablization();

    TEST(testSimpleConstraintProp,-1)
	void testSimpleConstraintProp();

    TEST(testSimpleLiteralization,-1)
	void testSimpleLiteralization();

    TEST(testSmemChunkDirect,-1)
	void testSmemChunkDirect();

    /* Fails but only when run from unit test and not always */
    TEST(testSMemChunkedQuery,-1)
	void testSMemChunkedQuery();

    TEST(testSTIVariablizationSameType,-1)
	void testSTIVariablizationSameType();

    TEST(testSTIVariablization,-1)
	void testSTIVariablization();

    TEST(testSTIwithreferents,-1)
	void testSTIwithreferents();

    TEST(testSuperstateIdentityOpaque,-1)
	void testSuperstateIdentityOpaque();

    TEST(testLearn,-1)
	void testLearn();
   // bug 1145
    TEST(testUngroundedinBTConstraint,-1)
	void testUngroundedinBTConstraint();

    TEST(testUngroundedLTI,-1)
	void testUngroundedLTI();

    TEST(testUngroundedMixed,-1)
	void testUngroundedMixed();

    TEST(testUngroundedRelationalConstraint,-1)
	void testUngroundedRelationalConstraint();

    TEST(testUngroundedSTIPromotion,-1)
	void testUngroundedSTIPromotion();

    TEST(testUngroundedSTIs,-1)
	void testUngroundedSTIs();

    TEST(testUnifyAmbiguousOutput,-1)
	void testUnifyAmbiguousOutput();

    TEST(testUnifyChildrenResults,-1)
	void testUnifyChildrenResults();

    TEST(testUnifythroughTwoTracesFourDeep,-1)
	void testUnifythroughTwoTracesFourDeep();

    TEST(testVrblzdConstraintonUngrounded,-1)
	void testVrblzdConstraintonUngrounded();


private:
	void build_and_check_chunk(const std::string& path, int decisions, int expectedchunks);
};

#endif /* Chunkingtestshpp */
