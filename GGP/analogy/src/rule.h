#ifndef __RULE_H__
#define __RULE_H__

#include <set>
#include <vector>
#include <iostream>

#include "predicate.h"

using namespace std;

struct Condition {
  Predicate pred;
  bool negated;

  Condition(const Predicate& p, bool n) : pred(p), negated(n) {}

  bool operator<(const Condition& other) const {
    if (pred != other.pred) {
      return pred < other.pred;
    }
    if (negated and !other.negated) {
      return true;
    }
    return false;
  }
  
  bool operator==(const Condition& other) const {
    return pred == other.pred && negated == other.negated;
  }
};

class Rule {

  public:
    Rule(const Predicate& h)
      : head(h)
    {}

    Rule(const Rule& other) : head(other.head), body(other.body) {}

    Predicate get_head() const {
      return head;
    }

    void add_body(const Predicate& p, bool negated) {
      //body.insert(b);
      body.push_back(Condition(p, negated));
    }

    void get_body(vector<Condition>& b) const {
      b.insert(b.begin(), body.begin(), body.end());
    }

    bool in_body(const Predicate& p, bool negated) const {
      return find(body.begin(), body.end(), Condition(p, negated)) != body.end();
      //return body.find(p) != body.end();
    }

    int body_size() const {
      return body.size();
    }

//    void body_intersect(const Rule& other, set<Predicate>& intersect) const {
//      set_intersection(body.begin(), body.end(), other.body.begin(), other.body.end(), 
//          inserter(intersect, intersect.begin()));
//    }

    bool operator<(const Rule& other) const {
      if (head != other.head) {
        return head < other.head;
      }
      return body < other.body;
    }

    friend ostream& operator<<(ostream& os, const Rule& r);

    bool operator==(const Rule& other) const {
      if (head != other.head) {
        return false;
      }
      return body == other.body;
    }

  private:
    Predicate head;
    vector<Condition> body;
};

#endif
