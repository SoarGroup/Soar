#include <assert.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <set>

#include "rule.h"
#include "matcher.h"

using namespace std;

string read_nonempty(ifstream& file) {
  string line = "";
  while (!file.eof() and line.find_first_not_of(" ") == string::npos) {
    getline(file, line);
  }
  return line;
}

Predicate read_function(ifstream& file, set<Predicate>& preds, PredicateType type) {
  string name = read_nonempty(file);
  int arity = 0;
  string line;
  while (true) {
    line = read_nonempty(file);
    if (line == "XXX_FUNCTION_END") {
      return Predicate(name, arity, type);
    }
    else if (line == "XXX_FUNCTION_BEGIN") {
      Predicate p1 = read_function(file, preds, FUNCTION);
      preds.insert(p1);
    }
    arity++;
  }
}

Predicate read_sentence(ifstream& file, set<Predicate>& preds) {
  string relation = read_nonempty(file);
  if (relation == "next" or relation == "true") {
    assert(read_nonempty(file) == "XXX_FUNCTION_BEGIN");

    Predicate p = read_function(file, preds, PERSISTENT);
    preds.insert(p);
    assert(read_nonempty(file) == "XXX_SENT_END");
    return p;
  }
  else if (relation == "legal" or relation == "does") {
    string role = read_nonempty(file);

    assert(read_nonempty(file) == "XXX_FUNCTION_BEGIN");

    Predicate p = read_function(file, preds, MOVE);
    preds.insert(p);
    assert(read_nonempty(file) == "XXX_SENT_END");
    return p;
  }
  else {
    int arity = 0;
    string line;
    while (true) {
      line = read_nonempty(file);
      if (line == "XXX_SENT_END") {
        Predicate p(relation, arity, ENTAILMENT);
        preds.insert(p);
        return p;
      }
      else if (line == "XXX_FUNCTION_BEGIN") {
        read_function(file, preds, FUNCTION);
      }
      arity++;
    }
  }
}

/* Right now I'm treating ors as if they don't exist */
void read_or(ifstream& file, set<Predicate>& preds, Rule& r) {
  string line;
  while (1) {
    line = read_nonempty(file);
    if (line == "XXX_SENT_BEGIN") {
      r.add_body(read_sentence(file, preds), false);
    }
    else if (line == "XXX_NOT") {
      assert(read_nonempty(file) == "XXX_SENT_BEGIN");
      r.add_body(read_sentence(file, preds), true);
    }
    else if (line == "XXX_OR_END") {
      return;
    }
    else {
      cout << line << endl;
      assert(false);
    }
  }
}


void read_xkif(ifstream& input, set<Rule>& rules, set<Predicate>& preds) {
  int state = 0;
  Rule* rule;
  string line = read_nonempty(input);
  bool nextSentNegated = false;
  while (!line.empty()) {
    switch (state) {
      case 0:
        if (line == "XXX_RULE_START") {
          assert(read_nonempty(input) == "XXX_SENT_BEGIN");
          rule = new Rule(read_sentence(input, preds));
          state = 1;
        }
        else {
          cout << line << endl;
          assert(false);
        }
        break;
      case 1:
        if (line == "XXX_RULE_END") {
          if (rule->body_size() > 0) {
            // discard facts and inits
            rules.insert(*rule);
          }
          state = 0;
        }
        else if (line == "XXX_SENT_BEGIN") {
          rule->add_body(read_sentence(input, preds), nextSentNegated);
          nextSentNegated = false;
        }
        else if (line == "XXX_NOT") {
          nextSentNegated = true;
          break;
        }
        else if (line == "XXX_OR_BEGIN") {
          read_or(input, preds, *rule);
        }
        break;
    }
    line = read_nonempty(input);
  }
  input.close();
}

