typedef struct agent_struct agent;
/*************************************************************************
 *************************************************************************/
extern agent * glbAgent;
#define current_agent(x) (glbAgent->x)
/*************************************************************************/
//#include "soarapi.h"
//#include "soarBuildOptions.h"

/* added (5/8/02) -ajc */
#include "soarapi.h"
#include "soar_core_api.h"
#include "definitions.h"
#include "source_cmd.h"

#include "kernel.h"
#include "print.h"
#include "gsysparam.h"
#include "symtab.h"

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <errno.h>

#define MAX_TOKENS 256
//#define MAX_LINE_LENGTH 1024


/**
 * Most of this code is stolen from soar/new-unix-interface/parsing.c
 */


int tokenizeString( char *input, int *ntokens, char ***tokens, 
		    char *token_terminator, soarResult *res );

char *getProductionFromFile( int (*readCharacter)(FILE *), FILE *f, 
			  Bool *eof_reached );

int loadProduction( char *command );





int sourceProductionsFromFile( char *fname ) {

  FILE *f;
  Bool eof_reached;

  f = fopen( fname, "r" );
  
  if ( !f ) {
    return SOAR_ERROR;
  }

  eof_reached = false;
  while( !eof_reached ) {
     char* tmp = getProductionFromFile( fgetc,f,  &eof_reached );
     loadProduction( tmp );
  }
  
  fclose( f );
  return SOAR_OK;
}
  
/*
 * A helper function for cleaning up the tokens in the "loadProduction"
 * function.
 */
void freeTokens(int ntokens, char** tokens) {
   for (int i=0; i<ntokens; i++) {
      free(tokens[i]);
   }
   free(tokens);
}

/*  
 * execute a command.
 *
 *  The command is expected to be a string suitable for passing to
 *  tokenizeString.  The first word is interpreted as the command name
 *  and the remaining words are interpreted as arguments to that command
 *
 */
int loadProduction( char *command ) {

  char **tokens;
  int ntokens;
  char token_terminator;
  soarResult res;  

  //if(strstr(command,"learn -on") != 0){
  //   int asasas=1;
  //}

  init_soarResult( res );

  token_terminator = '\0';

  int sl = strlen(command);

  /*
   *  Attempt to break the command into tokens.  If its is unparsable,
   *  i.e. a token is unterminated, the token_terminator character
   *  will be non-null. 
   */
  tokenizeString( command, &ntokens, &tokens, &token_terminator, &res );


  if ( token_terminator ) {    
    print(glbAgent, "\n\nError: Token unterminated...\n" );
    freeTokens(ntokens,tokens);
    free(command);
    return SOAR_ERROR;
  }
  
  /*
   *  Check for no-op.
   */
  if ( ntokens < 1 ) {
     freeTokens(ntokens,tokens);
     free(command);
     return SOAR_OK;
  }  
  
  /*
   *  the first token is the command name.  Make sure it is 'sp'
   */
  if (strcmp( tokens[0], "multi-attributes") == 0) {
     soar_cMultiAttributes(tokens[1], atoi(tokens[2]));
     freeTokens(ntokens,tokens);
     free(command);
     return SOAR_OK;
  }
  else  if (strcmp( tokens[0], "learn") == 0) {
     if(ntokens != 2){
        print(glbAgent, "Wrong # of arguments to excise\n");
        freeTokens(ntokens,tokens);
        free(command);
        return SOAR_ERROR;
     }

     if(strcmp( tokens[1], "-on") == 0) {
        print(glbAgent, "Learning On\n");
        soar_cSetLearning(ON);
        freeTokens(ntokens,tokens);
        free(command);
        return SOAR_OK;
     } 
     else if(strcmp( tokens[1], "-off") == 0) {
        print(glbAgent, "Learning Off\n");
        soar_cSetLearning(OFF);
        freeTokens(ntokens,tokens);
        free(command);
        return SOAR_OK;
     } 
     printf("Bad argument to learn\n");
     freeTokens(ntokens,tokens);
     free(command);
     return SOAR_ERROR;
  }
  else  if (strcmp( tokens[0], "excise") == 0) {
     print(glbAgent, "excising\n");
     if(ntokens != 2){
        print(glbAgent, "Wrong # of arguments to excise\n");
        freeTokens(ntokens,tokens);
        free(command);
        return SOAR_ERROR;
     }
     soar_cExciseProductionByName (tokens[1]);
     freeTokens(ntokens,tokens);
     free(command);
     return SOAR_OK;
  }
  else  if (strcmp( tokens[0], "set") == 0) {
     //print("Setting something\n");
     if(ntokens != 3){
        print(glbAgent, "Wrong # of arguments to excise\n");
        freeTokens(ntokens,tokens);
        free(command);
        return SOAR_ERROR;
     }
     if(strcmp( tokens[1], "warnings") == 0) {
        if(strcmp( tokens[2], "on") == 0) {
           set_sysparam (glbAgent, PRINT_WARNINGS_SYSPARAM, true);
           freeTokens(ntokens,tokens);
           free(command);
           return SOAR_OK;
        }
        else if (strcmp( tokens[2], "off") == 0) {
           set_sysparam (glbAgent, PRINT_WARNINGS_SYSPARAM, false);
           freeTokens(ntokens,tokens);
           free(command);
           return SOAR_OK;
        } 
        else {
           set_sysparam (glbAgent, MAX_ELABORATIONS_SYSPARAM, atoi(tokens[2]));
           printf("Bad value for set warnings\n");
           freeTokens(ntokens,tokens);
           free(command);
           return SOAR_OK;
        }
     }
     else if (strcmp( tokens[1], "max_elaborations") == 0) {
        set_sysparam( glbAgent, MAX_ELABORATIONS_SYSPARAM, atoi(tokens[2]) ); 
        freeTokens(ntokens,tokens);
        free(command);
        return SOAR_OK;
     }
     else if (strcmp( tokens[1], "save_backtraces") == 0) {
        if(strcmp( tokens[2], "on") == 0) {
           set_sysparam (glbAgent, EXPLAIN_SYSPARAM, true);
           freeTokens(ntokens,tokens);
           free(command);
           return SOAR_OK;
        }
        else if (strcmp( tokens[2], "off") == 0) {
           set_sysparam (glbAgent, EXPLAIN_SYSPARAM, false);
           freeTokens(ntokens,tokens);
           free(command);
           return SOAR_OK;
        } 
        freeTokens(ntokens,tokens);
        free(command);
        return SOAR_OK;
     }
  }
  else if (strcmp( tokens[0], "source") == 0) {
     FILE *f;
     f = fopen( tokens[1], "r" );
     
     if ( !f ) {
        printf("source-cmd could not open file %s", tokens[1]);
        freeTokens(ntokens,tokens);
        free(command);
        return SOAR_ERROR;;
     }
     
     Bool eof_reached = false;
     while( !eof_reached ) {
        char* tmp = getProductionFromFile( fgetc, f,  &eof_reached );
        loadProduction( tmp );
     }
     freeTokens(ntokens,tokens);
     free(command);
     return SOAR_OK;
  }
  
  if ( strcmp( tokens[0], "sp" ) ) {
	  print(glbAgent, "This ('%s') is not a production!\n", tokens[0] );
	  //return SOAR_ERROR;
  } 
  
  /*
   *  Otherwise, load the production body
   */
  if ( soar_Sp( ntokens, tokens, &res ) == SOAR_ERROR ) {
    print(glbAgent, "Error loading production\n");
  }
  else if ( *res.result ) {
    print (glbAgent, "   %s\n", res.result );
  }

  freeTokens(ntokens,tokens);
  free(command);
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
               if ( *current == '{' )  
                  brace_level++;
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

char *getProductionFromFile( int (*readCharacter)(FILE *), FILE *f, 
                            Bool *eof_reached ) {
   
   char *input_str;
   unsigned int input_str_len=0;
   unsigned int current_size=512;
   char *input; 
   int brace_level;
   int inQuotedString;
   
   int  inputInt;

   input_str = (char *)malloc(current_size);
   
   inQuotedString = 0;
   brace_level = 0;
   
   *eof_reached = false;
   
   input = input_str;
   while ( inputInt = (*readCharacter)(f) ) {
      
      
      if ( inputInt == EOF ) {
         *eof_reached = true;
         *input = '\0';
         break;
      }
      
      /* convert to char */
      *input = (char) inputInt;
      
      if ( *input == '#' ) {
         
         /* consume until end of line, without incrementing */
         while( inputInt = (*readCharacter)(f) ) {
            
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
         inQuotedString = !inQuotedString;
      }
      else if ( *input == '{' ) {
         brace_level++;
      }
      else if ( *input == '}' ) {
         brace_level--;
         
         if ( brace_level < 0 ) {
            print(glbAgent, "\n extra characters after close-brace\n" );
            
            
            /* invalidate command & reset */		
            input_str[0] = '\0';
            brace_level = 0;
            inQuotedString = 0;
      
            break;
         }
      }

      if(input_str_len++ >= 511){
         input_str_len = 0;
         current_size += 512;
         input_str = (char *)realloc(input_str, current_size);
         input = (input_str + current_size - 512)-1;/*we are about to increment anyway.*/
      }
      
      input++;
      
      
   }

  /*  print(glbAgent, "Get Command Returns '%s'\n", input_str ); */
  return input_str;
  
}
