/*
  Some debugging methods determining wether or not various data structures
  are in valid states. These are not fully fleshed out yet so feel free 
  to extend these as appropriate. (RDF)
*/

#ifndef DEBUGFCNS_H
#define DEBUGFCNS_H

typedef union symbol_union Symbol;
typedef struct wme_struct wme;
typedef struct slot_struct slot;
typedef struct agent_struct agent;

bool isSymbolValid(Symbol* sym);
bool isWmeValid(wme* w);
bool isSlotValid(slot* s);

#endif
