/*

  This file contains various api functions that mimic the behavior of Soar
  commands. Each of these is a modified version of a function from the Soar 
  8.4 interface that ends with the letters "Cmd" (for "Command"). For instance,
  "MemoriesCmd" becomes soar_Memories. In addition, the following changes have
  been made to these functions to make them independent of TCL:

    - Their parameters have been reduced to argv and argc. (The original
	  functions received a pointer to the TCL interpreter and a ClientData
	  struct in addition to these.)
    
	- Of course, all references to the TCL interpreter and the ClientData
	  struct have been removed.

	- All output to TCL has been redirected to stdout using printf.
      These are for testing purposes only.

    - Some commands read numerical arguments from the parser. Any TCL
	  functions used toward this end have been removed in favor of the atoi
	  function.
*/


#ifndef NEW_API_H
#define NEW_API_H

int soar_Memories(char ** argv, int argc);
int soar_Preferences(char ** argv, int argc);
int soar_AttributePreferencesMode(char ** argv, int argc);
int soar_MultiAttributes(char ** argv, int argc);
int soar_Matches(char ** argv, int argc);
int soar_Warnings(char ** argv, int argc);
int soar_Learn(char ** argv, int argc);
int soar_Echo(char ** argv, int argc);
int soar_ProductionFind(char ** argv, int argc);
int soar_FormatWatch(char ** argv, int argc);
int soar_FiringCounts(char ** argv, int argc);
int soar_MaxChunks(char ** argv, int argc);
int soar_MaxElaborations(char ** argv, int argc);
int soar_MaxNilOutputCycles(char ** argv, int argc);
int soar_ChunkNameFormat(char ** argv, int argc);
int soar_DefWmeDepth(char ** argv, int argc);
int soar_Version(char ** argv, int argc);
int soar_Soarnews(char ** argv, int argc);
int soar_InternalSymbols(char ** argv, int argc);
int soar_InputPeriod(char ** argv, int argc);
int soar_GDS_Print(char ** argv, int argc);
int soar_IndifferentSelection(char ** argv, int argc);

#endif
