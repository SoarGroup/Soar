#include "parsing.h"
#include "soarInterfaceCommands.h"

/*
 *  parsing.c
 *
 *  Helper functions to deal with the parsing of commands and arguments.
 *   
 *  These functions are not of particular interest to the usage of Soar
 *  or the API, and are documented somewhat more sparsely than the
 *  essential aspects of the interface.
 *
 */


typedef void * string;

#define memsize_of_string(gs) (*((int *)(gs)))
#define length_of_string(gs) (*(((int *)(gs))+1))
#define text_of_string(gs) (((char *)(gs)) + 2*sizeof(int *))

#define INITIAL_STRING_SIZE 100



int tokenizeString( char *input, int *ntokens, char ***tokens, 
		    char *token_terminator, soarResult *res );

string getCommandFromFile( int (*readCharacter)(FILE *), FILE *f, 
			  bool *eof_reached );

string make_blank_string (void);
void add_to_string (string *gs, char *string_to_add);
void clear_string (string gs);
void shorten_string( string *gs );


string gString;

/*  
 * execute a command.
 *
 *  The command is expected to be a string suitable for passing to
 *  tokenizeString.  The first word is interpreted as the command name
 *  and the remaining words are interpreted as arguments to that command
 *
 */
int executeCommand( char *command ) {

  char **tokens;
  int ntokens;
  char token_terminator;
  soarResult res;  
  soar_command *theCommand; 

  init_soarResult( res );

  token_terminator = '\0';

  /*
   *  Attempt to break the command into tokens.  If its is unparsable,
   *  i.e. a token is unterminated, the token_terminator character
   *  will be non-null. 
   */
  tokenizeString( command, &ntokens, &tokens, &token_terminator, &res );


  if ( token_terminator ) {    
    print( "\n\nError: Token unterminated...\n" );
    return SOAR_ERROR;
  }
  
  /*
   *  Check for no-op.
   */
  if ( ntokens < 1 ) return SOAR_OK;
  
  

  /*
   *  the first token is the command name.  Look for it in the command
   *  table.
   */
  theCommand = find_soar_command_structure( tokens[0] );
  
  /*
   *  If the command doesn't exist, print an error
   */
  if ( !theCommand ) {
    print( "Unknown command: '%s'\n", tokens[0] );
    return SOAR_ERROR;
  }
  
  /*
   *  Otherwise, execute the command, passing the tokens along to it.
   */
  if ( (*theCommand->command)( ntokens, tokens, &res ) == SOAR_ERROR ) {
    print( "Error evoking command: ");
    print ( "%s\n%s\n", theCommand->command_name, res.result );    
  }
  else if ( *res.result ) {
	print ( "   %s\n", res.result );
  }

  
  return SOAR_OK;
}



/*
 *  This function performs the majority of the work by breaking a
 *  single line up into tokens.  The line is expected to contain a
 *  single command and its arguments, so nothing too complex should be
 *  passed to this function. More complex input can be given to
 *  getCommandFromFile which deals with multiple lines, and other
 *  oddities.  A simple parsing method is used which is similar to that
 *  done by the Tcl shell.
 *  
 *  Basically, a line is split into words (words being alphanumeric
 *  strings seperated by whitespace).  
 *
 *  Words within double quotes are considered a single token as are
 *  words within curly braces.  
 *
 */

int tokenizeString( char *input, int *ntokens, char ***tokens, 
		     char *token_terminator, soarResult *res ) {


  char *token_begin, *token_end;        /* The beginning and end of a token */
  char *current;                        /* The current character */
  char *copy;
  int brace_level;                      /* The depth of nested braces */

  current = input;

#ifdef DEBUG
    printf( "Token Terminator = %c\n", *token_terminator);
    printf( "Tokenizing: '%s'\n", input );
#endif

  /*
   *  A command line can have at most MAX_TOKENS... beware.
   */
  *tokens = (char **)malloc( MAX_TOKENS * sizeof(char *) );
  *ntokens = 0;


  brace_level = 0;
  
  while( *current ) {

    /* find token begin */
    while ( !(*token_terminator) && isspace( *current ) ) {
      current++;
      
      if ( *current == '{' ) {
	*token_terminator = '}';
	
	/* Strip top level braces */
	current++;
	brace_level++;
      }
      else if( *current == '"' ) {
	*token_terminator = '"';
	
	/* Strip top level quotes */
	current++;       	
      }
    }
    
    token_begin = current;

#ifdef DEBUG
     printf( "Token %d", *ntokens ); 
#endif

    if ( *ntokens >= MAX_TOKENS ) {
      setSoarResultResult( res,
         "Too many tokens in string.  Exceeded maximum of %d\n", MAX_TOKENS );
      return SOAR_ERROR;
    }
  
    /* find token end */
    if ( *token_terminator ) {
      
      if ( *token_terminator == '}' ) {
	while( *current) {
	  if ( *current == '{' )  brace_level++;
	  if ( *current == '}' ) {
	    brace_level--;
	    if ( !brace_level ) break;
	  }
	  current++;	  
	}
      }
      else {
	while( *current && *current != *token_terminator ) current++;
      }
	    
      /* found terminator */
      if ( *current ) {
	
	/* Stip off top level token terminator */
	token_end = current;
	
	current++;
	*token_terminator = '\0';
      }
    }
    else {
      
      while( *current && !isspace( *current ) )
	current++;
      
      token_end = current;
    }
    
    
    
    if ( token_begin != token_end ) {
      
      /* Allocate memory for token storage */
      (*tokens)[*ntokens] = (char *)malloc( (token_end - token_begin + 1) * 
					    sizeof( char ) );
      

      copy = (*tokens)[*ntokens];
      while( token_begin != token_end ) {
		*copy++ = *token_begin++;
      }
      *copy = '\0';

      (*ntokens)++;
    }    
  }

#ifdef DEBUG
  {
    int i;
    for( i = 0; i < (*ntokens); i++ ) {
      printf( "Token %d: '%s'\n", i , (*tokens)[i] );
    }
  };

  printf( "Token Terminator = %c\n", *token_terminator);
  fflush( stdout );
#endif 

  /* HP: non-void function should return a value */
  return( SOAR_OK );
}









/*
 * getCommandFromFile
 *
 *  This function reads a command line from a file (including stdin) A
 *  command line is typically terminated by a new-line character.
 *  However, a semi-colon (;) is interpreted as a psuedo-new-line
 *  character, thus allowing mutliple 'command lines' to appear
 *  between new-line characters.
 *
 *  Note that if braces are used, new-line characters are ignored until
 *  matching close braces are found. This allows single arguments to span
 *  multiple lines.
 */
string getCommandFromFile( int (*readCharacter)(FILE *), FILE *f, 
			  bool *eof_reached ) {

  static char input_str[LINE_BUFFER_LENGTH];

  char *input; 
  int brace_level;
  char inQuotedString;
  int  inputInt;

  
  if ( gString == NULL ) gString = make_blank_string();
  else clear_string( gString );

  inQuotedString = 0;
  brace_level = 0;
  
  *eof_reached = FALSE;
  
  input = input_str;
  while ( (inputInt = (*readCharacter)(f)) ) {

    
    if ( inputInt == EOF ) {
      *eof_reached = TRUE;
      *input = '\0';
      break;
    }

    /* convert to char */
    *input = (char) inputInt;

    if ( *input == '#' && !inQuotedString ) {

      /* consume until end of line, without incrementing */
      while( (inputInt = (*readCharacter)(f)) ) {
	
		/* convert to char */
		*input = (char) inputInt;
	
		if ( (inputInt == EOF) || (*input == '\n') || (*input == '\0') ) {	 
		  *input = '\0';
		  break;
		}
      }
      continue;

    }
 
    /* Look for end of line */
    if ( *input == '\n' || *input == ';' ) {
      *input = '\n';
      if ( !inQuotedString && brace_level == 0 ) { 
				*input = '\0';
				break;
      }
    }
    else if ( *input == '"' ) {
			if ( inQuotedString == '"' ) inQuotedString = 0;
			else if ( inQuotedString == 0 ) inQuotedString = '"';
    }
		else if ( *input == '|' ) {
			if ( inQuotedString == '|' ) inQuotedString = 0;
			else if ( inQuotedString == 0 ) inQuotedString = '|';
		}
    else if ( *input == '{' ) {
      brace_level++;
    }
    else if ( *input == '}' ) {
      brace_level--;
      
      if ( brace_level < 0 ) {
	print( "\n extra characters after close-brace\n" );
	
	
	/* invalidate command & reset */		
	input_str[0] = '\0';
	brace_level = 0;
	inQuotedString = 0;
	
	break;
      }
    }
    
    input++;
    if ( (input - input_str) >= LINE_BUFFER_LENGTH ) {
	  
	  /* save the last character */
	  inputInt = (int)input_str[LINE_BUFFER_LENGTH - 1];
	  input_str[LINE_BUFFER_LENGTH - 1] = '\0';
	  add_to_string( &gString, input_str );

	  input = input_str;
	  *input++ = (char)inputInt;

    }

  } /* end while */

  if ( input != input_str ) {
	/* Then we've read a character */
	add_to_string( &gString, input_str );
  }

#ifdef DEBUG
  print( "Get Command Returns '%s'\n", text_of_string( gString ) ); 
#endif

  /* free up unused memory */
  //  shorten_string( &gString );
  return text_of_string(gString);
  
}
	





string make_blank_string (void) {
 string gs;

 gs = malloc (2*sizeof(int *) + INITIAL_STRING_SIZE );
 memsize_of_string(gs) = INITIAL_STRING_SIZE;
 length_of_string(gs) = 0;
 *(text_of_string(gs)) = 0;
 return gs;
}

void add_to_string (string *gs, char *string_to_add) {
  int current_length, length_to_add, new_length, new_memsize;
  string new;


  current_length = length_of_string(*gs);
  length_to_add = strlen (string_to_add);
  new_length = current_length + length_to_add;
  if (new_length + 1 > memsize_of_string(*gs)) {
    new_memsize = memsize_of_string(*gs);
    while (new_length + 1 > new_memsize) new_memsize = new_memsize * 2;
    new = malloc (new_memsize + 2*sizeof(int *));
    memsize_of_string(new) = new_memsize;
    strcpy (text_of_string(new), text_of_string(*gs));
    free (*gs);
    *gs = new;
  }
  strcpy (text_of_string(*gs)+current_length, string_to_add);
  length_of_string(*gs) = new_length;

}

void clear_string (string gs) {
  
  length_of_string( gs ) = 0;
  *(text_of_string(gs)) = 0;

}

void shorten_string( string *gs ) {

  int newlen;

  newlen = length_of_string( *gs ) + ((memsize_of_string( *gs ) - length_of_string( *gs )) / 2);


  if ( newlen >= memsize_of_string( *gs ) ) return;
  *gs = realloc( *gs, newlen + 2*sizeof( int * ) );
  memsize_of_string( *gs ) = newlen;
}


  

