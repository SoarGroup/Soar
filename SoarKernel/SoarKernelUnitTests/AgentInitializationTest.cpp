#include "AgentTestSuite.h"
#include "AgentInitializationTest.h"

#include "wmem.h"
#include "symtab.h"

#include <cppunit/extensions/Orthodox.h>
#include <cppunit/TestResult.h>

CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( AgentInitializationTest,
                                       getAgentTestSuiteName() );

AgentInitializationTest::AgentInitializationTest():m_kernel(0),m_agent(0) {}
AgentInitializationTest::~AgentInitializationTest() {}

// Setting up a kernel and agent for all the tests in this section
void AgentInitializationTest::setUp() {
   m_kernel = create_kernel();
   m_agent = create_soar_agent(m_kernel, "SampleTestAgent");
   initialize_soar_agent(m_kernel, m_agent);
}

// Cleaning up after the tests have completed
void AgentInitializationTest::tearDown() {
   destroy_soar_agent(m_kernel, m_agent);
   destroy_kernel(m_kernel);
}

// Does the created agent have the correct name
void AgentInitializationTest::testAgentName() {
   // Some very simple agent initialization tests.
   CPPUNIT_ASSERT(!strcmp(m_agent->name,"SampleTestAgent"));
}

// Testing the initial state of the productions
void AgentInitializationTest::testAgentProductions() {
   // There should be no productions in a newly created agent
   for (int i=0; i < NUM_PRODUCTION_TYPES; i++) {
      CPPUNIT_ASSERT(m_agent->num_productions_of_type[i] == 0);
      CPPUNIT_ASSERT(m_agent->all_productions_of_type[i] == 0);
   }
}

void AgentInitializationTest::testAgentWMState() {

   // There should be 5 wmes immediately after initialization
   //
   // S1 ^type state
   // S1 ^superstate nil
   // S1 ^io I1
   // I1 ^input-link I2
   // I1 ^output-link I3
   //
   CPPUNIT_ASSERT(m_agent->wme_addition_count == 5);
   CPPUNIT_ASSERT(m_agent->wme_removal_count == 0);
   CPPUNIT_ASSERT(m_agent->num_existing_wmes == 5);

   // What is the max_wm_size mean (looks like the maximum number
   // of wme's that have ever been in the agent's working memory
   // 
   // It is updated during the huge do_one_top_level_phase function
   // and isn't correct right after initialization.
   //
   // TODO: Fix this variable so that it is always correct (after
   // a function call is complete).
   //CPPUNIT_ASSERT(m_agent->max_wm_size == 5);

   // Iterating through working memory to make sure that all the above
   // wmes are there
   CPPUNIT_ASSERT(m_agent->num_wmes_in_rete == 5);

   Symbol* s1 = 0;

   bool found_type_wme = false;
   bool found_superstate_wme = false;
   bool found_io_wme = false;
   bool found_inputlink_wme = false;
   bool found_outputlink_wme = false;

   for (wme* curwme = m_agent->all_wmes_in_rete;
        curwme != 0;
        curwme = curwme->rete_next) {
      char* attr = curwme->attr->sc.name;

      // Checking that each wme has a structure that jives with 
      // the various agent struct memebers.
      if ( !strcmp(attr,"type") ) {
         found_type_wme = true;
         CPPUNIT_ASSERT(curwme->id == m_agent->top_state);
         CPPUNIT_ASSERT( !strcmp(curwme->value->sc.name,"state") );
      } else if ( !strcmp(attr,"superstate")) {
         found_superstate_wme = true;
         CPPUNIT_ASSERT(curwme->id == m_agent->top_state);
         CPPUNIT_ASSERT( !strcmp(curwme->value->sc.name,"nil") );
      } else if ( !strcmp(attr, "io") ) {
         found_io_wme = true;
         CPPUNIT_ASSERT(curwme->id == m_agent->top_state);
         CPPUNIT_ASSERT(curwme->value == m_agent->io_header);
      } else if ( !strcmp(attr,"input-link") ) {
         found_inputlink_wme = true;
         CPPUNIT_ASSERT(curwme->id == m_agent->io_header);
         CPPUNIT_ASSERT(curwme->value == m_agent->io_header_input);
      } else if ( !strcmp(attr,"output-link") ) {
         found_outputlink_wme = true;
         CPPUNIT_ASSERT(curwme->id == m_agent->io_header);
         CPPUNIT_ASSERT(curwme->value == m_agent->io_header_output);
      }
   }

   // Checking that all the appropriate wmes were found
   CPPUNIT_ASSERT(found_type_wme);
   CPPUNIT_ASSERT(found_superstate_wme);
   CPPUNIT_ASSERT(found_io_wme);
   CPPUNIT_ASSERT(found_inputlink_wme);
   CPPUNIT_ASSERT(found_outputlink_wme);

}

