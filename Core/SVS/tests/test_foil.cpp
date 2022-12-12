#include <iostream>
#include <cstring>
#include <string>
#include <vector>
#include <cassert>
#include "foil.h"
#include "common.h"

#define MAX_CLAUSES 10
#define EQUAL_MARGIN 1.0e-3

struct clause_test_spec
{
    const char* clause;
    double success_rate;
};

struct test_spec
{
    const char* path;
    bool prune;
    clause_test_spec clauses[MAX_CLAUSES];
};

test_spec tests[] =
{
    {
        "foil_tests/1.test", true, {
            { "intersect(0,1,2) & ramp(0,2)", 1.0 },
            { NULL, 0.0}
        }  // sentinel
    },
    {
        "foil_tests/neg.test", true, {
            { "~indicator(0,1)", 1.0 },
            { NULL, 0.0}
        }
    },
    {
        "foil_tests/ramp.test", true, {
            { "intersect(0,1,2)", 0.975 },
            { NULL, 0.0}
        }
    },
    {
        "foil_tests/ramp_prune.test", true, {
            { "~intersect(0,1,-1)",          0.889 },
            { "intersect(0,1,2) & box(0,2)", 0.405 },
            { NULL, 0.0}
        }
    },
    { NULL, false, { {NULL, 0.0} } }  // sentinel
};

typedef vector<clause> clause_vec;

// functions to make literals and clauses from a readable format
literal PL(const string& s);
clause PC(const string& s);

bool close(double a, double b);
double time();
void clause_from_str(const string& s, clause& c);
bool run_foil(const char* path, bool prune, clause_vec& clauses, relation& pos, relation& neg, relation_table& all_rels, double& time);
void standalone(const char* path, bool prune);
bool test();
void test_clauses(clause_vec& clauses, relation& pos, relation& neg, relation_table& all_rels);

void fix_variables(int num_auto_bound, clause& c); // in foil.cpp
bool test_fix_variables();

int main(int argc, char* argv[])
{
    if (argc > 1)
    {
        if (strcmp(argv[1], "-p") == 0)
        {
            standalone(argv[2], false);
        }
        else
        {
            standalone(argv[1], true);
        }
        exit(0);
    }

    test();
    test_fix_variables();
    return 0;
}


double time()
{
    timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (ts.tv_sec * 1e9 + ts.tv_nsec) / 1.0e6;
}


bool run_foil(const char* path, bool prune, clause_vec& clauses, relation& pos, relation& neg, relation_table& all_rels, double& t)
{
    FOIL foil;
    ifstream input(path);

    if (!input)
    {
        cerr << "could not load " << path << std::endl;
        assert(false);
    }

    if (!foil.load_foil6(input))
    {
        return false;
    }

    input.close();
    double t1 = time();
    foil.learn(prune, false);
    t = time() - t1;
    for (int i = 0, iend = foil.num_clauses(); i < iend; ++i)
    {
        clauses.push_back(foil.get_clause(i));
    }
    pos = foil.get_pos();
    neg = foil.get_neg();
    all_rels = foil.get_relations();
    return true;
}

void standalone(const char* path, bool prune)
{
    clause_vec clauses;
    relation pos, neg;
    relation_table all;
    double t;

    if (!run_foil(path, prune, clauses, pos, neg, all, t))
    {
        assert(false);
    }

    std::cout << t << " msecs" << std::endl;
    for (int i = 0, iend = clauses.size(); i < iend; ++i)
    {
        std::cout << i << ": " << clauses[i] << std::endl;
    }
    test_clauses(clauses, pos, neg, all);
}

bool test()
{
    stringstream ss;
    clause_vec clauses;
    relation pos, neg;
    relation_table all;
    double t, success, fp, fn;
    int i, j;
    bool result = true;

    for (int i = 0; tests[i].path != NULL; ++i)
    {
        //if (i == 3) LOG.turn_on(FOILDBG);
        clauses.clear();
        if (!run_foil(tests[i].path, tests[i].prune, clauses, pos, neg, all, t))
        {
            assert(false);
        }

        for (j = 0; tests[i].clauses[j].clause != NULL; ++j)
        {
            const clause_test_spec& spec = tests[i].clauses[j];
            assert(j < clauses.size());
            ss.str("");
            ss << clauses[j];
            string learned = ss.str();
            if (learned == spec.clause)
            {
                clause_success_rate(clauses[j], pos, neg, all, success, fp, fn);
                if (!close(success, spec.success_rate))
                {
                    std::cout << "Expected success rate " << spec.success_rate << ", got " << success << std::endl;
                    result = false;
                }
            }
            else
            {
                std::cout << "Test " << j << " expected " << spec.clause << ", got " << learned << std::endl;
                result = false;
            }
        }
    }
    return result;
}

bool close(double a, double b)
{
    return fabs(a - b) < EQUAL_MARGIN;
}

int test_clause_vec(const vector<clause>& clauses, const relation_table& rels, var_domains& d)
{
    for (int i = 0, iend = clauses.size(); i < iend; ++i)
    {
        if (test_clause(clauses[i], rels, d))
        {
            return i;
        }
    }
    return -1;
}

void test_clauses(clause_vec& clauses, relation& pos, relation& neg, relation_table& all_rels)
{
    relation::const_iterator i, iend;
    int matched, false_negs, false_pos;
    var_domains d;

    std::cout << "False negatives" << std::endl;
    for (false_negs = 0, i = pos.begin(), iend = pos.end(); i != iend; ++i)
    {
        d.clear();
        for (int j = 0, jend = i->size(); j < jend; ++j)
        {
            d[j].insert((*i)[j]);
        }
        matched = test_clause_vec(clauses, all_rels, d);
        if (matched < 0)
        {
            join(cout, *i, ", ") << std::endl;
            ++false_negs;
        }
    }

    std::cout << "False positives" << std::endl;
    for (false_pos = 0, i = neg.begin(), iend = neg.end(); i != iend; ++i)
    {
        d.clear();
        for (int j = 0, jend = i->size(); j < jend; ++j)
        {
            d[j].insert((*i)[j]);
        }
        matched = test_clause_vec(clauses, all_rels, d);
        if (matched >= 0)
        {
            std::cout << "(" << matched << ") ";
            join(cout, *i, ", ") << " | ";
            var_domains::const_iterator di, diend;
            for (di = d.begin(), diend = d.end(); di != diend; ++di)
            {
                assert(di->second.size() == 1);
                int val = *di->second.begin();
                std::cout << di->first << " <- " << val << " ";
            }
            std::cout << std::endl;
            ++false_pos;
        }
    }

    int total = pos.size() + neg.size();
    int correct = total - false_negs - false_pos;
    double success_rate = correct / static_cast<double>(total);
    std::cout << std::endl
         << success_rate << " success, " << correct << " correct, " << false_negs + false_pos << " incorrect, "
         << false_negs << " false negs, " << false_pos << " false pos" << std::endl;
}

literal PL(const string& s)
{
    bool negate;
    string name;
    tuple args;
    int open, close;
    vector<string> arg_str;

    open = s.find_first_of("(");
    close = s.find_first_of(")");
    assert(open != string::npos && close != string::npos && open < close);
    if (s[0] == '~')
    {
        negate = true;
        name = s.substr(1, open - 1);
    }
    else
    {
        negate = false;
        name = s.substr(0, open);
    }

    split(s.substr(open + 1, close - open - 1), ", ", arg_str);
    assert(arg_str.size() > 0);
    args.resize(arg_str.size());
    for (int i = 0, iend = arg_str.size(); i < iend; ++i)
    {
        if (!parse_int(arg_str[i], args[i]))
        {
            assert(false);
        }
    }
    return literal(name, args, negate);
}

clause PC(const string& s)
{
    clause c;
    vector<string> lit_strs;
    split(s, " &", lit_strs);
    for (int i = 0, iend = lit_strs.size(); i < iend; ++i)
    {
        c.push_back(PL(lit_strs[i]));
    }
    return c;
}

bool test_fix_variables()
{
    clause c1 = PC("A(0,1,2) & ~B(0,3)");
    clause t1 = PC("A(0,1,2) & ~B(0,-1)");
    fix_variables(3, c1);
    if (c1 != t1)
    {
        std::cout << "Expected " << t1 << ", got " << c1 << std::endl;
        return false;
    }

    clause c2 = PC("A(0,1) & B(0,3)");
    clause t2 = PC("A(0,1) & B(0,2)");
    fix_variables(1, c2);
    if (c2 != t2)
    {
        std::cout << "Expected " << t2 << ", got " << c2 << std::endl;
        return false;
    }
    return true;
}
