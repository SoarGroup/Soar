#ifndef __TEXT_INPUT_H__
#define __TEXT_INPUT_H__

#include "Epmem.h"
#include "SymbolFactory.h"
#include <list>
#include <istream>

extern int ReadEpisodes(std::istream &input, EpmemNS::Epmem &epmem);
extern int ReadCue(std::istream &input, EpmemNS::SymbolFactory *symfac, EpmemNS::WMEList &cue);

#endif