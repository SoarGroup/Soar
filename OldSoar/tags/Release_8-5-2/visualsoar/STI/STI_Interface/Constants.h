#ifndef CONSTANTS_H
#define CONSTANTS_H

// Define the range of ports we'll use for runtimes and tools.
const short kBaseRuntimePort	= 2000 ;
const short kMaxRuntimes		= 20 ;
const short kMaxRuntimePort	= kBaseRuntimePort + kMaxRuntimes - 1 ;

const short kBaseToolPort		= 2100 ;
const short kMaxTools			= 10 ;
const short kMaxToolPort		= kBaseToolPort + kMaxTools - 1 ;

#endif
