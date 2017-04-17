//
//  ChunkingTests.hpp
//  Soar-xcode
//
//  Created by Alex Turner on 7/29/15.
//  Copyright © 2015 University of Michigan – Soar Group. All rights reserved.
//

#ifndef ChunkingDemoTests_hpp
#define ChunkingDemoTests_hpp

#include "FunctionalTestHarness.hpp"

class ChunkingDemoTests : public FunctionalTestHarness
{
    public:
        TEST_CATEGORY(ChunkingDemoTests);

        void before() { setUp(); }
        void setUp();

        void after(bool caught) { tearDown(caught); }
        void tearDown(bool caught);

        TEST(Demo_Arithmetic, -1);
        TEST(Demo_Blocks_World_Hierarchical_Look_Ahead, -1);
        TEST(Demo_Blocks_World_Hierarchical, -1);
        TEST(Demo_Blocks_World_Look_Ahead_State_Evaluation, -1);
        TEST(Demo_Blocks_World_Look_Ahead, -1);
        TEST(Demo_Blocks_World_Operator_Subgoaling, -1);
        TEST(Demo_Eight_Puzzle, -1);
        //    TEST(Demo_Graph_Search, -1);
        TEST(Demo_MaC_Planning, -1);
        TEST(Demo_RL_Unit, -1);
        TEST(Demo_ToH_Recursive, -1);
        TEST(Demo_Water_Jug_Hierarchy, -1);
        TEST(Demo_Water_Jug_Look_Ahead, -1);
        TEST(Demo_Water_Jug_Tie, -1);
//        TEST(Elio_Agent, -1);
        TEST(PRIMS_Sanity1, -1);
        TEST(PRIMS_Sanity2, -1);
        TEST(Teach_Soar_90_Games, -1);
//        TEST(Teach_Soar_9_Games, -1);


        void Demo_Arithmetic();
        void Demo_Blocks_World_Hierarchical_Look_Ahead();
        void Demo_Blocks_World_Hierarchical();
        void Demo_Blocks_World_Look_Ahead_State_Evaluation();
        void Demo_Blocks_World_Look_Ahead();
        void Demo_Blocks_World_Operator_Subgoaling();
        void Demo_Eight_Puzzle();
        void Demo_Graph_Search();
        void Demo_MaC_Planning();
        void Demo_RL_Unit();
        void Demo_ToH_Recursive();
        void Demo_Water_Jug_Hierarchy();
        void Demo_Water_Jug_Look_Ahead();
        void Demo_Water_Jug_Tie();
        void Elio_Agent();
        void PRIMS_Sanity1();
        void PRIMS_Sanity2();
        void Teach_Soar_90_Games();
        void Teach_Soar_9_Games();

    private:
        void check_chunk(const char* pTestName, int64_t decisions, int64_t expected_chunks, bool directSourceChunks = false);
        void verify_chunk(const char* pTestName, int64_t expected_chunks, bool directSourceChunks = false);
        void save_chunks(const char* path);
        void save_chunks_internal(const char* path);
        void source_saved_chunks(const char* path);
};

#endif /* ChunkingDemoTests_hpp */
