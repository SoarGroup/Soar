#include <iostream>
#include <iterator>
#include <fstream>
#include <set>

#include "reader.h"
#include "rule.h"
#include "predicate.h"
#include "matcher.h"

using namespace std;

int main(int argc, char* argv[]) {
  if (argc != 3) {
    cout << "Usage: " << argv[0] << " <source xkif> <target xkif>" << endl;
    return 1;
  }

  set<Rule> srules;
  set<Rule> trules;
  set<Predicate> spreds;
  set<Predicate> tpreds;
 
  ifstream f1(argv[1]);
  ifstream f2(argv[2]);
  read_xkif(f1, srules, spreds);
  read_xkif(f2, trules, tpreds);

  /*
  copy(srules.begin(), srules.end(), ostream_iterator<Rule>(cout, "\n"));
  copy(spreds.begin(), spreds.end(), ostream_iterator<Predicate>(cout, "\n"));
  copy(trules.begin(), trules.end(), ostream_iterator<Rule>(cout, "\n"));
  copy(tpreds.begin(), tpreds.end(), ostream_iterator<Predicate>(cout, "\n"));
  */

  Matcher matcher(
      vector<Predicate>(spreds.begin(), spreds.end()), 
      vector<Predicate>(tpreds.begin(), tpreds.end()), 
      srules, 
      trules);

  cout << "HERE IT COMES" << endl;
  cout << matcher << endl;
  cout << "=========" << endl;

  while (true) {
    const Rule* r1;
    const Rule* r2;
    BodyMapping m;
    if (!matcher.getBestMatch(r1, r2, m)) {
      cout << "FINISHED0" << endl;
      break;
    }

    cout << "Best Match:" << endl;
    cout << *r1 << endl;
    cout << *r2 << endl;
    
//    if (!matcher.findBestBodyMapping(*r1, *r2, m)) {
//      cout << "FINISHED1" << endl;
//      break;
//    }
    for (BodyMapping::iterator
         i  = m.begin();
         i != m.end();
         ++i)
    {
      if (!matcher.predicateMatched(i->first) and 
          !matcher.predicateMatched(i->second)) 
      {
        matcher.addPredicateMatch(i->first, i->second);
      }
    }
    matcher.addRuleMatch(*r1, *r2);
  }

  vector<Predicate> usp;
  vector<Predicate> utp;
  vector<Rule> usr;
  vector<Rule> utr;

  matcher.getUnmatchedPreds(usp, utp);
  matcher.getUnmatchedRules(usr, utr);

  cout << "### UNMATCHED SOURCE PREDICATES ###" << endl;
  copy(usp.begin(), usp.end(), ostream_iterator<Predicate>(cout, "\n"));
  cout << "### UNMATCHED TARGET PREDICATES ###" << endl;
  copy(utp.begin(), utp.end(), ostream_iterator<Predicate>(cout, "\n"));

  cout << "### UNMATCHED SOURCE RULES ###" << endl;
  copy(usr.begin(), usr.end(), ostream_iterator<Rule>(cout, "\n"));
  cout << "### UNMATCHED TARGET RULES ###" << endl;
  copy(utr.begin(), utr.end(), ostream_iterator<Rule>(cout, "\n"));

  return 0;
}
