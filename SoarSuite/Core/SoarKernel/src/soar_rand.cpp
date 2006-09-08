#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H
#include <portability.h>

#include "soar_rand.h"

static MTRand gSoarRand;

// real number in [0,1]
double SoarRand()
{ return gSoarRand.rand(); }

// real number in [0,n]
double SoarRand(const double& max)
{ return gSoarRand.rand(max); }

// integer in [0,2^32-1]
unsigned long SoarRandInt()
{ return gSoarRand.randInt(); }

// integer in [0,n] for n < 2^32
unsigned long SoarRandInt(const unsigned long& max)
{ return gSoarRand.randInt(max); }


// automatically seed with a value based on the time or /dev/urandom
void SoarSeedRNG()
{ gSoarRand.seed(); }

// seed with a provided value
void SoarSeedRNG(const unsigned long seed)
{ gSoarRand.seed(seed); }
