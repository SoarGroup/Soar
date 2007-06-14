#ifndef __RuleMatch_H__
#define __RuleMatch_H__

class RuleMatch {
  public:
    RuleMatch(const Rule& source_rule, const Rule& target_rule);
    bool update(const Predicate& sp, const Predicate& tp);

  private:
    Rule sr;
    Rule tr;
    map<Predicate, Predicate> bodyMap;
};

#endif
