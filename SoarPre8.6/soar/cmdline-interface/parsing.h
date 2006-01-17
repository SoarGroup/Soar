#include "soarkernel.h"
#include "soarapi.h"


#define MAX_TOKENS 25
#define LINE_BUFFER_LENGTH 512

int tokenizeString( char *input, int *ntokens, char ***tokens, 
		    char *token_terminator, soarResult *res );


growable_string getCommandFromFile( int (*readCharacter)(FILE *), FILE *f, 
			  bool *eof_reached );

int executeCommand( char *command );

