#ifndef __Matcher_H__
#define __Matcher_H__

#include <vector>
#include <map>
#include <utility>

#include "predicate.h"
#include "rule.h"

using namespace std;

typedef pair<Rule, Rule> RulePair;
typedef pair<Predicate, Predicate> PredPair;
typedef map<Predicate, Predicate> PredMapping;
typedef pair<PredMapping, double> ScoredBodyMapping;
typedef pair<RulePair, vector<ScoredBodyMapping> > RuleMatchType;
typedef map<RulePair, vector<ScoredBodyMapping> > RuleMatchMap;

class Matcher {
  public:
    Matcher
    ( vector<Predicate> sourcePreds,
      vector<Predicate> targetPreds,
      set<Rule> sourceRules,
      set<Rule> targetRules );

    Matcher(const Matcher& other);

    bool findBestBodyMapping(const Rule& sr, const Rule& tr, PredMapping& mapping) const;

    double predicateMatchScore(const Predicate& p1, const Predicate& p2) const;
    bool srcPredMatched(const Predicate& p) const;
    bool tgtPredMatched(const Predicate& p) const;

    Predicate getSrcPredMatch(const Predicate& p) const;
    Predicate getTgtPredMatch(const Predicate& p) const;

    bool hasPredMatchConflict(const Predicate& p1, const Predicate& p2) const;

    void addPredicateMatch(const Predicate& sp, const Predicate& tp);
    
    bool rulesCanMatch(const Rule& r1, const Rule& r2, PredMapping& map) const;
    double ruleMatchScore(const Rule& r1, const Rule& r2, PredMapping& bestMap) const;
    void addRuleMatch(const Rule& sr, const Rule& tr);

    bool getBestMatch(const Rule*& r1, const Rule*& r2, PredMapping& bestMap) const;

    void getUnmatchedPreds(vector<Predicate>& sp, vector<Predicate>& tp);

    void getUnmatchedRules(vector<Rule>& sr, vector<Rule>& tr);

    friend ostream& operator<<(ostream& os, const Matcher& m);

  private:
    bool getBodyMaps(const Rule& sr, const Rule& tr, vector<PredMapping>& map) const;
    void updateRuleMatches(const Predicate& sp, const Predicate& tp);

    bool findBodyMappingInternal
         ( vector<Condition> smaller, 
           vector<Condition> larger,
           PredMapping& mapping ) const;
    
    void allBodyMappings
    ( const Rule& sr, 
      const Rule& tr, 
      vector<PredMapping>& mappings) const;

    bool allBodyMappingsInternal
         ( vector<Condition> smaller,  // by value on purpose
           vector<Condition> larger,
           PredMapping& partialMap,
           vector<PredMapping>& mappings ) const;

    double bodyMapScore(const Rule& sr, const Rule& tr, const PredMapping& m) const;

  private:
    set<Rule> sourceRules;
    set<Rule> targetRules;
    vector<Predicate> sourcePreds;
    vector<Predicate> targetPreds;
    map<Predicate, Predicate> matchedPreds;
    map<Rule, Rule> matchedRules;
    RuleMatchMap rMatchCands;
};

ostream& operator<<(ostream& os, const pair<Rule, vector<Rule> >& p);
ostream& operator<<(ostream& os, const Matcher& m);

#endif
