#include <assert.h>
#include <algorithm>
#include <iterator>
#include <boost/bind.hpp>

#include "matcher.h"

using namespace std;
using boost::bind;

Matcher::Matcher
( vector<Predicate> sPreds,
  vector<Predicate> tPreds,
  set<Rule> sRules,
  set<Rule> tRules )
: sourceRules(sRules.begin(), sRules.end()), 
  targetRules(tRules.begin(), tRules.end()),
  sourcePreds(sPreds), 
  targetPreds(tPreds)
{
  for (set<Rule>::iterator 
       i  = sourceRules.begin(); 
       i != sourceRules.end();
       ++i)
  {
    for (set<Rule>::iterator
         j  = targetRules.begin();
         j != targetRules.end();
         ++j)
    {
      BodyMapping map;
      if(rulesCanMatch(*i, *j, map)) {
        rMatchCands.insert(RuleMatchType(RulePair(*i, *j), map));
      }
    }
  }

  // map all predicates that appear in both source and target onto
  // each other
  vector<Predicate> toErase;
  for (vector<Predicate>::iterator
       i  = sourcePreds.begin();
       i != sourcePreds.end();
       ++i)
  {
    if (find(targetPreds.begin(), targetPreds.end(), *i) != targetPreds.end()) {
      matchedPreds.insert(PredPair(*i, *i));
      updateRuleMatches(*i, *i);
      toErase.push_back(*i);
    }
  }
  for (vector<Predicate>::iterator
       i  = toErase.begin();
       i != toErase.end();
       ++i)
  {
    sourcePreds.erase(remove(sourcePreds.begin(), sourcePreds.end(), *i), sourcePreds.end());
    targetPreds.erase(remove(targetPreds.begin(), targetPreds.end(), *i), targetPreds.end());
  }
}

Matcher::Matcher(const Matcher& other)
  : sourceRules(other.sourceRules),
    targetRules(other.targetRules),
    sourcePreds(other.sourcePreds),
    targetPreds(other.targetPreds),
    matchedPreds(other.matchedPreds),
    matchedRules(other.matchedRules),
    rMatchCands(other.rMatchCands)
{ }

bool Matcher::findBodyMapping
( const Rule& sr, 
  const Rule& tr, 
  BodyMapping& mapping) const
{
  vector<Condition> smaller;
  vector<Condition> larger;

  // if b is the body of the rule with fewer body conditions,
  // for every predicate p in b, there must be one predicate p' in
  // b' that p can match with
  if (sr.body_size() < tr.body_size()) {
    sr.get_body(smaller);
    tr.get_body(larger);
    return findBodyMappingInternal(smaller, larger, mapping);
  }
  else {
    tr.get_body(smaller);
    sr.get_body(larger);
    BodyMapping tempMap;
    if (findBodyMappingInternal(smaller, larger, tempMap)) {
      // have to reverse the order of the matching first
      for(BodyMapping::iterator
          i  = tempMap.begin();
          i != tempMap.end();
          ++i)
      {
        mapping.insert(PredPair(i->second, i->first));
      }
      return true;
    }
    else {
      return false;
    }
  }
}

bool Matcher::findBodyMappingInternal
( vector<Condition> smaller,  // by value on purpose
  vector<Condition> larger,
  BodyMapping& mapping ) const
{
  if (smaller.size() == 0) {
    return true;
  }
  else {
    Predicate p1 = smaller[0].pred;
    bool p1n = smaller[0].negated;
    smaller.erase(smaller.begin());
    bool exhausted = false;
    while (!exhausted) {
      exhausted = true;
      for (vector<Condition>::iterator 
           i  = larger.begin();
           i != larger.end();
           ++i)
      {
        if (predicatesCanMatch(p1, i->pred) and p1n == i->negated) {
          Predicate p2 = i->pred;
          larger.erase(i);
          if (findBodyMappingInternal(smaller, larger, mapping)) {
            mapping.insert(PredPair(p1, p2));
            return true;
          }
          else {
            // note that we've erased a predicate that can't
            // match in the larger set at this point
            exhausted = false;
            break;
          }
        }
      }
      // if we get to here and exhausted hasn't been set to false,
      // then we've run through the entire larger set, and there are
      // no matching candidates.
    }
    return false;
  }
}

bool Matcher::rulesCanMatch
( const Rule& sr, 
  const Rule& tr, 
  BodyMapping& mapping) const
{
  // rules heads must be matchable
  if (!predicatesCanMatch(sr.get_head(), tr.get_head())) {
    return false;
  }

  // facts
  if (sr.body_size() == 0 and tr.body_size() == 0) {
    return true;
  }

  // can't match facts with rules
  if (sr.body_size() == 0 or tr.body_size() == 0) {
    return false;
  }

  BodyMapping tempMap;
  if (findBodyMapping(sr, tr, tempMap)) {
    mapping.insert(tempMap.begin(), tempMap.end());
    return true;
  }
  else {
    return false;
  }
}

/*
bool Matcher::rulesCannotMatch(const RuleMatchType& m) {
  return !rulesCanMatch(m.first.first, m.first.second, m.second);
}
*/

void Matcher::updateRuleMatches(const Predicate& sp, const Predicate& tp) {
  vector<RuleMatchMap::iterator> toErase;
  for (RuleMatchMap::iterator
       j  = rMatchCands.begin();
       j != rMatchCands.end();
       ++j)
  {
    const Rule& sr = j->first.first;
    const Rule& tr = j->first.second;
    for(BodyMapping::iterator
        i  = j->second.begin();
        i != j->second.end();
        ++i)
    {
      if (i->first == sp and i->second == tp) {
        // new match consistent with mapping
        break;
      }
      if (i->first == sp or i->second == tp) {
        // the match is not consistent, and not disjoint from, the
        // existing mapping. The mapping is invalidated, calculate
        // new mapping
        BodyMapping newMap;
        if (findBodyMapping(sr, tr, newMap)) {
          // new mapping succeeded
          j->second = newMap;
        }
        else {
          // new mapping failed
          toErase.push_back(j);
        }
        break;
      }
    }
  }

  for(vector<RuleMatchMap::iterator>::iterator
      i  = toErase.begin();
      i != toErase.end();
      ++i)
  {
    cout << "REMOVING CANDIDATE MATCH " << (*i)->first.first << " --> " << (*i)->first.second << endl;
    rMatchCands.erase(*i);
  }
}

bool Matcher::predicatesCanMatch(const Predicate& p1, const Predicate& p2) const {
  if (p1.get_arity() != p2.get_arity()) {
    return false;
  }
  if (p1.get_type() != p2.get_type()) {
    return false;
  }
  map<Predicate, Predicate>::const_iterator p1_pos = matchedPreds.find(p1);
  map<Predicate, Predicate>::const_iterator p2_pos = matchedPreds.find(p2);
  if (p1_pos != matchedPreds.end() and p1_pos->second != p2) {
    return false;
  }
  if (p2_pos != matchedPreds.end() and p2_pos->second != p1) {
    return false;
  }
  return true;
}

int Matcher::ruleMatchScore(const Rule& r1, const Rule& r2) const {
  int score = 0;
  if (predicateMatched(r1.get_head())) {
    score += 10;
  }
  vector<Condition> body;
  if (r1.body_size() == r2.body_size()) {
    score += 3;
  }
  if (r1.body_size() < r2.body_size()) {
    r1.get_body(body);
  }
  else {
    r2.get_body(body);
  }
  for(vector<Condition>::iterator
      i =  body.begin();
      i != body.end();
      ++i)
  {
    if (predicateMatched(i->pred)) {
      score += 1;
    }
  }
  return score;
}

// should make this smarter/more efficient
/*
void Matcher::updatePossibleRuleMatches(const Predicate& sp, const Predicate& tp) {
  RuleMatchMap::iterator new_end;
  new_end = remove_if(rMatchCands.begin(), rMatchCands.end(), bind(&Matcher::rulesCannotMatch, this, 
  for (RuleMatchMap::iterator
       i =  possibleRuleMatches.begin();
       i != possibleRuleMatches.end();
       ++i)
  {
    i->second.erase(remove_if(i->second.begin(), i->second.end(), 
                              boost::bind(&Matcher::rulesCannotMatch, this, i->first, _1)),
                    i->second.end());
  }
}
*/

bool Matcher::getBestMatch(const Rule*& r1, const Rule*& r2) const {
  int bestScore = -1;
  r1 = NULL; r2 = NULL;
  for (RuleMatchMap::const_iterator
       i =  rMatchCands.begin();
       i != rMatchCands.end();
       ++i)
  {
    const Rule& sr = i->first.first;
    const Rule& tr = i->first.second;
    int score = ruleMatchScore(sr, tr);
    if (score > bestScore) {
      bestScore = score;
      r1 = &(sr);
      r2 = &(tr);
    }
  }
  if (r1 == NULL or r2 == NULL) {
    // no more match candidates
    return false;
  }
  else {
    return true;
  }
}

void Matcher::addPredicateMatch(const Predicate& sp, const Predicate& tp) {
  assert(matchedPreds.find(sp) == matchedPreds.end());
  assert(matchedPreds.find(tp) == matchedPreds.end());
  matchedPreds.insert(PredPair(sp, tp));
  if (sp != tp) {
    matchedPreds.insert(PredPair(tp, sp));
  }
  cout << "MATCHING PREDICATE " << sp << " TO " << tp << endl;

  sourcePreds.erase(remove(sourcePreds.begin(), sourcePreds.end(), sp), sourcePreds.end());
  targetPreds.erase(remove(targetPreds.begin(), targetPreds.end(), tp), targetPreds.end());
  updateRuleMatches(sp, tp);
}

void Matcher::addRuleMatch(const Rule& sr, const Rule& tr) {
  cout << "MATCHING RULE " << sr << " TO " << tr << endl;

  vector<RuleMatchMap::iterator> toErase;
  matchedRules.insert(RulePair(sr, tr));
  //rMatchCands.erase(remove_if(rMatchCands.begin(), rMatchCands.end(), bind(&RuleMatchType::first, _1) == rp), rMatchCands.end());

  for (RuleMatchMap::iterator
       i  = rMatchCands.begin();
       i != rMatchCands.end();
       ++i)
  {
    if (i->first.first == sr) {
      toErase.push_back(i);
    }
  }
  for (vector<RuleMatchMap::iterator>::iterator
       i  = toErase.begin();
       i != toErase.end();
       ++i)
  {
    rMatchCands.erase(*i);
  }
}

bool Matcher::getBodyMatch(const Rule& sr, const Rule& tr, BodyMapping& map) const {
  RuleMatchMap::const_iterator pos = rMatchCands.find(RulePair(sr, tr));
  if (pos == rMatchCands.end()) {
    return false;
  }
  map.insert(pos->second.begin(), pos->second.end());
  return true;
}

ostream& operator<<(ostream& os, const Matcher& m) {
  for(RuleMatchMap::const_iterator
      i  = m.rMatchCands.begin();
      i != m.rMatchCands.end();
      ++i)
  {
    os << i->first.first << "\n  --> " << i->first.second;
  }
  return os;
}
