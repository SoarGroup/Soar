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

//#define SAVE_LOG_FILES
//#define TURN_EXPLAINER_ON
#define INIT_AFTER_RUN

void ChunkingTests::setUp()
{
    FunctionalTestHarness::setUp();
}

void ChunkingTests::tearDown(bool caught)
{
    FunctionalTestHarness::tearDown(caught);
}

void ChunkingTests::source(const std::string& pTestName)
{
    sml::ClientAnalyzedXML response;

    std::string sourceName = this->getCategoryName() + "_" + pTestName + ".soar";
    std::string lPath = SoarHelper::GetResource(sourceName);
    assertNonZeroSize_msg("Could not find test file '" + sourceName + "'", lPath);
    agent->ExecuteCommandLineXML(std::string("source \"" + lPath + "\"").c_str(), &response);
}

void ChunkingTests::agent_command(const char* pCmd)
{
    agent->ExecuteCommandLine(pCmd, true, false);
}

void ChunkingTests::start_log(const char* pTestName)
{
    std::string lCmdName("output log ");
    lCmdName += pTestName;
    lCmdName += "_log.txt";
    #ifdef SAVE_LOG_FILES
        agent_command(lCmdName.c_str());
    #endif
}

void ChunkingTests::continue_log(const char* pTestName)
{
    std::string lCmdName("output log -A ");
    lCmdName += pTestName;
    lCmdName += "_log.txt";
    #ifdef SAVE_LOG_FILES
        agent_command(lCmdName.c_str());
    #endif
}

void ChunkingTests::close_log()
{
    std::string lCmdName("output log -c");
    #ifdef SAVE_LOG_FILES
        agent_command(lCmdName.c_str());
    #endif
}

void ChunkingTests::save_chunks(const char* pTestName)
{
    std::string lCmdName;
    #ifdef SAVE_LOG_FILES
        lCmdName = "output command-to-file unit_test_chunks_";
        lCmdName += pTestName;
        lCmdName += ".soar print -frc";
    #else
        lCmdName = "output command-to-file unit_test_chunks.soar print -fcr";
    #endif
    agent_command(lCmdName.c_str());
}


void ChunkingTests::save_chunks_internal(const char* pTestName)
{
    std::string lCmdName;
    #ifdef SAVE_LOG_FILES
        lCmdName = "output command-to-file unit_test_chunks_";
        lCmdName += pTestName;
        lCmdName += ".soar print -frci";
    #else
        lCmdName = "output command-to-file unit_test_chunks.soar print -fcri";
    #endif
    agent_command(lCmdName.c_str());
}


void ChunkingTests::source_saved_chunks(const char* pTestName)
{
    std::string lCmdName;
    #ifdef SAVE_LOG_FILES
        lCmdName = "source unit_test_chunks_";
        lCmdName += pTestName;
        lCmdName += ".soar";
    #else
        lCmdName = "source unit_test_chunks.soar";
    #endif
    agent_command(lCmdName.c_str());
}

void ChunkingTests::check_chunk(const char* pTestName, int64_t decisions, int64_t expected_chunks, bool directSourceChunks)
{
    start_log(pTestName);
    source(pTestName);
    #ifdef TURN_EXPLAINER_ON
        agent_command("explain all on");
        agent_command("explain just on");
    #endif
    #ifdef SAVE_LOG_FILES
        agent_command("trace -CbL 2");
    #endif
    agent->RunSelf(decisions, sml::sml_DECISION);
    assertTrue_msg(agent->GetLastErrorDescription(), agent->GetLastCommandLineResult());
    #ifdef SAVE_LOG_FILES
        agent_command("chunk ?");
        agent_command("production firing-count");
        agent_command("print -cf");
    #endif
    if (!directSourceChunks)
    {
        close_log();
        save_chunks(pTestName);
        tearDown(false);
        setUp();
        continue_log(pTestName);
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
            outStringStream << "Only learned " << ignored << " of the expected " << expected_chunks << ". " << (sourced - ignored) << " were not what we expected.";
            #ifdef SAVE_LOG_FILES
            agent->ExecuteCommandLine((std::string("output log --add |") + outStringStream.str().c_str() + std::string("|")).c_str(), false, false);
            agent_command("print -cf");
            #endif
        } else {
            std::cout << " " << ignored << " ";
            #ifdef SAVE_LOG_FILES
            agent->ExecuteCommandLine("output log -a Success!!!  All expected rules were learned!!!", false, false);
            #endif
//            if (ignored > expected_chunks)
//            {
//                std::cout << std::endl << "Learned more chunks than expected! " << expected_chunks << " < " << ignored;
//            }
        }

        assertTrue_msg(outStringStream.str().c_str(), ignored >= expected_chunks);

    }

    #ifdef INIT_AFTER_RUN
        #ifdef SAVE_LOG_FILES
            agent_command("output log -a Testing re-initialization of Soar for memory leaks and crashes.");
        #endif
        agent_command("soar init");
        agent_command("trace 0");
        agent_command("run 100");
        agent_command("trace 1");
        agent_command("soar init");
    #endif
    close_log();
}
