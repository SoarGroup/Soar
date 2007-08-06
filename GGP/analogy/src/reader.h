#ifndef __Reader_H__
#define __Reader_H__

#include <assert.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <set>
#include <fstream>

#include "rule.h"
#include "predicate.h"
#include "matcher.h"

using namespace std;

class Reader {
public:
  Reader(char* filename) : lineno(0), file(filename) {}

  ~Reader() {
    file.close();
  }

  Predicate read_function(set<Predicate>& preds, PredicateType type) {
    string name = read_nonempty();
    int arity = 0;
    string line;
    while (true) {
      line = read_nonempty();
      if (line == "XXX_FUNCTION_END") {
        return Predicate(name, arity, type);
      }
      else if (line == "XXX_FUNCTION_BEGIN") {
        Predicate p1 = read_function(preds, FUNCTION);
        preds.insert(p1);
      }
      arity++;
    }
  }

  Predicate read_sentence(set<Predicate>& preds) {
    string relation = read_nonempty();
    if (relation == "next" or relation == "true") {
      expect("XXX_FUNCTION_BEGIN");

      Predicate p = read_function(preds, PERSISTENT);
      preds.insert(p);
      expect("XXX_SENT_END");
      return p;
    }
    else if (relation == "legal" or relation == "does") {
      string role = read_nonempty(); // skip the role line

      string temp = read_nonempty();
      if (temp == "XXX_FUNCTION_BEGIN") {
        // move with parameters
        Predicate p = read_function(preds, MOVE);
        preds.insert(p);
        expect("XXX_SENT_END");
        return p;
      }
      else {
        // move without parameters
        Predicate p(temp, 0, MOVE);
        preds.insert(p);
        expect("XXX_SENT_END");
        return p;
      }
    }
    else {
      int arity = 0;
      string line;
      while (true) {
        line = read_nonempty();
        if (line == "XXX_SENT_END") {
          Predicate p(relation, arity, ENTAILMENT);
          preds.insert(p);
          return p;
        }
        else if (line == "XXX_FUNCTION_BEGIN") {
          read_function(preds, FUNCTION);
        }
        arity++;
      }
    }
  }

  /* Right now I'm treating ors as if they don't exist */
  void read_or(set<Predicate>& preds, RulePtr& r) {
    string line;
    while (1) {
      line = read_nonempty();
      if (line == "XXX_SENT_BEGIN") {
        r->add_body(read_sentence(preds), false);
      }
      else if (line == "XXX_NOT") {
        expect("XXX_SENT_BEGIN");
        r->add_body(read_sentence(preds), true);
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

  void read_xkif(RulePtrSet& rules, set<Predicate>& preds) {
    int state = 0;
    RulePtr rule;
    string line = "temp";
    bool nextSentNegated = false;
    while (true) {
      switch (state) {
        case 0:
          line = read_nonempty();
          if (line.empty()) {
            // finished
            return;
          }
          else if (line != "XXX_RULE_START") {
            cerr << "Expected XXX_RULE_START at line " << lineno << ", got " << line << endl;
            exit(1);
          }
          expect("XXX_SENT_BEGIN");
          rule = RulePtr(new Rule(read_sentence(preds)));
          state = 1;
          break;
        case 1:
          line = read_nonempty();
          if (line == "XXX_RULE_END") {
            if (rule->body_size() > 0) {
              // discard facts and inits
              rules.insert(rule);
            }
            state = 0;
          }
          else if (line == "XXX_SENT_BEGIN") {
            rule->add_body(read_sentence(preds), nextSentNegated);
            nextSentNegated = false;
          }
          else if (line == "XXX_NOT") {
            nextSentNegated = true;
            break;
          }
          else if (line == "XXX_OR_BEGIN") {
            read_or(preds, rule);
          }
          else {
            cout << "Unexpected line at " << lineno << endl;
            assert(false);
          }
          break;
      }
    }
  }

private:

  string read_nonempty() {
    string line = "";
    while (!file.eof() and line.find_first_not_of(" ") == string::npos) {
      getline(file, line);
      ++lineno;
    }
    assert(strlen(line.c_str()) > 0 || file.eof());
    return line;
  }

  void expect(string expected) {
    string got = read_nonempty();
    if (got != expected) {
      cerr << "Expected " << expected << " at line " << lineno << ", got " << got << endl;
      exit(1);
    }
  }

  int lineno;
  ifstream file;
};

#endif
