/*
  Some debugging methods determining wether or not various data structures
  are in valid states. These are not fully fleshed out yet so feel free 
  to extend these as appropriate. (RDF)
*/

#include "debug.h"

#include "wmem.h"
#include "gdatastructs.h"
#include "symtab.h"
#include "agent.h"

#include <iostream>

/*
  Simple method for testing some aspects of a symbol to make sure 
  that it's possibly valid.
*/
bool isSymbolValid(Symbol* sym) {
   /* Symbol type must be one of the five valid types*/
   if ( sym->common.symbol_type != VARIABLE_SYMBOL_TYPE &&
        sym->common.symbol_type != IDENTIFIER_SYMBOL_TYPE &&
        sym->common.symbol_type != SYM_CONSTANT_SYMBOL_TYPE &&
        sym->common.symbol_type != INT_CONSTANT_SYMBOL_TYPE &&
        sym->common.symbol_type != FLOAT_CONSTANT_SYMBOL_TYPE ) {
      std::cerr << "Symbol is invalid type = " 
                << (int) sym->common.symbol_type 
                << std::endl;
      return false;
   }

   /* Reference count must be non-zero */
   if ( sym->common.reference_count == 0 ) {
      std::cerr << "Symbol reference count is zero!" << std::endl;
      return false;
   }

   /* ID symbols should have a name letter in the range A-Z */
   if ( sym->common.symbol_type == IDENTIFIER_SYMBOL_TYPE ) {
      if ( sym->id.name_letter < 'A' || sym->id.name_letter > 'Z' ) {
         std::cerr << "Symbol name letter is not in valid range = " 
                   << sym->id.name_letter 
                   << std::endl;
         return false;
      }
   }

   return true;
}

/*
  Simple method for testing some aspects of a wme to make sure 
  that it's possibly valid.
*/
bool isWmeValid(wme* w) {
   /* The id attribute and value symbols must be valid*/
   if ( !isSymbolValid(w->id) ) {
      std::cerr << "Wme id is invalid!" << std::endl;
      return false;
   }

   if ( !isSymbolValid(w->attr) ) {
      std::cerr << "Wme attribute is invalid!" << std::endl;
      return false;
   }

   if ( !isSymbolValid(w->value) ) {
      std::cerr << "Wme value is invalid!" << std::endl;
      return false;
   }

   /* Reference count must be non-zero  Apparently some wme's 
      exist for quite a while with no reference count (unlike 
      symbol objects. */
   /*
   if ( w->reference_count == 0 ) {
      std::cerr << "Wme reference count is zero!" << std::endl;
      return false;
   }
   */

   return true;
}

/*
  Simple method for testing some aspects of a slot to make sure 
  that it's possibly valid.
*/
bool isSlotValid(slot* s) {
   /* Check that the slot id and attribute are valid */
   if ( !isSymbolValid(s->id) ) {
      std::cerr << "Slot id symbol is invalid!" << std::endl;
      return false;
   }
   if ( !isSymbolValid(s->attr) ) {
      std::cerr << "Slot attribute symbol is invalid!" << std::endl;
      return false;
   }

   /* Check that all the wmes in the slot are valid */
   wme* prevwme = NIL;
   for (wme* curwme = s->wmes; curwme != NIL; curwme = curwme->next) {
      if ( curwme->prev != prevwme ) {
         std::cerr << "Slot wmes previous and next are inconsistent!" << std::endl;
         return false;
      }
      if (!isWmeValid(curwme)) {
         std::cerr << "A slot wme is invalid!" << std::endl;
         return false;
      }
      prevwme = curwme;
   }

   /* Check that all the preference wmes are valid */
   prevwme = NIL; 
   for (wme* curwme = s->acceptable_preference_wmes; curwme != NIL; curwme = curwme->next) {
      if ( curwme->prev != prevwme ) {
         std::cerr << "Slot acceptable pref wmes prev and next are inconsistent!" 
                   << std::endl;
         return false;
      }
      if ( !isWmeValid(curwme)) {
         std::cerr << "Slot acceptable pref wme is invalid!" << std::endl;
         return false;
      }
      prevwme = curwme;
   }

   return true;
}

