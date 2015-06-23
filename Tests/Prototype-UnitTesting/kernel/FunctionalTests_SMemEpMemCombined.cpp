//
//  FunctionalTests_SMemEpMemCombined.cpp
//  Soar-xcode
//
//  Created by Alex Turner on 6/23/15.
//  Copyright © 2015 University of Michigan – Soar Group. All rights reserved.
//

#include "FunctionalTests_SMemEpMemCombined.hpp"

#include <string>

void SMemEpMemCombinedFunctionalTests::smemEpMemFactorizationCombinationTest()
{
	runTestSetup("testSMemEpMemFactorization");
	
	agent->RunSelf(100);
	
	std::string actualResultSMem = agent->ExecuteCommandLine("smem --print");
	
	std::string expectedResultSMem = R"expected(
========================================
			Semantic Memory
========================================
(@F4 ^complete |true| ^number 3 ^factor @F5 [+5.0])
(@F5 ^value 3 ^multiplicity 1 [+6.0])
(@F12 ^complete |true| ^number 5 ^factor @F13 [+3.0])
(@F13 ^value 5 ^multiplicity 1 [+4.0])
(@F17 ^complete |true| ^number 7 ^factor @F18 [+7.0])
(@F18 ^value 7 ^multiplicity 1 [+8.0])
)expected";
	
	assertTrue("Unexpected output from SMem!", actualResultSMem == expectedResultSMem);
	
	std::string actualResultEpMem = agent->ExecuteCommandLine("epmem --print 97");
	
	std::string expectedResultEpMem = R"expected(
(<id0> ^counter 7 ^factorization-object @F17 ^has-factorization-object true ^has-factorization-object-complete true ^io <id1> ^name Factorization ^needs-factorization true ^number-to-factor 7 ^number-to-factor-int 7 ^operator* <id3> <id7> ^reward-link <id2> ^superstate nil ^type state)
(<id1> ^input-link <id5> ^output-link <id4>)
(<id3> ^name factor-number ^number-to-factor 7)
(<id7> ^factorization-object @F17 ^name check)
(@F17 ^complete true ^factor @F18 ^number 7)
(@F18 ^multiplicity 1 ^value 7)
)expected";
	
	assertTrue("Unexpected output from EpMem!", actualResultEpMem == expectedResultEpMem);
}
