//
//  SMemEpMemCombinedFunctionalTests.cpp
//  Prototype-UnitTesting
//
//  Created by Alex Turner on 6/23/15.
//  Copyright © 2015 University of Michigan – Soar Group. All rights reserved.
//

#include "SMemEpMemCombinedFunctionalTests.hpp"

#include <string>

void SMemEpMemCombinedFunctionalTests::testSmemEpMemFactorteizationCombinationTest()
{
	runTestSetup("testSMemEpMemFactorization");
	
	agent->RunSelf(100);
	
	std::string actualResultSMem = agent->ExecuteCommandLine("smem --print");
	
	std::string expectedResultSMem = "========================================\n            Semantic Memory             \n========================================\n(@F4 ^complete true ^factor @F5 ^number 3 [+5.000])\n(@F5 ^multiplicity 1 ^value 3 [+6.000])\n(@F12 ^complete true ^factor @F13 ^number 5 [+3.000])\n(@F13 ^multiplicity 1 ^value 5 [+4.000])\n(@F17 ^complete true ^factor @F18 ^number 7 [+7.000])\n(@F18 ^multiplicity 1 ^value 7 [+8.000])\n\n";
	
	assertTrue_msg("Unexpected output from SMem!", actualResultSMem == expectedResultSMem);
	
	std::string actualResultEpMem = agent->ExecuteCommandLine("epmem --print 97");
	
	std::string expectedResultEpMem = "========================================\n               Episode 97               \n========================================\n(<id0> ^counter 7 ^factorization-object @F17 ^has-factorization-object true ^has-factorization-object-complete true ^io <id1> ^name Factorization ^needs-factorization true ^number-to-factor 7 ^number-to-factor-int 7 ^operator* <id3> <id10> ^reward-link <id4> ^superstate nil ^svs <id2> ^type state)\n(<id1> ^input-link <id6> ^output-link <id5>)\n(<id2> ^command <id8> ^spatial-scene <id7>)\n(<id3> ^name factor-number ^number-to-factor 7)\n(<id7> ^id world)\n(<id10> ^factorization-object @F17 ^name check)\n(@F17 ^complete true ^factor @F18 ^number 7)\n(@F18 ^multiplicity 1 ^value 7)\n\n";
	
	assertTrue_msg("Unexpected output from EpMem!", actualResultEpMem == expectedResultEpMem);
}
