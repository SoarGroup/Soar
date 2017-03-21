//
//  ChunkingDemoTests.cpp
//  Soar-xcode
//
//  Created by Alex Turner on 7/29/15.
//  Copyright © 2015 University of Michigan – Soar Group. All rights reserved.
//

#include "ChunkingDemoTests.hpp"

#include "sml_ClientAnalyzedXML.h"

#include "SoarHelper.hpp"

/* Note that some test don't get as many successful learned chunks as expected because Soar is not
 * able to detect they're duplicates using the sourcing mechanism these tests use to verify chunk contents.  */

void ChunkingDemoTests::Demo_Arithmetic()                                 { check_chunk("Demo_Arithmetic", 2810, 29); }
void ChunkingDemoTests::Demo_Blocks_World_Hierarchical_Look_Ahead()       { check_chunk("Demo_Blocks_World_Hierarchical_Look_Ahead", 47, 1); }
void ChunkingDemoTests::Demo_Blocks_World_Hierarchical()                  { check_chunk("Demo_Blocks_World_Hierarchical", 24, 20); }
void ChunkingDemoTests::Demo_Blocks_World_Look_Ahead_State_Evaluation()   { check_chunk("Demo_Blocks_World_Look_Ahead_State_Evaluation", 60, 34); }
void ChunkingDemoTests::Demo_Blocks_World_Look_Ahead()                    { check_chunk("Demo_Blocks_World_Look_Ahead", 65, 11); }
void ChunkingDemoTests::Demo_Blocks_World_Operator_Subgoaling()           { check_chunk("Demo_Blocks_World_Operator_Subgoaling", 6, 1); }
void ChunkingDemoTests::Demo_Eight_Puzzle()                               { check_chunk("Demo_Eight_Puzzle", 20, 7); }
void ChunkingDemoTests::Demo_Graph_Search()                               { check_chunk("Demo_Graph_Search", 20, 7); }
void ChunkingDemoTests::Demo_MaC_Planning()                               { check_chunk("Demo_MaC_Planning", 122, 34); }
void ChunkingDemoTests::Demo_RL_Unit()                                    { check_chunk("Demo_RL_Unit", 26, 6); }
void ChunkingDemoTests::Demo_ToH_Recursive()                              { check_chunk("Demo_ToH_Recursive", 23, 10); }
void ChunkingDemoTests::Demo_Water_Jug_Hierarchy()                        { check_chunk("Demo_Water_Jug_Hierarchy", 99, 3); }
void ChunkingDemoTests::Demo_Water_Jug_Look_Ahead()                       { check_chunk("Demo_Water_Jug_Look_Ahead", 102, 16); }
void ChunkingDemoTests::Demo_Water_Jug_Tie()                              { check_chunk("Demo_Water_Jug_Tie", 21, 5); }
void ChunkingDemoTests::Elio_Agent()                                      { check_chunk("Elio_Agent", 795, 135); }
void ChunkingDemoTests::PRIMS_Sanity1()                                   { check_chunk("PRIMS_Sanity1", 795, 23); }
void ChunkingDemoTests::PRIMS_Sanity2()                                   { check_chunk("PRIMS_Sanity2", 728, 19); }
void ChunkingDemoTests::Teach_Soar_90_Games()                             { check_chunk("Teach_Soar_90_Games", 10000, 16, true); } /* Probably re-ordering problems.  The rules learned are huge */

void ChunkingDemoTests::setUp()
{
    FunctionalTestHarness::setUp();
}

void ChunkingDemoTests::tearDown(bool caught)
{
    FunctionalTestHarness::tearDown(caught);
}

void ChunkingDemoTests::save_chunks(const char* pTestName)
{
    std::string lCmdName;
    #ifdef SAVE_LOG_FILES
        lCmdName = "output command-to-file ";
        SoarHelper::add_log_dir_if_exists(lCmdName);
        lCmdName += "temp_chunks_";
        lCmdName += pTestName;
        lCmdName += ".soar print -frc";
    #else
        lCmdName = "output command-to-file temp_chunks.soar print -fcr";
    #endif
    SoarHelper::agent_command(agent,lCmdName.c_str());
}


void ChunkingDemoTests::save_chunks_internal(const char* pTestName)
{
    std::string lCmdName;
    #ifdef SAVE_LOG_FILES
        lCmdName = "output command-to-file ";
        SoarHelper::add_log_dir_if_exists(lCmdName);
        lCmdName += "temp_chunks_";
        lCmdName += pTestName;
        lCmdName += ".soar print -frci";
    #else
        lCmdName = "output command-to-file temp_chunks.soar print -fcri";
    #endif
    SoarHelper::agent_command(agent,lCmdName.c_str());
}

void ChunkingDemoTests::source_saved_chunks(const char* pTestName)
{
    std::string lCmdName;
    #ifdef SAVE_LOG_FILES
        lCmdName = "source ";
        SoarHelper::add_log_dir_if_exists(lCmdName);
        lCmdName += "temp_chunks_";
        lCmdName += pTestName;
        lCmdName += ".soar";
    #else
        lCmdName = "source temp_chunks.soar";
    #endif
    SoarHelper::agent_command(agent,lCmdName.c_str());
}

void ChunkingDemoTests::check_chunk(const char* pTestName, int64_t decisions, int64_t expected_chunks, bool directSourceChunks)
{
    SoarHelper::start_log(agent, pTestName);
    assertTrue_msg("Could not find " + this->getCategoryName() + " test file '" + pTestName + "'", SoarHelper::source(agent, this->getCategoryName(), pTestName));
    #ifdef TURN_EXPLAINER_ON
        SoarHelper::agent_command(agent,"explain all on");
        SoarHelper::agent_command(agent,"explain just on");
    #endif
//    #ifdef SAVE_LOG_FILES
//        SoarHelper::agent_command(agent,"trace -CbL 2");
//    #endif
    SoarHelper::check_learning_override(agent);
    agent->RunSelf(decisions, sml::sml_DECISION);
    assertTrue_msg(agent->GetLastErrorDescription(), agent->GetLastCommandLineResult());

    SoarHelper::init_check_to_find_refcount_leaks(agent);
    verify_chunk(pTestName, expected_chunks, directSourceChunks);
}

void ChunkingDemoTests::verify_chunk(const char* pTestName, int64_t expected_chunks, bool directSourceChunks)
{
    #ifdef SAVE_LOG_FILES
    SoarHelper::agent_command(agent,"chunk ?");
    SoarHelper::agent_command(agent,"production firing-count");
    SoarHelper::agent_command(agent,"print -cf");
    #endif
    if (!directSourceChunks)
    {
        SoarHelper::close_log(agent);
        save_chunks(pTestName);
        tearDown(false);
        setUp();
        SoarHelper::continue_log(agent, pTestName);
        source_saved_chunks(pTestName);
    } else {
        SoarHelper::close_log(agent);
        save_chunks_internal(pTestName);
        tearDown(false);
        setUp();
        SoarHelper::continue_log(agent, pTestName);
        source_saved_chunks(pTestName);
    }
    {
        sml::ClientAnalyzedXML response;

        std::string sourceName = this->getCategoryName() + "_" + pTestName + "_expected" + ".soar";

        std::string lPath = SoarHelper::GetResource(sourceName);
        assertNonZeroSize_msg("Could not find test file '" + sourceName + "'", lPath);

        agent->ExecuteCommandLineXML(std::string("source \"" + lPath + "\"").c_str(), &response);
        int sourced, excised, ignored;
        ignored = response.GetArgInt(sml::sml_Names::kParamIgnoredProductionCount, -1);
        sourced = response.GetArgInt(sml::sml_Names::kParamSourcedProductionCount, -1);
        excised = response.GetArgInt(sml::sml_Names::kParamExcisedProductionCount, -1);
        std::ostringstream outStringStream("");
        if (ignored < expected_chunks)
        {
            outStringStream << "Only learned " << ignored << " of the expected " << expected_chunks << ".";
            #ifdef SAVE_LOG_FILES
            agent->ExecuteCommandLine((std::string("output log --add |") + outStringStream.str().c_str() + std::string("|")).c_str(), false, false);
            SoarHelper::agent_command(agent,"print -cf");
            #endif
        } else {
            std::cout << " " << ignored << "/" << expected_chunks << " ";
            #ifdef SAVE_LOG_FILES
            agent->ExecuteCommandLine("output log -a Success!!!  All expected rules were learned!!!", false, false);
            #endif
        }
        assertTrue_msg(outStringStream.str().c_str(), ignored >= expected_chunks);
    }
    SoarHelper::close_log(agent);
}
