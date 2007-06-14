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
typedef pair<RulePair, BodyMapping > RuleMatchType;
typedef map<RulePair, BodyMapping > RuleMatchMap;

class Matcher {
  public:
    Matcher
    ( vector<Predicate> sourcePreds,
      vector<Predicate> targetPreds,
      set<Rule> sourceRules,
      set<Rule> targetRules );

    Matcher(const Matcher& other);

    bool findBodyMapping(const Rule& sr, const Rule& tr, BodyMapping& mapping) const;

    bool predicatesCanMatch(const Predicate& p1, const Predicate& p2) const;
    bool predicateMatched(const Predicate& p) const { 
      return matchedPreds.find(p) != matchedPreds.end();
    }
    void addPredicateMatch(const Predicate& sp, const Predicate& tp);
    
    bool rulesCanMatch(const Rule& r1, const Rule& r2, BodyMapping& map) const;
    int  ruleMatchScore(const Rule& r1, const Rule& r2) const;
    void addRuleMatch(const Rule& sr, const Rule& tr);

    bool getBestMatch(const Rule*& r1, const Rule*& r2) const;
    bool getBodyMatch(const Rule& sr, const Rule& tr, BodyMapping& map) const;

    friend ostream& operator<<(ostream& os, const Matcher& m);

  private:
    void updateRuleMatches(const Predicate& sp, const Predicate& tp);

    bool findBodyMappingInternal
         ( vector<Condition> smaller, 
           vector<Condition> larger,
           BodyMapping& mapping ) const;

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
