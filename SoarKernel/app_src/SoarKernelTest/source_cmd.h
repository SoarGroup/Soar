#ifndef GSYSPARAM_H
#include "gsysparam.h"
#endif

#include <stdio.h>

extern char *getProductionFromFile( int (*readCharacter)(FILE *) , FILE *f, Bool *eof_reached );

extern int sourceProductionsFromFile( char *fname );

extern int loadProduction( char *command );


