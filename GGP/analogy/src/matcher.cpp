/* How to map predicates:
 */
/* How to map places:
 * 1. Align places locally
 *      A. line up predicates
 *      B. Enumerate all consistent mappings, choose best one
 *           - Start with most constrained predicate, progress to less constrained
 *    Now for each place we have a list of mappings that are optimal within the rules,
 *    in other words, for each place->place pair, we know which rules consider 
 *    that mapping optimal
 * 2. Select place pair with "best" alignment and commit to that mapping
 *      "Best" meaning having the most positive matches and the least
 *      negative matches
 * 3. Find all constraints involving that place pair, and commit to the
 *    consequential mappings
 */

#include <assert.h>
#include <math.h>
#include <algorithm>
#include <iterator>
#include <boost/bind.hpp>

#include "matcher.h"
#include "mapper.h"

using namespace std;
using boost::bind;

ostream& operator<<(ostream& os, PredMapping& bm) {
  for (PredMapping::iterator
       i  = bm.begin();
       i != bm.end();
       ++i)
  {
    os << i->first << " --> " << i->second << endl;
  }
  return os;
}

void bmStr(PredMapping& bm) {
  cout << bm;
}

Matcher::Matcher
( Pred2PlaceTypeMap& sPreds,
  Pred2PlaceTypeMap& tPreds,
  RulePtrSet& sRules,
  RulePtrSet& tRules )
: sourceRules(sRules.begin(), sRules.end()), 
  targetRules(tRules.begin(), tRules.end()),
  sourcePreds(sPreds), 
  targetPreds(tPreds)
{
  for (RulePtrSet::iterator 
       i  = sourceRules.begin(); 
       i != sourceRules.end();
       ++i)
  {
    for (RulePtrSet::iterator
         j  = targetRules.begin();
         j != targetRules.end();
         ++j)
    {
      vector<PredMapping> mappings;
      vector<ScoredBodyMapping> scoredMappings;
      allBodyMappings(*i, *j, mappings);
      for(vector<PredMapping>::iterator
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
  Pred2PlaceTypeMap::iterator i = sourcePreds.begin();
  while (i != sourcePreds.end()) {
    Pred2PlaceTypeMap::iterator tPos = targetPreds.find(i->first);
    if (tPos != targetPreds.end()) {
      matchedPreds.insert(PredPair(i->first, i->first));
      updateRuleMatches(i->first, i->first);
      Pred2PlaceTypeMap::iterator j = i;
      ++j;
      sourcePreds.erase(i);
      i = j;
      targetPreds.erase(tPos);
    }
    else {
      ++i;
    }
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
( const RulePtr& sr, 
  const RulePtr& tr, 
  vector<PredMapping>& mappings) const
{
  PredMapping dummy;

  vector<Condition> smaller;
  vector<Condition> larger;

  // if b is the body of the rule with fewer body conditions,
  // for every predicate p in b, there must be one predicate p' in
  // b' that p can match with
  if (sr->body_size() < tr->body_size()) {
    sr->get_body(smaller);
    tr->get_body(larger);
    allBodyMappingsInternal(smaller, larger, dummy, mappings, false);
  }
  else {
    vector<PredMapping> revPossibleMaps;
    tr->get_body(smaller);
    sr->get_body(larger);
    PredMapping tempMap;
    allBodyMappingsInternal(smaller, larger, dummy, revPossibleMaps, true);
    for(vector<PredMapping>::iterator
        i  = revPossibleMaps.begin();
        i != revPossibleMaps.end();
        ++i)
    {
      PredMapping temp;
      for(PredMapping::iterator
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
//  PredMapping& mapping ) const
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
  PredMapping& partialMap,
  vector<PredMapping>& mappings,
  bool isRev) const
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
      vector<Condition>::iterator i = larger.begin();
      while (i != larger.end()) {
        double score;
        if (isRev) {
          score = predicateMatchScore(i->pred, p1);
        }
        else {
          score = predicateMatchScore(p1, i->pred);
        }
        if (score > 0 and p1n == i->negated) {
          Predicate p2 = i->pred;
          i = larger.erase(i);
          PredMapping notAsPartialMap(partialMap);
          notAsPartialMap.insert(PredPair(p1, p2));
          if (allBodyMappingsInternal(smaller, larger, notAsPartialMap, mappings, isRev)) {
            return true;
          }
          else {
            // note that we've erased a predicate that can't
            // match in the larger set at this point
            exhausted = false;
            break;
          }
        }
        else {
          ++i;
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
( const RulePtr& sr, 
  const RulePtr& tr, 
  const PredMapping& m) const 
{
  int unmatched_conds = std::max(sr->body_size(), tr->body_size()) - m.size();
  double total = 0;
  for (PredMapping::const_iterator
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
//  PredMapping& mapping) const
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
//  PredMapping tempMap;
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
  RuleMatchMap::iterator i = rMatchCands.begin();
  while (i != rMatchCands.end()) {
    vector<ScoredBodyMapping>::iterator j = i->second.begin();
    while (j != i->second.end()) {
      double newScore = bodyMapScore(i->first.first, i->first.second, j->first);
      if (newScore == 0) {
        // this body map is no longer valid
        j = i->second.erase(j);
      }
      else {
        j->second = newScore;
        ++j;
      }
    }
    if (i->second.size() == 0) {
      RuleMatchMap::iterator temp = i;
      ++i;
      rMatchCands.erase(temp);
    }
    else {
      ++i;
    }
  }
}

double Matcher::predicateMatchScore
( const Predicate& sp, 
  const Predicate& tp ) const 
{
  map<Predicate, Predicate>::const_iterator sp_pos = matchedPreds.find(sp);
  map<Predicate, Predicate>::const_iterator tp_pos = matchedPreds.find(tp);
  if (sp_pos != matchedPreds.end() and sp_pos->second != tp) {
    return 0;
  }
  if (tp_pos != matchedPreds.end() and tp_pos->second != sp) {
    return 0;
  }
  if (sp.get_arity() == 0 and tp.get_arity() == 0) {
    return 1;
  }
  
  set<PlaceType> spTypes = sourcePreds.find(sp)->second;
  set<PlaceType> tpTypes = targetPreds.find(tp)->second;
  
  vector<PlaceType> intersection;
  set_intersection(spTypes.begin(), spTypes.end(), 
                   tpTypes.begin(), tpTypes.end(), 
                   back_inserter(intersection));
  double score = intersection.size() / (spTypes.size() + tpTypes.size());
  return score;
}

double Matcher::ruleMatchScore
( const RulePtr& r1, 
  const RulePtr& r2, 
  PredMapping& bestMap) const 
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
  if (srcPredMatched(r1->get_head())) {
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
( RulePtr& r1, 
  RulePtr& r2, 
  PredMapping& bestMap) const 
{
  double bestScore = -1;
  r1 = RulePtr((Rule*)NULL); r2 = RulePtr((Rule*)NULL);
  for (RuleMatchMap::const_iterator
       i =  rMatchCands.begin();
       i != rMatchCands.end();
       ++i)
  {
    const RulePtr& sr = i->first.first;
    const RulePtr& tr = i->first.second;
    PredMapping m;
    double score = ruleMatchScore(sr, tr, m);
    if (score > bestScore) {
      bestScore = score;
      r1 = sr;
      r2 = tr;
      bestMap = m;
    }
  }
  if (r1.get() == NULL or r2.get() == NULL) {
    // no more match candidates
    return false;
  }
  else {
    return true;
  }
}

void Matcher::addPredicateMatch(const Predicate& sp, const Predicate& tp) {
  assert(!srcPredMatched(sp));
  assert(!tgtPredMatched(tp));

  matchedPreds.insert(PredPair(sp, tp));
  if (sp != tp) {
    matchedPreds.insert(PredPair(tp, sp));
  }
  cout << "MATCH " << sp << " " << tp << endl;

  sourcePreds.erase(sp);
  targetPreds.erase(tp);
  updateRuleMatches(sp, tp);
}

void Matcher::addRuleMatch(const RulePtr& sr, const RulePtr& tr) {
  DEBUG(cout << "RULE MATCH " << *sr << " TO " << *tr << endl;)

  sourceRules.erase(sr);
  targetRules.erase(tr);

  matchedRules.insert(RulePair(sr, tr));
  //rMatchCands.erase(remove_if(rMatchCands.begin(), rMatchCands.end(), bind(&RuleMatchType::first, _1) == rp), rMatchCands.end());
  for(RuleMatchMap::iterator i = rMatchCands.begin(); i != rMatchCands.end(); ++i) {
    if (*(i->first.first) == *sr) {
      rMatchCands.erase(i);
    }
    else if (*(i->first.second) == *tr) {
      rMatchCands.erase(i);
    }
  }
  /*
  RuleMatchMap::iterator i = rMatchCands.begin(); 
  while (i != rMatchCands.end()) {
    if (i->first.first == sr) {
      RuleMatchMap::iterator temp = i;
      ++i;
      rMatchCands.erase(temp);
    }
    else if (i->first.second == tr) {
      RuleMatchMap::iterator temp = i;
      ++i;
      rMatchCands.erase(temp);
    }
    else {
      ++i;
    }
  }
  */
}

bool Matcher::getBodyMaps
( const RulePtr& sr, 
  const RulePtr& tr, 
  vector<PredMapping>& map) const
{
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

void Matcher::getUnmatchedPreds
( vector<Predicate>& sp, vector<Predicate>& tp )
{
  for (Pred2PlaceTypeMap::iterator 
       i  = sourcePreds.begin();
       i != sourcePreds.end();
       ++i)
  {
    sp.push_back(i->first);
  }
  for (Pred2PlaceTypeMap::iterator 
       i  = targetPreds.begin();
       i != targetPreds.end();
       ++i)
  {
    tp.push_back(i->first);
  }
}

void Matcher::getUnmatchedRules
( vector<RulePtr>& sr, vector<RulePtr>& tr)
{
  sr.insert(sr.begin(), sourceRules.begin(), sourceRules.end());
  tr.insert(tr.begin(), targetRules.begin(), targetRules.end());
}

bool Matcher::srcPredMatched(const Predicate& p) const { 
  return matchedPreds.find(p) != matchedPreds.end();
}

bool Matcher::tgtPredMatched(const Predicate& p) const {
  for(PredMapping::const_iterator
      i  = matchedPreds.begin();
      i != matchedPreds.end();
      ++i)
  {
    if (i->second == p) {
      return true;
    }
  }
  return false;
}

Predicate Matcher::getSrcPredMatch(const Predicate& p) const {
  assert(srcPredMatched(p));
  PredMapping::const_iterator pos = matchedPreds.find(p);
  return pos->first;
}

Predicate Matcher::getTgtPredMatch(const Predicate& p) const {
  for (PredMapping::const_iterator
       i  = matchedPreds.begin();
       i != matchedPreds.end();
       ++i)
  {
    if (i->second == p) {
      return i->first;
    }
  }
  assert(false);
}

bool Matcher::hasPredMatchConflict
( const Predicate& sp, const Predicate& tp) const 
{
  if (srcPredMatched(sp) and getSrcPredMatch(sp) != tp) {
    return true;
  }
  if (tgtPredMatched(tp) and getTgtPredMatch(tp) != sp) {
    return true;
  }
  return false;
}


ostream& operator<<(ostream& os, const Matcher& m) {
  for(RuleMatchMap::const_iterator
      i  = m.rMatchCands.begin();
      i != m.rMatchCands.end();
      ++i)
  {
    os << *(i->first.first) << "\n  --> " << *(i->first.second);
    os << endl;
  }
  return os;
}

