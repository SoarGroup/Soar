#include "soarapi.h"
#include "soar_core_api.h"
#include "soarkernel.h"
#include "demo_adder.h"


/*
 *  two Global variables used to store state information between IO cycles.
 */
int number_received;
int last_tt;


/*
 *   The input function
 */
void io_input_fn( agent *a, soar_callback_data data,
	      soar_call_data call_data ) {
  
  char buff[20];
  psoar_wme w;


  switch ( (int) call_data ) {

  case TOP_STATE_JUST_CREATED:
    print( "\nTop state just created\n" );
    break;


  case NORMAL_INPUT_CYCLE:
    print( "\nNormal Input Cycle\n" );
    
    /* 
     * If we received a number on the last DC, increment it and
     * add it to the input link
     */
    if ( number_received > -1 ) {

      /*
       *  If we have previously added a wme to the input-link, its
       *  time-tag will be stored in last_tt and be > -1.  In this 
       *  situation, we want to remove it, so that our agent doesn't
       *  get confused by there being more than one number on the
       *  input-link at a time.
       *
       *  In general, it is the responsibility of the environment
       *  to add and remove wmes from the input-link. 
       */
      if ( last_tt > - 1) {
	
	/* 
	 *  Remove the last wme we added
	 */
	soar_cRemoveWmeUsingTimetag( last_tt );
      }            

      
      print( "Adding a number to the input link (%d)\n", number_received+1 );

      /*
       *  Get ready to add a new number to the input-link by first
       *  writing this new number (the number received from the output-link
       *  plus 1) into a string buffer.
       */
      sprintf( buff, "%d", number_received+1);

      /*
       *  Add the wme.  Note that here we assume that I2 is the input-link
       *  Note that this might not always be the case.  A more robust
       *  version of this code would grab the input-link ID when
       *  when the input-function is called with the mode 
       *  TOP_STATE_JUST_CREATED
       */
      last_tt = soar_cAddWme( "I2", "^number", buff, FALSE, &w);

      print( "Timetag (last_tt) = %d\n", last_tt );

    }
    break;
  }


  
}


/*
 *  The output function
 */
void io_output_fn( agent *a, soar_callback_data data,
	      soar_call_data call_data ) {

  io_wme *o_wme;
  char *buff;

  number_received = -1;

  switch( ((output_call_info *)call_data)->mode ) {
    
  case MODIFIED_OUTPUT_COMMAND:
  case ADDED_OUTPUT_COMMAND:

    /* 
     *  Just to show how the output-function is being invoked...
     */
    if ( ((output_call_info *)call_data)->mode == MODIFIED_OUTPUT_COMMAND ) {
      print ("\nA command on the output-link has been modified!\n" );
    }
    else if (((output_call_info *)call_data)->mode == ADDED_OUTPUT_COMMAND) {
      print ("\nA new command has been added to the output-link!\n" );
    }


    /*
     *  Cycle through all wmes which were added to the output-link
     */
    for( o_wme = ((output_call_info *)call_data)->outputs;
	 o_wme != NULL; o_wme = o_wme->next ) {
     
      /*
       *  Look for a wme whose attribute is "number"...This is the only one
       *  we care about...
       * 
       *  Here we use the soar core api's wme accessor functions to yield
       *  string representations of the wme's data.  It is possible to
       *  do this in other ways, one useful method is to preallocate
       *  a buffer "big enough" to hold any attribute, or value, and
       *  pass this as the second argument to the accessor function.
       *  This means that a new buffer is not allocated, and does not
       *  need to be freed, and can result in significant time savings.
       *  See the documentation in soar_core_api.h for details.
       *  
       */
      buff = soar_cGetWmeAttr( o_wme, NULL, 0 );
      if ( !strcmp( buff, "number")) {
	
	free( buff );
	buff = soar_cGetWmeValue( o_wme, NULL, 0 );
	number_received = atoi( buff );
	free( buff );
 
    	print( "Received number %d from the output link\n", number_received);
      }
      else {
	free( buff );
      }
    }    
    break;

    
  }


}



