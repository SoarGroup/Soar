#include <iostream>
#include <iterator>
#include <fstream>
#include <set>

#include "reader.h"
#include "rule.h"
#include "predicate.h"
#include "matcher.h"
#include "mapper.h"

using namespace std;

int main(int argc, char* argv[]) {
  if (argc != 3) {
    cout << "Usage: " << argv[0] << " <source xkif> <target xkif>" << endl;
    return 1;
  }

  RulePtrSet srules;
  RulePtrSet trules;
  set<Predicate> spreds;
  set<Predicate> tpreds;
 
  ifstream f1(argv[1]);
  ifstream f2(argv[2]);
  read_xkif(f1, srules, spreds);
  read_xkif(f2, trules, tpreds);

  Matcher matcher(
      vector<Predicate>(spreds.begin(), spreds.end()), 
      vector<Predicate>(tpreds.begin(), tpreds.end()), 
      srules, 
      trules);

  DEBUG(
  for(RulePtrSet::iterator i = srules.begin(); i != srules.end(); ++i)
  {
    cout << **i << endl;
  })
  DEBUG(
  for(RulePtrSet::iterator i = trules.begin(); i != trules.end(); ++i)
  {
    cout << **i << endl;
  })

  DEBUG(copy(spreds.begin(), spreds.end(), ostream_iterator<Predicate>(cout, "\n"));)
  DEBUG(copy(tpreds.begin(), tpreds.end(), ostream_iterator<Predicate>(cout, "\n"));)

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
    else if (!matcher.hasPredMatchConflict(r1->get_head(), r2->get_head())) {
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
