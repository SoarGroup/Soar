/*******************************************************************************
   Almost all of the functions in this file are almost certainly deprecated.
   This is the only function in interface.cpp which is needed by other files.
   As such, the following directive has been added to ensure that everything
   is ignored. -AJC (8/7/02)
*******************************************************************************/
#ifdef _DEPRECATED_

/*************************************************************************
 *
 *  file:  ma_interface.cpp
 *
 * =======================================================================
 *  These are the old, old interface routines for multi-agent soar mode.
 *  They are obsolete since Soar 7.0, and should be removed from the
 *  distribution...although if the Tcl interface isn't desired, we
 *  may want to adapt these somehow.
 * =======================================================================
 *
 * Copyright (c) 1995-1999 Carnegie Mellon University,
 *                         The Regents of the University of Michigan,
 *                         University of Southern California/Information
 *                         Sciences Institute.  All rights reserved.
 *
 * The Soar consortium proclaims this software is in the public domain, and
 * is made available AS IS.  Carnegie Mellon University, The University of 
 * Michigan, and The University of Southern California/Information Sciences 
 * Institute make no warranties about the software or its performance,
 * implied or otherwise.
 * =======================================================================
 
/* =================================================================
                          ma_interface.c
      Interface routines for multi-agent soar mode
   ================================================================= */

#include "interface.h"
#include "gsysparam.h"
#include "agent.h"
#include "print.h"
#include "init_soar.h"
#include "scheduler.h"
#include "kernel_struct.h"

/* -------------------------------------------------------------------
   
                          "Create Agent" Command

   Syntax:  (create-agent agent-name [agent-display-type])
------------------------------------------------------------------- */

char *help_on_create_agent[] = {
"Command: create-agent",
"",
"Syntax: (create-agent agent-name [agent-display-type])",
"",
"This command creates a single soar agent identified by the given",
"name.  The optional agent-display-type is used to select a set",
"of X resources that define the display characteristics of the",
"agent's window.  The agent-display-type is used to form the",
"prefix \"Soar.<agent-display-type>\" used in retrieving X",
"resources.  For example, if you had an agent-display-type of",
"Friendly, then the border color of the window would be set by",
"finding the resource \"Soar.Friendly.borderColor\".  You can",
"create new resources by editing the XSoar file of X resources.",
"The agent-display-type is valid only when using the X version",
"of Soar.",
0 };

Bool create_agent_interface_routine (Kernel* thisKernel, agent* thisAgent) {
  char sub_dir[MAXPATHLEN];    /* AGR 536 */
#define AGENT_NAME_BUFFER_SIZE 100
  char agent_name[AGENT_NAME_BUFFER_SIZE];
#define DISPLAY_TYPE_BUFFER_SIZE 100
  char display_type[DISPLAY_TYPE_BUFFER_SIZE];
  agent * first_agent;
  agent * new_agent;
  cons * c;
  agent * the_agent;
  Bool agent_already_defined;

  memset(sub_dir, 0, MAXPATHLEN);
  memset(agent_name, 0, MAXPATHLEN);
  memset(display_type, 0, MAXPATHLEN);
  first_agent = thisAgent;

  get_lexeme(thisAgent);  /* Consume "create-agent", advance to agent name */

  if (thisAgent->lexeme.type != SYM_CONSTANT_LEXEME) {
    print(thisAgent, "Expected agent name.\n");
    print_location_of_most_recent_lexeme(thisAgent);    
    return FALSE;
  }

  agent_already_defined = FALSE;
  for (c = thisKernel->all_soar_agents; c != NIL; c = c->rest) {
    the_agent = (agent *) c->first;
    if (!strcmp(the_agent->name, thisAgent->lexeme.string)) {
      agent_already_defined = TRUE;
    }
  }

  if (agent_already_defined) {
    print (thisAgent, "\nError: Agent %s ", thisAgent->lexeme.string);
    print (thisAgent, "already defined, duplicate declaration ignored.\n");
    print_location_of_most_recent_lexeme(thisAgent);
    return FALSE;
  } else {
    print(thisAgent, "\nCreating agent %s.\n", thisAgent->lexeme.string);
    strncpy (sub_dir, thisAgent->lexeme.string, MAXPATHLEN - 1);
    strncat (sub_dir, "/", MAXPATHLEN - 1 - strlen(sub_dir));

    strncpy(agent_name, thisAgent->lexeme.string, AGENT_NAME_BUFFER_SIZE - 1);
    
    get_lexeme(thisAgent);
   
    if (thisAgent->lexeme.type == R_PAREN_LEXEME)
	display_type[0] = '\0';
    else
      {
	if (thisAgent->lexeme.type != SYM_CONSTANT_LEXEME) 
	  {
	    print(thisAgent, "agent-display-type not a symbolic constant, ");
	    print(thisAgent, "using default instead.\n");
	    display_type[0] = '\0';
	  } 
	else
	  {
	    strncpy(display_type, thisAgent->lexeme.string, DISPLAY_TYPE_BUFFER_SIZE - 1);
	  }
      }

    /* Read remaining tokens to avoid a hang caused by tokens
       left in x_input_buffer intended for the creating agent
       but read by the created agent during the load_file 
       below. */

    if (thisAgent->lexeme.type != R_PAREN_LEXEME)
      get_lexeme(thisAgent);

    while (thisAgent->lexeme.type != R_PAREN_LEXEME)
      {
	print(thisAgent, "Ignoring extra token: %s\n", thisAgent->lexeme.string);
	get_lexeme(thisAgent);
      }

    thisAgent = create_soar_agent(thisKernel, agent_name);

#ifdef USE_X_DISPLAY
    create_agent_window(thisAgent, display_type);
#else
    if (display_type[0] != '\0')
      print(thisAgent, "agent-display-type argument only useable with X window version.\n");
#endif

#ifdef USE_X_DISPLAY
    print_agent_prompt(thisAgent);
#endif

    new_agent = thisAgent;
    thisAgent = first_agent;
    push(thisAgent, new_agent, thisKernel->all_soar_agents);

    if (thisAgent->lexeme.type != R_PAREN_LEXEME)
      get_lexeme(thisAgent); /* consume this one, advance to next agent name */

    thisKernel->all_soar_agents = destructively_reverse_list (thisKernel->all_soar_agents);

    return TRUE;
  }
}

/* -------------------------------------------------------------------
                           "Destroy Agent" Command

   Syntax:  (destroy-agent agent-name)
------------------------------------------------------------------- */

char *help_on_destroy_agent[] = {
"Command: destroy-agent",
"",
"Syntax: (destroy-agent agent-name)",
"",
"Remove agent from Soar.  If this command is issued in the window",
"of the agent being destroyed, then control switches to another",
"agent.",
0 };

Bool destroy_agent_interface_routine (Kernel* thisKernel, agent* thisAgent) {
  cons * c;
  
  get_lexeme(thisAgent);  /* consume "destroy-agent" */

  if (thisAgent->lexeme.type == SYM_CONSTANT_LEXEME) {
    for(c = thisKernel->all_soar_agents; c != NIL; c = c->rest) {
      if (!strcmp(thisAgent->lexeme.string,
		  ((agent *)c->first)->name)) {
        get_lexeme(thisAgent); 

	if (thisAgent == (agent *) c->first)
	  { 
	    print(thisAgent, "\nError: Attempt to delete current interpreter ignored.\n");
	    return FALSE;
	  }	

	destroy_soar_agent((agent *) c->first);
        return TRUE;
      }
    }

    if(!strcmp(thisAgent->lexeme.string, "control")) {
      print (thisAgent, "The global control agent cannot be destroyed.\n");
    } else {
      if (strcmp(thisAgent->lexeme.string, ")")) {
	print (thisAgent, "Unknown agent name.\n");
      } else {
	print (thisAgent, "Expected agent name.\n");
      }
    }
  } else {
    print (thisAgent, "Expected agent name.\n");
  }
  print_location_of_most_recent_lexeme(thisAgent);
  if (strcmp(thisAgent->lexeme.string, ")")) {
    get_lexeme(thisAgent); 
  }
  return FALSE;      
}

/* -------------------------------------------------------------------
   
                          "Agent Go" Command

   Syntax:  (agent-go agent-name [integer | 'forever'] [type])
            type ::= p | e | d | g | ps | s | o | context-variable
------------------------------------------------------------------- */

char *help_on_agent_go[] = {
"Command: agent-go",
"",
"Syntax: (agent-go agent_name [integer | 'forever'] [type])",
"        type ::= 'p' | 'e' | 'd' | 'g' | 'ps' | 's' | 'o' | context-variable",
"",
"This command operates exactly the same as the go command, EXCEPT that",
"this only defines the go settings for a agent when it is selected to",
"run by the multi-agent scheduler.  Hence, the go settings are defined",
"for subsequent runs, but no runs are made when the command is read."
"",
"See also:  go, d, run",
0 };

Bool agent_go_interface_routine (Kernel* thisKernel, agent* thisAgent) {
  cons  * c;
  agent * the_agent;
                         /* Initialize these to make gcc -Wall happy */
  long               prev_go_number = 0;
  Symbol           * prev_go_slot_attr = NULL;
  goal_stack_level   prev_go_slot_level = 0;
  enum go_type_enum  prev_go_type = GO_DECISION;
  Bool the_result;

  get_lexeme(thisAgent);

  for (c = thisKernel->all_soar_agents; c != NIL; c = c->rest) {
    the_agent = (agent *) c->first;
    if (!strcmp(the_agent->name, thisAgent->lexeme.string)) {
      if(the_agent != thisAgent) {
        prev_go_number     = thisAgent->go_number;
        prev_go_slot_attr  = thisAgent->go_slot_attr;
        prev_go_slot_level = thisAgent->go_slot_level;
        prev_go_type       = thisAgent->go_type;

        thisAgent->go_number     = the_agent->go_number;
        thisAgent->go_slot_attr  = the_agent->go_slot_attr;
        thisAgent->go_slot_level = the_agent->go_slot_level;
        thisAgent->go_type       = the_agent->go_type;
      }

      the_result = old_parse_go_command(thisAgent);

      if (the_result && the_agent != thisAgent) {
	the_agent->go_number     = thisAgent->go_number;
	the_agent->go_slot_attr  = thisAgent->go_slot_attr;
	the_agent->go_slot_level = thisAgent->go_slot_level;
	the_agent->go_type       = thisAgent->go_type;
      }

      if(the_agent != thisAgent) {
        thisAgent->go_number     = prev_go_number;
        thisAgent->go_slot_attr  = prev_go_slot_attr;
        thisAgent->go_slot_level = prev_go_slot_level;
        thisAgent->go_type       = prev_go_type;
      }
      return the_result;
    }
  }  

  print (thisAgent, "Agent %s is not known.  Ignoring command.\n", 
	 thisAgent->lexeme.string);

  print_location_of_most_recent_lexeme(thisAgent);
  if (strcmp(thisAgent->lexeme.string, ")")) {
    get_lexeme(thisAgent); 
  }
  return FALSE;      
}


/* -------------------------------------------------------------------
   
                           "schedule" Command

   Syntax:  (schedule [cycles])
------------------------------------------------------------------- */

char *help_on_schedule[] = {
"Command: schedule",
"",
"Syntax: (schedule [cycles])",
"",
"Schedule agents in a multi-agent soar setting.  If given, cycles",
"indicates how many cycles to run the scheduler.  If not given, the",
"scheduler runs until interrupted with a control-c or all agents",
"have completed processing.",
0 };

Bool schedule_interface_routine (agent* thisAgent) {
  struct lexeme_info last_lexeme;
#ifdef USE_X_DISPLAY
  cons * c;
  agent * the_agent;
#endif
  int cycles = -1;
  
  get_lexeme(thisAgent);  /* consume "schedule" */

  if (thisAgent->lexeme.type==INT_CONSTANT_LEXEME) {
    cycles = thisAgent->lexeme.int_val;
    scheduler_cycle_count = cycles;
    get_lexeme(thisAgent);  
  } else {
    scheduler_cycle_count = -1;
  }

  last_lexeme = thisAgent->lexeme; 

  schedule_agents(thisAgent, cycles);

#ifdef USE_X_DISPLAY  
  for (c = thisKernel->all_soar_agents; c != NIL; c = c->rest) {
    the_agent = (agent *) c->first;
    if (the_agent != thisAgent) {
    print_agent_prompt (the_agent);
    }
  }
#endif

  thisAgent->lexeme = last_lexeme;
  return TRUE;
}

/* -------------------------------------------------------------------
                           "Select Agent" Command

   Syntax:  (select-agent agent-name)
------------------------------------------------------------------- */

#ifndef USE_X_DISPLAY

char *help_on_select_agent[] = {
"Command: select-agent",
"",
"Syntax: (select-agent agent-name)",
"",
"Select agent to receive commands",
0 };

Bool select_agent_interface_routine (Kernel* thisKernel, agent* thisAgent) {
  cons * c;
  
  get_lexeme(thisAgent);  /* consume "select-agent" */

  if (thisAgent->lexeme.type == SYM_CONSTANT_LEXEME) {
    for(c = thisKernel->all_soar_agents; c != NIL; c = c->rest) {
      if (!strcmp(thisAgent->lexeme.string,
		  ((agent *)c->first)->name)) {
        get_lexeme(thisAgent); 

/*  AGR 533
        if (((agent *)c->first)->lexeme.type == EOF_LEXEME)
           print ("\nSoar agent %s> ", ((agent *)c->first)->name);
*/

	((agent *)c->first)->lexeme = thisAgent->lexeme; 

/************************************************************************/
	thisAgent = (agent *)c->first;

/* Here is where I make my first bug fix.  This is to fix bug 533, the
   missing prompt bug reported by Mike Hucka.  I will endeavour to comment
   my changes to the source code, in the hopes that they will be
   understandable to future maintainers.  I have found the existing source
   code to be only marginally understandable.
   My fix consists of one statement, which follows this comment.  I put it in
   because for some reason, the "thisAgent = (agent *)c->first;" statement
   immediately above changes thisAgent->current_char from '\n' to ' ',
   which causes the next prompt (in lexer.c) to not print out.  So it's a
   bit of a hack, but in that respect it seems to fit in with the rest of
   this code.  AGR 94.03.15  */
/* With this change, there were instances where 2 prompts were printed out,
   so I have commented out above where the other prompt gets printed out.
   AGR 94.03.15  */
/* Note that I am trying to implement some kind of standard notation for
   fixing bugs.  For now, I will attach a comment to every fix I make.
   This comment will be in the form "AGR x" where AGR are my initials and
   x is a number which corresponds to the bug number.  Allan G Rempel. */

	thisAgent->current_char = '\n';     /* AGR 533 */

        return TRUE;
      }
    }
    if (strcmp(thisAgent->lexeme.string, ")")) {
      print (thisAgent, "Unknown agent name.\n");
    } else {
      print (thisAgent, "Expected agent name.\n");
    }
  } else {
    print (thisAgent, "Expected agent name.\n");
  }
  print_location_of_most_recent_lexeme(thisAgent);
  if (strcmp(thisAgent->lexeme.string, ")")) {
    get_lexeme(thisAgent); 
  }
  return FALSE;      
}

#else /* USE_X_DISPLAY is defined */

/* -------------------------------------------------------------------
   
                          "Monitor" Command

   Syntax:  (monitor "command")
------------------------------------------------------------------- */

char *help_on_monitor[] = {
"Command: monitor",
"",
"Syntax: (monitor \"command\")",
"",
"Monitor displays the results of executing the given command after",
"each agent-go cycle.",
0 };

Bool monitor_interface_routine (agent* thisAgent) {
  
  get_lexeme(thisAgent);  /* consume "monitor", advance to quoted file name */
  if (thisAgent->lexeme.type!=QUOTED_STRING_LEXEME) {
    print ("Expected string in quotes for command to monitor\n");
    print_location_of_most_recent_lexeme(thisAgent);
    return FALSE;
  }

  create_monitor_window(thisAgent, thisAgent->lexeme.string);

  strncpy(thisAgent->monitor->input_buffer, thisAgent->lexeme.string, X_INFO_STRUCT_INPUT_BUFFER_SIZE);
  thisAgent->monitor->input_buffer[X_INFO_STRUCT_INPUT_BUFFER_SIZE - 1] = 0; /* ensure null termination */
  strncat(thisAgent->monitor->input_buffer, "\n", X_INFO_STRUCT_INPUT_BUFFER_SIZE - 1 - strlen(thisAgent->monitor->input_buffer));

  get_lexeme(thisAgent);  /* consume command, advance to rparen */
  return TRUE;
}

#endif /* USE_X_DISPLAY */

void init_multi_agent_built_in_commands(agent* thisAgent) {

  add_command (thisAgent, "create-agent", create_agent_interface_routine);
  add_help (thisAgent, "create-agent", help_on_create_agent);

/* See command declaration above...
  add_command (thisAgent, "create-agents", create_agents_interface_routine);
  add_help (thisAgent, "create-agents", help_on_create_agents);
*/
  add_command (thisAgent, "destroy-agent", destroy_agent_interface_routine);
  add_help (thisAgent, "destroy-agent", help_on_destroy_agent);

  add_command (thisAgent, "agent-go", agent_go_interface_routine);
  add_help (thisAgent, "agent-go", help_on_agent_go);

  add_command (thisAgent, "schedule", schedule_interface_routine);
  add_help (thisAgent, "schedule", help_on_schedule);

#ifdef USE_X_DISPLAY
  add_command (thisAgent, "monitor", monitor_interface_routine);
  add_help (thisAgent, "monitor", help_on_monitor);
#else
  add_command (thisAgent, "select-agent", select_agent_interface_routine);
  add_help (thisAgent, "select-agent", help_on_select_agent);
#endif /* USE_X_DISPLAY */
}

#endif /* _DEPRECATED_ */
