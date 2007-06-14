#include <vector>
#include <map>
#include <iostream>
#include <iterator>
#include <algorithm>
#include <boost/bind.hpp>

using namespace std;

int main() {
  vector<int> a;
  a.push_back(2);
  a.push_back(3);
  a.push_back(7);
  a.push_back(10);
  a.push_back(20);

  map<int, vector<int> > b;
  b[5] = a;
  b[10] = a;
  a.erase(remove(ALL(a), 3), a.end());
  copy(ALL(a), ostream_iterator<int>(cout, " "));
  cout << endl;
  copy(ALL(b[5]), ostream_iterator<int>(cout, " "));
  cout << endl;
  return 0;
}
