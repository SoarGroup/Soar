#include "AttributeSet.h"

void AttributeSet::add(string name, int value) {
  intAttribs.push_back(IntAttribType(name, value));
}

void AttributeSet::add(string name, float value) {
  floatAttribs.push_back(FloatAttribType(name, value));
}

void AttributeSet::add(string name, string value) {
  stringAttribs.push_back(StringAttribType(name, value));
}

void AttributeSet::clear() {
  intAttribs.clear();
  floatAttribs.clear();
  stringAttribs.clear();
}
