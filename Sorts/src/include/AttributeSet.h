#ifndef AttributeSet_H
#define AttributeSet_H

#include <iostream>
#include <list>
#include <utility>

using namespace std;

typedef pair<string, int>    IntAttribType;
typedef pair<string, float>  FloatAttribType;
typedef pair<string, string> StringAttribType;

class AttributeSet {
public:
  AttributeSet() { }

  void add(string name, int value);
  void add(string name, float value);
  void add(string name, string value);

  const list<IntAttribType>& getIntAttributes() {
    return intAttribs;
  }

  const list<FloatAttribType>& getFloatAttributes() {
    return floatAttribs;
  }

  const list<StringAttribType>& getStringAttributes() {
    return stringAttribs;
  }

  void clear();

private:
  list<IntAttribType>    intAttribs;
  list<FloatAttribType>  floatAttribs;
  list<StringAttribType> stringAttribs;
};

#endif
