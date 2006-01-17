#include "soar_core_api.h"



void askCallback( soar_callback_agent the_agent,
		  soar_callback_data data,
		  soar_call_data call_data ) {
  
  
  int num_candidates;
  preference *cand;

  *((soar_apiAskCallbackData *)call_data)->selection = 
    ((soar_apiAskCallbackData *)call_data)->candidates;

  num_candidates = 0;

  for (cand = ((soar_apiAskCallbackData *)call_data)->candidates;
       cand!=NIL; cand=cand->next_candidate) {
    num_candidates++;
    print( " --> %s\n", symbol_to_string( cand->value, TRUE, NULL, 0 ) );
  }
}



