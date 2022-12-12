#include <iostream>
#include <cassert>
#include "foil.h"
#include "common.h"

int main(int argc, char* argv[])
{
    relation_table rels;
    clause c;
    std::tuple args;
    var_domains d;

    rels["ramp"] = relation(2);
    rels["ramp"].add(0, 100);
    rels["ramp"].add(1, 100);
    rels["floor"] = relation(2);
    rels["floor"].add(0, 200);
    rels["floor"].add(1, 200);

    args.push_back(0);
    args.push_back(-1);
    c.push_back(literal("ramp", args, true));
    c.push_back(literal("floor", args, true));

    d[0].insert(0);

    std::cout << test_clause(c, rels, d) << std::endl;
    return 0;
}
