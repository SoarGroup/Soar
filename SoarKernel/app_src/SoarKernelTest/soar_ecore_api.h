/**
 * \file soar_ecore_api.h
 *   
 *                     The Extended Low Level interface to Soar
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

#ifndef SOAR_ECORE_API
#define SOAR_ECORE_API

#include "soarapi.h"

#include "gsysparam.h"

int soar_ecCaptureInput( char *filename );
int soar_ecWatchLevel( int level );
int soar_ecSp ( char *rule, char *sourceFile );
int soar_ecAddWmeFilter( char *szId, char *szAttr, char *szValue, 
			Bool adds, Bool removes );
int soar_ecRemoveWmeFilter( char *szId, char *szAttr, char *szValue, 
			Bool adds, Bool removes );
void soar_ecPrintSystemStatistics( void );
void soar_ecPrintMemoryStatistics (void);
void soar_ecPrintReteStatistics (void);
void soar_ecPrintMemoryPoolStatistics (void);
void soar_ecExplainChunkTrace(char *chunk_name);
void soar_ecExplainChunkCondition(char *chunk_name, int cond_number);
void soar_ecExplainChunkConditionList(char *chunk_name);
int soar_ecResetWmeFilters( Bool adds, Bool removes);
void soar_ecListWmeFilters( Bool adds, Bool removes);
int soar_ecPrintAllProductionsOfType( int type, Bool internal,
                                     Bool print_fname, Bool full_prod );
void soar_ecPrintProductionsBeingTraced();
void soar_ecStopAllProductionTracing();
int soar_ecBeginTracingProductions( int n, char **names );
int soar_ecStopTracingProductions( int n, char **names );

#endif
