#ifndef __Reader_H__
#define __Reader_H__

#include <fstream>
#include "rule.h"
#include "predicate.h"

void read_xkif(ifstream& input, RulePtrSet& rules, set<Predicate>& preds);

#endif
