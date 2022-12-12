#include <iostream>
#include <cassert>
#include <algorithm>
#include <iterator>
#include "relation.h"

set<int> a;
interval_set b;

void same()
{
    set<int>::const_iterator ai;
    interval_set::const_iterator bi;

    for (ai = a.begin(), bi = b.begin(); ai != a.end() && bi != b.end(); ++ai, ++bi)
    {
        assert(*ai == *bi);
        assert(b.contains(*ai));
    }
    assert(ai == a.end() && bi == b.end());
    assert(a.size() == b.size());
}

void intervals(int n)
{
    assert(b.num_intervals() == n);
}

void erase(int x)
{
    a.erase(x);
    b.erase(x);
    same();
}

void insert(int x)
{
    a.insert(x);
    b.insert(x);
    same();
}

set<int> make_run(int i, int j)
{
    set<int> s;
    for (; i <= j; ++i)
    {
        s.insert(i);
    }
    return s;
}

set<int> make_random(int i, int j, int k)
{
    assert(j - i + 1 >= k);
    vector<int> v;
    set<int> s;

    sample(k, i, j, v);
    for (i = 0; i < v.size(); ++i)
    {
        s.insert(v[i]);
    }
    return s;
}

void unify(const set<int>& s)
{
    set<int>::const_iterator i;
    interval_set b1(s), old = b;

    for (i = s.begin(); i != s.end(); ++i)
    {
        a.insert(*i);
    }
    b.unify(b1);

    same();
}

void intersect(const set<int>& s)
{
    set<int>::const_iterator i;
    interval_set b1(s);

    for (i = a.begin(); i != a.end();)
    {
        if (s.find(*i) == s.end())
        {
            a.erase(i++);
        }
        else
        {
            i++;
        }
    }
    b.intersect(b1);

    same();
}

void subtract(const set<int>& s)
{
    set<int>::const_iterator i;
    interval_set b1(s);

    for (i = a.begin(); i != a.end();)
    {
        if (s.find(*i) != s.end())
        {
            a.erase(i++);
        }
        else
        {
            i++;
        }
    }
    b.subtract(b1);

    same();
}

set<int> combine(const set<int>& s1, const set<int>& s2)
{
    set<int> s;
    set<int>::const_iterator i;
    for (i = s1.begin(); i != s1.end(); ++i)
    {
        s.insert(*i);
    }
    for (i = s2.begin(); i != s2.end(); ++i)
    {
        s.insert(*i);
    }
    return s;
}

int main(int argc, char* argv[])
{
    interval_set s;
    for (int i = 0; i < 5; ++i)
    {
        s.insert(i);
    }
    vector<int> v(s.begin(), s.end());

    for (int i = 0; i < 10; ++i)
    {
        insert(i);
    }
    intervals(1);

    // edge deletes
    erase(0);
    erase(9);
    intervals(1);

    // interval split
    erase(5);
    intervals(2);

    for (int i = 0; i < 5; ++i)
    {
        erase(i);
    }
    intervals(1);

    // new interval
    insert(1);
    intervals(2);
    insert(20);
    intervals(3);
    insert(21);
    intervals(3);

    // edge insertions
    insert(5);
    insert(9);
    intervals(3);

    // interval merge
    insert(11);
    intervals(4);
    insert(10);
    intervals(3);
    insert(-1);
    intervals(4);
    insert(0);
    intervals(3);

    // multiple insert
    insert(0);
    insert(0);

    // set assign
    b = a;
    same();

    a = make_run(0, 5);
    b = a;

    // contained union
    unify(make_run(0, 5));

    // edge unions
    unify(make_run(-5, -1));
    unify(make_run(6, 10));

    // disjoint unions
    unify(make_run(-10, -7));
    unify(make_run(12, 15));

    a = make_run(0, 100);
    b = a;

    // edge intersections
    intersect(make_run(0, 100));
    intersect(make_run(0, 99));
    intersect(make_run(1, 99));

    intersect(combine(make_run(0, 30), make_run(60, 100)));
    intersect(make_run(0, 100));


    a = make_run(0, 10);
    b = a;

    subtract(make_run(0, 2));
    subtract(make_run(8, 10));
    subtract(make_run(0, 3));
    subtract(make_run(7, 10));
    subtract(make_run(3, 7));

    for (int i = 0; i < 100; ++i)
    {
        unify(make_random(-100, 100, 50));
        intersect(make_random(-100, 100, 50));
        subtract(make_random(-100, 100, 50));
    }

    return 0;
}
