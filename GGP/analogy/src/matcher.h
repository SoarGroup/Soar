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
typedef map<Predicate, Predicate> BodyMapping;
typedef pair<BodyMapping, double> ScoredBodyMapping;
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

    bool findBestBodyMapping(const Rule& sr, const Rule& tr, BodyMapping& mapping) const;

    double predicateMatchScore(const Predicate& p1, const Predicate& p2) const;
    bool predicateMatched(const Predicate& p) const { 
      return matchedPreds.find(p) != matchedPreds.end();
    }
    void addPredicateMatch(const Predicate& sp, const Predicate& tp);
    
    bool rulesCanMatch(const Rule& r1, const Rule& r2, BodyMapping& map) const;
    double ruleMatchScore(const Rule& r1, const Rule& r2, BodyMapping& bestMap) const;
    void addRuleMatch(const Rule& sr, const Rule& tr);

    bool getBestMatch(const Rule*& r1, const Rule*& r2, BodyMapping& bestMap) const;

    void getUnmatchedPreds(vector<Predicate>& sp, vector<Predicate>& tp);

    void getUnmatchedRules(vector<Rule>& sr, vector<Rule>& tr);

    friend ostream& operator<<(ostream& os, const Matcher& m);

  private:
    bool getBodyMaps(const Rule& sr, const Rule& tr, vector<BodyMapping>& map) const;
    void updateRuleMatches(const Predicate& sp, const Predicate& tp);

    bool findBodyMappingInternal
         ( vector<Condition> smaller, 
           vector<Condition> larger,
           BodyMapping& mapping ) const;
    
    void allBodyMappings
    ( const Rule& sr, 
      const Rule& tr, 
      vector<BodyMapping>& mappings) const;

    bool allBodyMappingsInternal
         ( vector<Condition> smaller,  // by value on purpose
           vector<Condition> larger,
           BodyMapping& partialMap,
           vector<BodyMapping>& mappings ) const;

    double bodyMapScore(const Rule& sr, const Rule& tr, const BodyMapping& m) const;

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
