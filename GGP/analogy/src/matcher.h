#ifndef __Matcher_H__
#define __Matcher_H__

#include <vector>
#include <map>
#include <utility>

#include "predicate.h"
#include "rule.h"

using namespace std;

enum PlaceType { OBJECT, COORD, NUMBER, UNKNOWN };

typedef pair<RulePtr, RulePtr> RulePair;

struct ltRulePair {
  bool operator()(const RulePair& rp1, const RulePair& rp2) const {
    if (*(rp1.first) < *(rp2.first)) {
      return true;
    }
    return *(rp1.second) < *(rp2.second);
  }
};

typedef map<RulePtr, RulePtr, ltRulePtr> RuleMap;
typedef pair<Predicate, Predicate> PredPair;
typedef map<Predicate, Predicate> PredMapping;
typedef pair<PredMapping, double> ScoredBodyMapping;
typedef pair<RulePair, vector<ScoredBodyMapping> > RuleMatchType;
typedef map<RulePair, vector<ScoredBodyMapping>, ltRulePair> RuleMatchMap;
typedef map<Predicate, set<PlaceType> > Pred2PlaceTypeMap;

class Matcher {
  public:
    Matcher
    ( Pred2PlaceTypeMap& sourcePreds,
      Pred2PlaceTypeMap& targetPreds,
      RulePtrSet& sourceRules,
      RulePtrSet& targetRules );

    Matcher(const Matcher& other);

    bool findBestBodyMapping(const RulePtr sr, const RulePtr tr, PredMapping& mapping) const;

    double predicateMatchScore(const Predicate& p1, const Predicate& p2) const;
    bool srcPredMatched(const Predicate& p) const;
    bool tgtPredMatched(const Predicate& p) const;

    Predicate getSrcPredMatch(const Predicate& p) const;
    Predicate getTgtPredMatch(const Predicate& p) const;

    bool hasPredMatchConflict(const Predicate& p1, const Predicate& p2) const;

    void addPredicateMatch(const Predicate& sp, const Predicate& tp);
    
    bool rulesCanMatch(const RulePtr& r1, const RulePtr& r2, PredMapping& map) const;
    double ruleMatchScore(const RulePtr& r1, const RulePtr& r2, PredMapping& bestMap) const;
    void addRuleMatch(const RulePtr& sr, const RulePtr& tr);

    bool getBestMatch(RulePtr& r1, RulePtr& r2, PredMapping& bestMap) const;

    void getUnmatchedPreds(vector<Predicate>& sp, vector<Predicate>& tp);

    void getUnmatchedRules(vector<RulePtr>& sr, vector<RulePtr>& tr);

    friend ostream& operator<<(ostream& os, const Matcher& m);

  private:
    bool getBodyMaps(const RulePtr& sr, const RulePtr& tr, vector<PredMapping>& map) const;
    void updateRuleMatches(const Predicate& sp, const Predicate& tp);

    bool findBodyMappingInternal
         ( vector<Condition> smaller, 
           vector<Condition> larger,
           PredMapping& mapping ) const;
    
    void allBodyMappings
    ( const RulePtr& sr, 
      const RulePtr& tr, 
      vector<PredMapping>& mappings) const;

    bool allBodyMappingsInternal
         ( vector<Condition> smaller,  // by value on purpose
           vector<Condition> larger,
           PredMapping& partialMap,
           vector<PredMapping>& mappings,
           bool isRev) const;

    double bodyMapScore(const RulePtr& sr, const RulePtr& tr, const PredMapping& m) const;

  private:
    RulePtrSet sourceRules;
    RulePtrSet targetRules;
    Pred2PlaceTypeMap sourcePreds;
    Pred2PlaceTypeMap targetPreds;
    map<Predicate, Predicate> matchedPreds;
    RuleMap matchedRules;
    RuleMatchMap rMatchCands;
};

ostream& operator<<(ostream& os, const pair<RulePtr, vector<RulePtr> >& p);
ostream& operator<<(ostream& os, const Matcher& m);

#endif
