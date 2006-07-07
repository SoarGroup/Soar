/*
    This file is part of Sorts, an interface between Soar and ORTS.
    (c) 2006 James Irizarry, Sam Wintermute, and Joseph Xu

    Sorts is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    Sorts is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Sorts; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA    
*/
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
