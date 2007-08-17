#include <iostream>
#include <iterator>
#include <fstream>
#include <set>
#include <assert.h>

#include "reader.h"
#include "rule.h"
#include "predicate.h"
#include "matcher.h"
#include "mapper.h"

using namespace std;

void split_str(string s, vector<string>& tokens) {
  int start = 0;
  int i = 1;
  while (i < s.size()) {
    if (s[i] == ' ') {
      tokens.push_back(s.substr(start, i - start));
      start = i + 1;
      i += 2;
    }
    else {
      ++i;
    }
  }
  if (start < s.size()) {
    tokens.push_back(s.substr(start, s.size() - start));
  }
}

void readTypeData(char* filename, const set<Predicate>& preds, Pred2PlaceTypeMap& m) {
  ifstream f(filename);
  string line;
  while (not f.eof()) {
    getline(f, line);
    vector<string> toks;
    set<PlaceType> types;
    split_str(line, toks);
    if (toks.size() == 0) {
      continue;
    }
    for (int i = 1; i < toks.size(); ++i) {
      if (toks[i] == "object") {
        types.insert(OBJECT);
      }
      else if (toks[i] == "coordinate") {
        types.insert(COORD);
      }
      else if (toks[i] == "number") {
        types.insert(NUMBER);
      }
      else if (toks[i] == "unknown") {
        types.insert(UNKNOWN);
      }
    }
    // find the predicate
    bool found = false;
    for (set<Predicate>::const_iterator
         i  = preds.begin();
         i != preds.end();
         ++i)
    {
      if (i->get_name() == toks[0]) {
        m[*i] = types;
        found = true;
        break;
      }
    }
    if (!found) {
      cout << toks[0] << endl;
    }
    assert(found);
  }
}

int main(int argc, char* argv[]) {
  if (argc != 5) {
    cout << "Usage: " << argv[0] << " <src xkif> <tgt xkif> <src types> <tgt types>" << endl;
    return 1;
  }

  RulePtrSet srules;
  RulePtrSet trules;
  set<Predicate> spreds;
  set<Predicate> tpreds;
  Pred2PlaceTypeMap sTypes;
  Pred2PlaceTypeMap tTypes;
 
  Reader r1(argv[1]);
  r1.read_xkif(srules, spreds);
  Reader r2(argv[2]);
  r2.read_xkif(trules, tpreds);

  readTypeData(argv[3], spreds, sTypes);
  readTypeData(argv[4], tpreds, tTypes);

  // if there is a predicate not in the types file, add it with all unknown types
  for(set<Predicate>::iterator i = spreds.begin(); i != spreds.end(); ++i) {
    if (sTypes.find(*i) == sTypes.end()) {
      set<PlaceType> types;
      for (int j = 0; j < i->get_arity(); ++j) {
        types.insert(UNKNOWN);
      }
      sTypes[*i] = types;
    }
  }
  for(set<Predicate>::iterator i = tpreds.begin(); i != tpreds.end(); ++i) {
    if (tTypes.find(*i) == tTypes.end()) {
      set<PlaceType> types;
      for (int j = 0; j < i->get_arity(); ++j) {
        types.insert(UNKNOWN);
      }
      tTypes[*i] = types;
    }
  }

  Matcher matcher(sTypes, tTypes, srules, trules);

  while (true) {
    RulePtr r1;
    RulePtr r2;
    PredMapping m;
    if (!matcher.getBestMatch(r1, r2, m)) {
//      cout << "FINISHED0" << endl;
      break;
    }

    DEBUG(cout << "Best Match:" << endl;)
    DEBUG(cout << *r1 << endl;)
    DEBUG(cout << *r2 << endl;)
    
//    if (!matcher.findBestBodyMapping(*r1, *r2, m)) {
//      cout << "FINISHED1" << endl;
//      break;
//    }

    // commit to predicate matches
    // head predicates
    if (!matcher.srcPredMatched(r1->get_head()) and !matcher.tgtPredMatched(r2->get_head())) {
      matcher.addPredicateMatch(r1->get_head(), r2->get_head());
    }
    else if (matcher.hasPredMatchConflict(r1->get_head(), r2->get_head())) {
      DEBUG(cout << "ALREADY MATCHED!" << endl;);
      DEBUG(cout << r1->get_head() << endl << r2->get_head() << endl;)
    }
    else {
      DEBUG(cout << "MATCH PREDICATE CONFLICT!" << endl;)
      DEBUG(cout << r1->get_head() << endl << r2->get_head() << endl;)
    }

    // body predicates
    for (PredMapping::iterator
         i  = m.begin();
         i != m.end();
         ++i)
    {
      if (!matcher.srcPredMatched(i->first) and !matcher.tgtPredMatched(i->second)) {
        matcher.addPredicateMatch(i->first, i->second);
      }
//      else if (!matcher.hasPredMatchConflict(i->first, i->second)) {
//        cout << "MATCH PREDICATE CONFLICT!" << endl;
//        cout << i->first << endl << i->second << endl;
//      }
    }
    matcher.addRuleMatch(r1, r2);
  }

  vector<Predicate> usp;
  vector<Predicate> utp;
  vector<RulePtr> usr;
  vector<RulePtr> utr;

  matcher.getUnmatchedPreds(usp, utp);
  matcher.getUnmatchedRules(usr, utr);

  DEBUG(cout << "### UNMATCHED SOURCE PREDICATES ###" << endl;)
  DEBUG(copy(usp.begin(), usp.end(), ostream_iterator<Predicate>(cout, "\n"));)
  DEBUG(cout << "### UNMATCHED TARGET PREDICATES ###" << endl;)
  DEBUG(copy(utp.begin(), utp.end(), ostream_iterator<Predicate>(cout, "\n"));)

  DEBUG(cout << "### UNMATCHED SOURCE RULES ###" << endl;)
  DEBUG(
  for(vector<RulePtr>::iterator i = usr.begin(); i != usr.end(); ++i)
  {
    cout << **i << endl;
  })
  DEBUG(cout << "### UNMATCHED TARGET RULES ###" << endl;)
  DEBUG(
  for(vector<RulePtr>::iterator i = utr.begin(); i != utr.end(); ++i)
  {
    cout << **i << endl;
  })

  return 0;
}
