#include <portability.h>

#include "soar_rand.h"

static MTRand gSoarRand;

// real number in [0,1]
SOAR_DLL double SoarRand()
{ return gSoarRand.rand(); }

// real number in [0,n]
SOAR_DLL double SoarRand(const double& max)
{ return gSoarRand.rand(max); }

// integer in [0,2^32-1]
SOAR_DLL uint32_t SoarRandInt()
{ return gSoarRand.randInt(); }

// integer in [0,n] for n < 2^32
SOAR_DLL uint32_t SoarRandInt(const uint32_t& max)
{ return gSoarRand.randInt(max); }


// automatically seed with a value based on the time or /dev/urandom
SOAR_DLL void SoarSeedRNG()
{ gSoarRand.seed(); }

// seed with a provided value
SOAR_DLL void SoarSeedRNG(const uint32_t seed)
{ gSoarRand.seed(seed); }
