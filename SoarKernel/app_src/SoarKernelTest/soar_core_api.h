/**
 * \file soar_core_api.h
 *   
 * This file contains the low-level (core) interface to the Soar production
 * system. 
 * 
 *
 * Copyright (c) 1995-1999 Carnegie Mellon University,
 *                         University of Michigan,
 *                         University of Southern California/Information
 *                         Sciences Institute.  All rights reserved.
 *
 * The Soar consortium proclaims this software is in the public domain, and
 * is made available AS IS.  Carnegie Mellon University, The University of 
 * Michigan, and The University of Southern California/Information Sciences 
 * Institute make no warranties about the software or its performance,
 * implied or otherwise.  All rights reserved.
 *
 * $Id$
 *
 */

#ifndef SOAR_CORE_API
#define SOAR_CORE_API

#include "soarapi.h"

#include "init_soar.h"
#include "gsysparam.h"

int soar_cRun( long n, Bool allAgents, enum go_type_enum type, 
	       enum soar_apiSlotType slot  );
void soar_cStopAllAgents( void );
void soar_cStopCurrentAgent( char *reason );
int soar_cDestroyAgentByName( char *name );
int soar_cDestroyAllAgentsWithName( char *name );
void soar_cDestroyAgentByAddress (psoar_agent delete_agent);
void soar_cInitializeSoar (void);
int soar_cReInitSoar (void);
void soar_cSetLearning( int setting );
double soar_cDetermineTimerResolution( double *min, double *max);
void soar_cSetCurrentAgent( psoar_agent a );
int soar_cAddWme( char *szId, char *szAttr, char *szValue, 
			  Bool acceptable_preference, psoar_wme *new_wme );
int soar_cRemoveWme( psoar_wme the_wme );
int soar_cRemoveWmeUsingTimetag( int num );
void soar_cRemoveInputFunction (agent * a, char * name);
void soar_cAddOutputFunction (agent * a, soar_callback_fn f, 
			  soar_callback_data cb_data, 
			  soar_callback_free_fn free_fn,
			  char * output_link_name);
void soar_cRemoveOutputFunction (agent * a, char * name);
void soar_cAddCallback (soar_callback_agent the_agent, 
		 	 SOAR_CALLBACK_TYPE callback_type, 
			 soar_callback_fn fn, 
			 soar_callback_data data,
			 soar_callback_free_fn free_fn,
			 soar_callback_id id);
void soar_cRemoveCallback (soar_callback_agent the_agent, 
			   SOAR_CALLBACK_TYPE callback_type, 
			   soar_callback_id id);
void soar_cPushCallback (soar_callback_agent the_agent, 
		 	 SOAR_CALLBACK_TYPE callback_type, 
			 soar_callback_fn fn, 
			 soar_callback_data data,
			 soar_callback_free_fn free_fn);
void soar_cPopCallback (soar_callback_agent the_agent, 
			SOAR_CALLBACK_TYPE callback_type);
int soar_cLoadReteNet( char *filename );
int soar_cSaveReteNet( char *filename );
void soar_cQuit ( void );
int soar_cDestroyAgentById( int agent_id );
psoar_agent soar_cGetAgentByName( char *name );
void soar_cAddInputFunction (agent * a, soar_callback_fn f, 
			     soar_callback_data cb_data, 
			     soar_callback_free_fn free_fn, char * name);
void soar_cExciseAllProductions (void);
void soar_cExciseAllTaskProductions (void);
void soar_cExciseAllProductionsOfType ( byte type );
char *soar_cGetAgentOutputLinkId( psoar_agent a, char *buff, size_t buff_size );
int soar_cSetOperand2( Bool turnOn );
int soar_cMultiAttributes( char *attribute, int value );
void soar_cCreateAgent (const char * agent_name, bool b);
int soar_cExciseProductionByName ( char *name );

#endif
