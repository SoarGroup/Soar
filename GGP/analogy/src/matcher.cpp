#include <assert.h>
#include <math.h>
#include <algorithm>
#include <iterator>
#include <boost/bind.hpp>

#include "matcher.h"

using namespace std;
using boost::bind;

ostream& operator<<(ostream& os, BodyMapping& bm) {
  for (BodyMapping::iterator
       i  = bm.begin();
       i != bm.end();
       ++i)
  {
    os << i->first << " --> " << i->second << endl;
  }
  return os;
}

void bmStr(BodyMapping& bm) {
  cout << bm;
}

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
      vector<BodyMapping> mappings;
      vector<ScoredBodyMapping> scoredMappings;
      allBodyMappings(*i, *j, mappings);
      for(vector<BodyMapping>::iterator
          k  = mappings.begin();
          k != mappings.end();
          ++k)
      {
        double score = bodyMapScore(*i, *j, *k);
        scoredMappings.push_back(ScoredBodyMapping(*k, score));
      }
      rMatchCands.insert(RuleMatchType(RulePair(*i, *j), scoredMappings));
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

void Matcher::allBodyMappings
( const Rule& sr, 
  const Rule& tr, 
  vector<BodyMapping>& mappings) const
{
  BodyMapping dummy;

  vector<Condition> smaller;
  vector<Condition> larger;

  // if b is the body of the rule with fewer body conditions,
  // for every predicate p in b, there must be one predicate p' in
  // b' that p can match with
  if (sr.body_size() < tr.body_size()) {
    sr.get_body(smaller);
    tr.get_body(larger);
    allBodyMappingsInternal(smaller, larger, dummy, mappings);
  }
  else {
    vector<BodyMapping> revPossibleMaps;
    tr.get_body(smaller);
    sr.get_body(larger);
    BodyMapping tempMap;
    allBodyMappingsInternal(smaller, larger, dummy, revPossibleMaps);
    for(vector<BodyMapping>::iterator
        i  = revPossibleMaps.begin();
        i != revPossibleMaps.end();
        ++i)
    {
      BodyMapping temp;
      for(BodyMapping::iterator
          j  = i->begin();
          j != i->end();
          ++j)
      {
        temp.insert(PredPair(j->second, j->first));
      }
      mappings.push_back(temp);
    }
  }
}

//bool Matcher::findBodyMappingInternal
//( vector<Condition> smaller,  // by value on purpose
//  vector<Condition> larger,
//  BodyMapping& mapping ) const
//{
//  if (smaller.size() == 0) {
//    return true;
//  }
//  else {
//    Predicate p1 = smaller[0].pred;
//    bool p1n = smaller[0].negated;
//    smaller.erase(smaller.begin());
//    bool exhausted = false;
//    while (!exhausted) {
//      exhausted = true;
//      for (vector<Condition>::iterator 
//           i  = larger.begin();
//           i != larger.end();
//           ++i)
//      {
//        if (predicatesCanMatch(p1, i->pred) and p1n == i->negated) {
//          Predicate p2 = i->pred;
//          larger.erase(i);
//          if (findBodyMappingInternal(smaller, larger, mapping)) {
//            mapping.insert(PredPair(p1, p2));
//            return true;
//          }
//          else {
//            // note that we've erased a predicate that can't
//            // match in the larger set at this point
//            exhausted = false;
//            break;
//          }
//        }
//      }
//      // if we get to here and exhausted hasn't been set to false,
//      // then we've run through the entire larger set, and there are
//      // no matching candidates.
//    }
//    return false;
//  }
//}

bool Matcher::allBodyMappingsInternal
( vector<Condition> smaller,  // by value on purpose
  vector<Condition> larger,
  BodyMapping& partialMap,
  vector<BodyMapping>& mappings ) const
{
  if (smaller.size() == 0) {
    mappings.push_back(partialMap);
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
        double score = predicateMatchScore(p1, i->pred);
        if (score > 0 and p1n == i->negated) {
          Predicate p2 = i->pred;
          larger.erase(i);
          BodyMapping notAsPartialMap(partialMap);
          notAsPartialMap.insert(PredPair(p1, p2));
          if (allBodyMappingsInternal(smaller, larger, notAsPartialMap, mappings)) {
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

double Matcher::bodyMapScore
( const Rule& sr, 
  const Rule& tr, 
  const BodyMapping& m) const 
{
  int unmatched_conds = std::max(sr.body_size(), tr.body_size()) - m.size();
  double total = 0;
  for (BodyMapping::const_iterator
       i  = m.begin();
       i != m.end();
       ++i)
  {
    double score = predicateMatchScore(i->first, i->second);
    if (score == 0) {
      return 0;
    }
    total += score;
  }
  return total - unmatched_conds;
}

//bool Matcher::rulesCanMatch
//( const Rule& sr, 
//  const Rule& tr, 
//  BodyMapping& mapping) const
//{
//  // rules heads must be matchable
//  if (!predicatesCanMatch(sr.get_head(), tr.get_head())) {
//    return false;
//  }
//
//  // facts
//  if (sr.body_size() == 0 and tr.body_size() == 0) {
//    return true;
//  }
//
//  // can't match facts with rules
//  if (sr.body_size() == 0 or tr.body_size() == 0) {
//    return false;
//  }
//
//  BodyMapping tempMap;
//  if (findBodyMapping(sr, tr, tempMap)) {
//    mapping.insert(tempMap.begin(), tempMap.end());
//    return true;
//  }
//  else {
//    return false;
//  }
//}

/*
bool Matcher::rulesCannotMatch(const RuleMatchType& m) {
  return !rulesCanMatch(m.first.first, m.first.second, m.second);
}
*/

/* update the candidate rule matches given that sp and tp were just
 * mapped to one another.
 */
void Matcher::updateRuleMatches(const Predicate& sp, const Predicate& tp) {
  vector<RuleMatchMap::iterator> toErase;
  for (RuleMatchMap::iterator
       i  = rMatchCands.begin();
       i != rMatchCands.end();
       ++i)
  {
    vector<vector<ScoredBodyMapping>::iterator> toErase2;
    
    for(vector<ScoredBodyMapping>::iterator
        j  = i->second.begin();
        j != i->second.end();
        ++j)
    {
      double newScore = bodyMapScore(i->first.first, i->first.second, j->first);
      if (newScore == 0) {
        toErase2.push_back(j);
      }
      else {
        j->second = newScore;
      }
    }

    for(vector<vector<ScoredBodyMapping>::iterator>::iterator
        j  = toErase2.begin();
        j != toErase2.end();
        ++j)
    {
      i->second.erase(*j);
    }
    if (i->second.size() == 0) {
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

double Matcher::predicateMatchScore
( const Predicate& p1, 
  const Predicate& p2 ) const 
{
  map<Predicate, Predicate>::const_iterator p1_pos = matchedPreds.find(p1);
  map<Predicate, Predicate>::const_iterator p2_pos = matchedPreds.find(p2);
  if (p1_pos != matchedPreds.end() and p1_pos->second != p2) {
    return 0;
  }
  if (p2_pos != matchedPreds.end() and p2_pos->second != p1) {
    return 0;
  }
  double p1a = p1.get_arity() + 0.5;
  double p2a = p2.get_arity() + 0.5;
  double score = (p1a + p2a) / (fabs(p1a - p2a) + 1);
  if (p1.get_type() == p2.get_type()) {
    return 1.5 * score;
  }
  return score;
}

double Matcher::ruleMatchScore
( const Rule& r1, 
  const Rule& r2, 
  BodyMapping& bestMap) const 
{
  double score = 0;
  RuleMatchMap::const_iterator p = rMatchCands.find(RulePair(r1, r2));

  if (p == rMatchCands.end()) {
    return -1;
  }

  vector<ScoredBodyMapping>::const_iterator bestPos = p->second.begin();
  for (vector<ScoredBodyMapping>::const_iterator
       i  = p->second.begin();
       i != p->second.end();
       ++i)
  {
    if (i->second > score) {
      bestPos = i;
      score = i->second;
    }
  }
  if (predicateMatched(r1.get_head())) {
    score *= 2;
  }
  bestMap = bestPos->first;
  return score;
}

//int Matcher::ruleMatchScore(const Rule& r1, const Rule& r2) const {
//  int score = 0;
//  if (predicateMatched(r1.get_head())) {
//    score += 10;
//  }
//  vector<Condition> body;
//  if (r1.body_size() == r2.body_size()) {
//    score += 3;
//  }
//  if (r1.body_size() < r2.body_size()) {
//    r1.get_body(body);
//  }
//  else {
//    r2.get_body(body);
//  }
//  for(vector<Condition>::iterator
//      i =  body.begin();
//      i != body.end();
//      ++i)
//  {
//    if (predicateMatched(i->pred)) {
//      score += 1;
//    }
//  }
//  return score;
//}

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

bool Matcher::getBestMatch
( const Rule*& r1, 
  const Rule*& r2, 
  BodyMapping& bestMap) const 
{
  double bestScore = -1;
  r1 = NULL; r2 = NULL;
  for (RuleMatchMap::const_iterator
       i =  rMatchCands.begin();
       i != rMatchCands.end();
       ++i)
  {
    const Rule& sr = i->first.first;
    const Rule& tr = i->first.second;
    BodyMapping m;
    double score = ruleMatchScore(sr, tr, m);
    if (score > bestScore) {
      bestScore = score;
      r1 = &(sr);
      r2 = &(tr);
      bestMap = m;
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

  sourceRules.erase(sr);
  targetRules.erase(tr);

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
    else if (i->first.second == tr) {
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

bool Matcher::getBodyMaps(const Rule& sr, const Rule& tr, vector<BodyMapping>& map) const {
  RuleMatchMap::const_iterator pos = rMatchCands.find(RulePair(sr, tr));
  if (pos == rMatchCands.end()) {
    return false;
  }
  for (vector<ScoredBodyMapping>::const_iterator
       i  = pos->second.begin();
       i != pos->second.end();
       ++i)
  {
    map.push_back(i->first);
  }
  return true;
}

void Matcher::getUnmatchedPreds(vector<Predicate>& sp, vector<Predicate>& tp) {
  sp.insert(sp.begin(), sourcePreds.begin(), sourcePreds.end());
  tp.insert(tp.begin(), targetPreds.begin(), targetPreds.end());
}

void Matcher::getUnmatchedRules(vector<Rule>& sr, vector<Rule>& tr) {
  sr.insert(sr.begin(), sourceRules.begin(), sourceRules.end());
  tr.insert(tr.begin(), targetRules.begin(), targetRules.end());
}

ostream& operator<<(ostream& os, const Matcher& m) {
  for(RuleMatchMap::const_iterator
      i  = m.rMatchCands.begin();
      i != m.rMatchCands.end();
      ++i)
  {
    os << i->first.first << "\n  --> " << i->first.second;
    os << endl;
  }
  return os;
}

