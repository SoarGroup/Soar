//
//  SMemFunctionalTests.cpp
//  Prototype-UnitTesting
//
//  Created by Alex Turner on 6/23/15.
//  Copyright © 2015 University of Michigan – Soar Group. All rights reserved.
//

#include "SMemFunctionalTests.hpp"

#include "SoarHelper.hpp"

#include "symbol_manager.h"
#include "working_memory.h"

void SMemFunctionalTests::testSimpleCueBasedRetrieval()
{
	SoarHelper::setStopPhase(agent, SoarHelper::StopPhase::OUTPUT);
	runTest("testSimpleCueBasedRetrieval", -1);
}

void SMemFunctionalTests::testSimpleNonCueBasedRetrieval()
{
	SoarHelper::setStopPhase(agent, SoarHelper::StopPhase::OUTPUT);
	runTest("testSimpleNonCueBasedRetrieval", 2);
}

void SMemFunctionalTests::testSimpleStore()
{
	SoarHelper::setStopPhase(agent, SoarHelper::StopPhase::OUTPUT);
	runTest("testSimpleStore", 2);
}

void SMemFunctionalTests::testTrivialMathQuery()
{
	SoarHelper::setStopPhase(agent, SoarHelper::StopPhase::OUTPUT);
	runTest("testTrivialMathQuery", 2);
}

void SMemFunctionalTests::testBadMathQuery()
{
	SoarHelper::setStopPhase(agent, SoarHelper::StopPhase::OUTPUT);
	runTest("testBadMathQuery", 2);
}

void SMemFunctionalTests::testMaxQuery()
{
	SoarHelper::setStopPhase(agent, SoarHelper::StopPhase::OUTPUT);
	runTest("testMax", 1);
}


void SMemFunctionalTests::testMaxMixedTypes()
{
	SoarHelper::setStopPhase(agent, SoarHelper::StopPhase::OUTPUT);
	runTest("testMaxMixedTypes", 1);
}

void SMemFunctionalTests::testMaxMultivalued()
{
	SoarHelper::setStopPhase(agent, SoarHelper::StopPhase::OUTPUT);
	runTest("testMaxMultivalued", 1);
}

void SMemFunctionalTests::testMin()
{
	SoarHelper::setStopPhase(agent, SoarHelper::StopPhase::OUTPUT);
	runTest("testMin", 1);
}

void SMemFunctionalTests::testMaxNegQuery()
{
	SoarHelper::setStopPhase(agent, SoarHelper::StopPhase::OUTPUT);
	runTest("testMaxNegation", 1);
}

void SMemFunctionalTests::testGreater()
{
	SoarHelper::setStopPhase(agent, SoarHelper::StopPhase::OUTPUT);
	runTest("testGreater", 1);
}

void SMemFunctionalTests::testLess()
{
	SoarHelper::setStopPhase(agent, SoarHelper::StopPhase::OUTPUT);
	runTest("testLess", 1);
}

void SMemFunctionalTests::testGreaterOrEqual()
{
	SoarHelper::setStopPhase(agent, SoarHelper::StopPhase::OUTPUT);
	runTest("testGreaterOrEqual", 1);
}

void SMemFunctionalTests::testLessOrEqual()
{
	SoarHelper::setStopPhase(agent, SoarHelper::StopPhase::OUTPUT);
	runTest("testLessOrEqual", 1);
}

void SMemFunctionalTests::testLessWithNeg()
{
	SoarHelper::setStopPhase(agent, SoarHelper::StopPhase::OUTPUT);
	runTest("testLessWithNeg", 1);
}

void SMemFunctionalTests::testLessNoSolution()
{
	SoarHelper::setStopPhase(agent, SoarHelper::StopPhase::OUTPUT);
	runTest("testLessNoSolution", 1);
}

void SMemFunctionalTests::testSimpleStoreMultivaluedAttribute()
{
	SoarHelper::setStopPhase(agent, SoarHelper::StopPhase::OUTPUT);
	runTest("testSimpleStoreMultivaluedAttribute", 2);
}

void SMemFunctionalTests::testSimpleFloat()
{
	runTest("testSimpleFloat", 5);
}

void SMemFunctionalTests::testMaxDoublePrecision_Irrational()
{
	runTest("testMaxDoublePrecision-Irrational", 5);
}

void SMemFunctionalTests::testMaxDoublePrecision()
{
	runTest("testMaxDoublePrecision", 5);
}

void SMemFunctionalTests::testSimpleNonCueBasedRetrievalOfNonExistingLTI()
{
	runTest("testSimpleNonCueBasedRetrievalOfNonExistingLTI", 1);
}

void SMemFunctionalTests::testNegQuery()
{
	runTest("testNegQuery", 248);
}

void SMemFunctionalTests::testNegStringFloat()
{
	runTest("testNegStringFloat", 5);
}

void SMemFunctionalTests::testNegQueryNoHash()
{
	runTest("testNegQueryNoHash", 1);
}

void SMemFunctionalTests::testCueSelection()
{
	runTestSetup("testCueSelection");

	std::string result = agent->RunSelf(2);

	agent::BasicWeightedCue* bwc = internal_agent->lastCue;
	assertTrue_msg("Incorrect cue selected",
			   bwc &&
			   (std::string(bwc->cue->attr->to_string()) == "name") &&
			   (std::string(bwc->cue->value->to_string()) == "val") &&
			   bwc->weight == 4);
}

void SMemFunctionalTests::testSimpleNonCueBasedRetrieval_ActivationRecency()
{
	runTestSetup("testSimpleNonCueBasedRetrieval_ActivationRecency");

	std::string result = agent->RunSelf(3);

	assertTrue_msg("testSimpleNonCueBasedRetrieval_ActivationRecency functional test did not halt", halted);

	std::string expected = "(@1 ^x 1 ^y 2 ^z 3 [+2.000])\n(@2 ^location @1 ^name foo [+1.000])\n(@3 ^x 2 ^y 3 ^z 1 [+0.000])\n(@4 ^location @3 ^name bar [+0.000])\n";

	result = agent->ExecuteCommandLine("print @");

	assertTrue_msg("testSimpleNonCueBasedRetrieval_ActivationRecency: Invalid Activation Values\nString did not match:\n" "(@1 ^x 1 ^y 2 ^z 3 [+2.000])\n(@2 ^location @1 ^name foo [+1.000])\n(@3 ^x 2 ^y 3 ^z 1 [+0.000])\n(@4 ^location @3 ^name bar [+0.000])\n", result == expected);
}

void SMemFunctionalTests::testSimpleNonCueBasedRetrieval_ActivationRecency_WithoutActivateOnQuery()
{
	runTestSetup("testSimpleNonCueBasedRetrieval_ActivationRecency_WithoutActivateOnQuery");

	std::string result = agent->RunSelf(3);

	assertTrue_msg("testSimpleNonCueBasedRetrieval_ActivationRecency_WithoutActivateOnQuery functional test did not halt", halted);

	std::string expected = "(@1 ^x 1 ^y 2 ^z 3 [+1.000])\n(@2 ^location @1 ^name foo [+0.000])\n(@3 ^x 2 ^y 3 ^z 1 [+0.000])\n(@4 ^location @3 ^name bar [+0.000])\n";

	result = agent->ExecuteCommandLine("print @");

	assertTrue_msg("testSimpleNonCueBasedRetrieval_ActivationRecency_WithoutActivateOnQuery: Invalid Activation Values\nString did not match:\n(@1 ^x 1 ^y 2 ^z 3 [+1.000])\n(@2 ^location @1 ^name foo [+0.000])\n(@3 ^x 2 ^y 3 ^z 1 [+0.000])\n(@4 ^location @3 ^name bar [+0.000])\n", result == expected);
}

void SMemFunctionalTests::testSimpleNonCueBasedRetrieval_ActivationFrequency()
{
	runTestSetup("testSimpleNonCueBasedRetrieval_ActivationFrequency");

	std::string result = agent->RunSelf(3);

	assertTrue_msg("testSimpleNonCueBasedRetrieval_ActivationFrequency functional test did not halt", halted);

	std::string expected = "(@1 ^x 1 ^y 2 ^z 3 [+1.000])\n(@2 ^location @1 ^name foo [+1.000])\n(@3 ^x 2 ^y 3 ^z 1 [+0.000])\n(@4 ^location @3 ^name bar [+0.000])\n";

	result = agent->ExecuteCommandLine("print @");

	assertTrue_msg("testSimpleNonCueBasedRetrieval_ActivationFrequency: Invalid Activation Values\n String did not match:\n(@1 ^x 1 ^y 2 ^z 3 [+1.000])\n(@2 ^location @1 ^name foo [+1.000])\n(@3 ^x 2 ^y 3 ^z 1 [+0.000])\n(@4 ^location @3 ^name bar [+0.000])\n", result == expected);
}

bool SMemFunctionalTests::checkActivationValues(std::string activationString, std::vector<double> lowEndExpectations, std::vector<double> highEndExpectations, const char* file, const int line)
{
	std::vector<std::string> activationLevels;
	std::string activation = "";
	bool inActivationParse = false;

	for (char c : activationString)
	{
		if (c == '[')
		{
			inActivationParse = true;
			continue;
		}
		else if (c == ']' && inActivationParse)
		{
			inActivationParse = false;
			activationLevels.push_back(activation);

			activation = "";

			continue;
		}

		if (inActivationParse && (isdigit(c) || c == '.' || c == '+' || c == '-'))
		{
			if (activation.length() != 0 &&
				(c == '+' || c == '-'))
			{
				throw SoarAssertionException("Found a +/- where there shouldn't be in Activation Levels!", file, line);
			}

			activation += c;
		}
		else if (inActivationParse)
		{
			throw SoarAssertionException("Non-Digit Character in Activation Level", file, line);
		}
	}

	if (activationLevels.size() != lowEndExpectations.size())
	{
		throw SoarAssertionException("Low End Expectations is not the same size as parsed Activation Levels!", file, line);
	}
	else if (activationLevels.size() != highEndExpectations.size())
	{
		throw SoarAssertionException("High End Expectations is not the same size as parsed Activation Levels!", file, line);
	}

	std::vector<double> activationLevelsAsDoubles;

	for (std::string a : activationLevels)
	{
		std::stringstream ss(a);
		double result;
		ss >> result;
		activationLevelsAsDoubles.push_back(result);
	}

	for (size_t i = 0;i < activationLevelsAsDoubles.size();i++)
	{
		double a = activationLevelsAsDoubles[i];

		if (!(a >= lowEndExpectations[i] && a <= highEndExpectations[i]))
		{
			std::stringstream ss;
			ss << "Parsed Activation " << i+1 << " (" << a << ") is not within [" << lowEndExpectations[i] << ", " << highEndExpectations[i] << "]";
			throw SoarAssertionException(ss.str(), file, line);
		}
	}

	return true;
}

void SMemFunctionalTests::testSimpleNonCueBasedRetrieval_ActivationBaseLevel_Stable()
{
	runTestSetup("testSimpleNonCueBasedRetrieval_ActivationBaseLevel_Stable");

	agent->RunSelf(6);
    std::string test_smem = agent->ExecuteCommandLine("smem");

	assertTrue_msg("testSimpleNonCueBasedRetrieval_ActivationBaseLevel_Stable functional test did not halt", halted);

	std::vector<double> lowEndExpectations;
	std::vector<double> highEndExpectations;

	lowEndExpectations.push_back(0.0);
	highEndExpectations.push_back(0.0);

	lowEndExpectations.push_back(0.534);
	highEndExpectations.push_back(0.535);

	lowEndExpectations.push_back(0.0);
	highEndExpectations.push_back(0.0);

	lowEndExpectations.push_back(0.534);
	highEndExpectations.push_back(0.535);

 //agent->ExecuteCommandLine("print @");

	// This is the expected output from print @ modified from CSoar to look like JSoar outputs it (reverse string attributes)
//	std::string expected = R"raw(
//
//========================================
//            Semantic Memory
//========================================
//(@L1 ^x 1 ^y 2 ^z 3 [+0.0])
//(@L2 ^x 2 ^y 3 ^z 1 [+0.456])
//(@X1 ^name |foo| ^location @L1 [+0.0])
//(@X2 ^name |foo| ^location @L2 [+0.456])
//
//)raw";

	std::string result = agent->ExecuteCommandLine("print @");

	assertTrue_msg("testSimpleNonCueBasedRetrieval_ActivationBaseLevel_Stable: Invalid Activation Values", checkActivationValues(result, lowEndExpectations, highEndExpectations, __FILE__, __LINE__));
}

void SMemFunctionalTests::testSimpleNonCueBasedRetrieval_ActivationBaseLevel_Naive()
{
	runTestSetup("testSimpleNonCueBasedRetrieval_ActivationBaseLevel_Naive");

	agent->RunSelf(6);

	assertTrue_msg("testSimpleNonCueBasedRetrieval_ActivationBaseLevel_Naive functional test did not halt", halted);

	std::vector<double> lowEndExpectations;
	std::vector<double> highEndExpectations;

	lowEndExpectations.push_back(0.0);
	highEndExpectations.push_back(0.0);

	lowEndExpectations.push_back(0.791);
	highEndExpectations.push_back(0.792);

	lowEndExpectations.push_back(0.0);
	highEndExpectations.push_back(0.0);

	lowEndExpectations.push_back(-0.347);
	highEndExpectations.push_back(-0.346);

	// This is the expected output from print @ modified from CSoar to look like JSoar outputs it (reverse string attributes)
//	std::string expected = R"raw(
//
//========================================
//            Semantic Memory
//========================================
//(@L1 ^x 1 ^y 2 ^z 3 [+0.0])
//(@L2 ^x 2 ^y 3 ^z 1 [+0.456])
//(@X1 ^name |foo| ^location @L1 [-0.693])
//(@X2 ^name |foo| ^location @L2 [+0.456])
//
//)raw";

	std::string result = agent->ExecuteCommandLine("print @");

	assertTrue_msg("testSimpleNonCueBasedRetrieval_ActivationBaseLevel_Naive: Invalid Activation Values", checkActivationValues(result, lowEndExpectations, highEndExpectations, __FILE__, __LINE__));
}

void SMemFunctionalTests::testSimpleNonCueBasedRetrieval_ActivationBaseLevel_Incremental()
{
	runTestSetup("testSimpleNonCueBasedRetrieval_ActivationBaseLevel_Incremental");

	agent->RunSelf(6);

	assertTrue_msg("testSimpleNonCueBasedRetrieval_ActivationBaseLevel_Incremental functional test did not halt", halted);

	std::vector<double> lowEndExpectations;
	std::vector<double> highEndExpectations;

	lowEndExpectations.push_back(0.0);
	highEndExpectations.push_back(0.0);

	lowEndExpectations.push_back(0.791);
	highEndExpectations.push_back(0.792);

	lowEndExpectations.push_back(0.0);
	highEndExpectations.push_back(0.0);

	lowEndExpectations.push_back(-0.550);
	highEndExpectations.push_back(-0.549);

	// This is the expected output from print @ modified from CSoar to look like JSoar outputs it (reverse string attributes)
//	std::string expected = R"raw(
//
//========================================
//            Semantic Memory
//========================================
//(@L1 ^x 1 ^y 2 ^z 3 [-0.347])
//(@L2 ^x 2 ^y 3 ^z 1 [+0.405])
//(@X1 ^name |foo| ^location @L1 [+0.109])
//(@X2 ^name |foo| ^location @L2 [+0.144])
//
//)raw";

	std::string result = agent->ExecuteCommandLine("print @");

	assertTrue_msg("testSimpleNonCueBasedRetrieval_ActivationBaseLevel_Incremental: Invalid Activation Values", checkActivationValues(result, lowEndExpectations, highEndExpectations, __FILE__, __LINE__));
}

void SMemFunctionalTests::testDbBackupAndLoadTests()
{
	runTestSetup("testFactorization");

    agent->RunSelf(1223, sml::sml_DECISION);

	std::string resultOfPS1 = agent->ExecuteCommandLine("p s1");

	// NOTE: This is because of an ordering difference between Windows & Linux/OS X.  Functionally,
	// it doesn't matter so this is a #def because effort was considered to be better spent elsewhere
	// than finding out why.
#ifndef _MSC_VER
	std::string expectedResultOfPS1 = "(S1 ^counter 50 ^epmem E1 ^io I1 ^name Factorization ^operator O1385\n       ^operator O1385 + ^reward-link R1 ^smem S2 ^superstate nil ^svs S3\n       ^type state ^using-smem true)\n";
#else
	std::string expectedResultOfPS1 = "(S1 ^counter 50 ^epmem E1 ^io I1 ^name Factorization ^operator O1385 +\n       ^operator O1385 ^reward-link R1 ^smem S2 ^superstate nil ^svs S3\n       ^type state ^using-smem true)\n";
#endif

	assertTrue_msg("Didn't stop where expected!", resultOfPS1 == expectedResultOfPS1);

	agent->ExecuteCommandLine("smem --backup backup.sqlite");
	agent->ExecuteCommandLine("smem --init");

	std::string resultOfP = agent->ExecuteCommandLine("p");

	//assertTrue_msg("smem --init didn't excise all productions!", resultOfP.length() == 0);

	resultOfPS1 = agent->ExecuteCommandLine("p s1");

	expectedResultOfPS1 = "(S1 ^epmem E1 ^io I1 ^reward-link R1 ^smem S2 ^superstate nil ^svs S3\n       ^type state)\n";

	//assertTrue_msg("smem --init didn't reinit WM!", resultOfPS1 == expectedResultOfPS1);

	agent->ExecuteCommandLine("smem --set path backup.sqlite");
	agent->ExecuteCommandLine("smem --set database file");
	agent->ExecuteCommandLine("smem --set append on");
	agent->ExecuteCommandLine("smem --init");

	runTestSetup("testFactorization");

	agent->RunSelf(2811 + 1);

	assertTrue_msg("testFactorization: Test did not halt.", halted);

	std::string resultOfPD2F197 = agent->ExecuteCommandLine("p @197 -d 2");
#ifndef _MSC_VER
    std::string expectedResultOfPD2F197 = "(@197 ^complete true ^factor @48 @198 ^number 100 [+447.000])\n  (@198 ^multiplicity 2 ^value 2 [+448.000])\n  (@48 ^multiplicity 2 ^value 5 [+449.000])\n";
#else
    std::string expectedResultOfPD2F197 = "(@197 ^complete true ^factor @48 @198 ^number 100 [+447.000])\n  (@48 ^multiplicity 2 ^value 5 [+449.000])\n  (@198 ^multiplicity 2 ^value 2 [+448.000])\n";
#endif
	assertTrue_msg("testFactorization: Test did not get the correct result!", expectedResultOfPD2F197 == resultOfPD2F197);
	
	std::string pwd = agent->ExecuteCommandLine("pwd");
	remove((pwd + "/backup.sqlite").c_str());
}

void SMemFunctionalTests::testReadCSoarDB()
{
	agent->InitSoar();

	std::string db = SoarHelper::GetResource("smem-csoar-db.sqlite");
	assertNonZeroSize_msg("No CSoar db!", db);
	agent->ExecuteCommandLine(std::string("smem --set path \"" + db + "\"").c_str());
	agent->ExecuteCommandLine("smem --set database file");
	agent->ExecuteCommandLine("smem --set append-database on");
	agent->ExecuteCommandLine("smem --init");

	std::string actualResult = agent->ExecuteCommandLine("print @1");

	std::string expectedResult = "(@1 ^complete true ^factor @2 ^number 2 [+0.000])\n(@2 ^multiplicity 1 ^value 2 [+0.000])\n(@3 ^complete true ^factor @4 ^number 3 [+0.000])\n(@4 ^multiplicity 1 ^value 3 [+0.000])\n(@5 ^complete true ^factor @6 ^number 4 [+0.000])\n(@6 ^multiplicity 2 ^value 2 [+0.000])\n";

	assertTrue_msg("Unexpected output from CSoar database!", actualResult == expectedResult);
}

void SMemFunctionalTests::testMultiAgent()
{
	std::vector<sml::Agent*> agents;

	for (int i = 1;i <= 250;i++)
	{
		std::stringstream ss("Agent ");
		ss << i;

		sml::Agent* t = kernel->CreateAgent(ss.str().c_str());
		std::string sourceName = getCategoryName() + "_testMultiAgent.soar";
		std::string sourceUrl = SoarHelper::GetResource(sourceName);
		assertNonZeroSize_msg("Could not find test file " + sourceName, sourceUrl);
		agent->ExecuteCommandLine(("source " + sourceUrl).c_str());

		agents.push_back(t);
	}

	kernel->RunAllAgents(3);

	for (auto a : agents)
	{
		try
		{
			if (SoarHelper::getDecisionPhasesCount(a) != 3)
			{
				throw SoarAssertionException("Agent did not stop correctly! Ran too many cycles!", __FILE__, __LINE__);
			}

			std::string result = a->ExecuteCommandLine("epmem");

			if (result.find("memory") == std::string::npos)
			{
				throw SoarAssertionException("Non Memory Driver!", __FILE__, __LINE__);
			}
		}
		catch (SoarAssertionException& e)
		{
			kernel->DestroyAgent(a);
			throw e;
		}

		kernel->DestroyAgent(a);
	}
}

void SMemFunctionalTests::testISupport()
{
	runTest("smem-i-support", 6);
}

void SMemFunctionalTests::testISupportWithLearning()
{
	std::string result = agent->ExecuteCommandLine("chunk always") ;

	runTest("smem-i-support", 6);
}

void SMemFunctionalTests::testSmemArithmetic()
{
    agent->ExecuteCommandLine("watch 0");
    agent->ExecuteCommandLine("srand 1080");

    runTest("arithmetic", 46436);
}
