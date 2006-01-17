#include "soarapi.h"


void cb_exit ( agent *the_agent,
	       soar_callback_data data,
	       soar_call_data call_data );


void cb_print ( agent *the_agent,
	       soar_callback_data data,
	       soar_call_data call_data );


typedef struct soar_command_struct {

  /* Warning this MUST be the first field, for the ht routines */
  struct soar_command_struct *next;

  char *command_name;
  int (*command)( int, const char **, soarResult * );

} soar_command;


extern unsigned long masks_for_n_low_order_bits[];



/*
 *  Prototypes
 */
soar_command *new_soar_command( char *name, 
				int (*cmd)(int, const char **, soarResult *) );

int hash_soar_command( void * item, short nbits );
int interface_Source( int argc, const char **argv, soarResult *res );
soar_command *find_soar_command_structure( char *name );

void init_soar_command_table( void );
