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
#ifndef SDLCanvasDefs_h
#define SDLCanvasDefs_h

struct SDLCanvasPoint {
  Uint16 x;
  Uint16 y;
  SDLCanvasPoint(Uint16 _x, Uint16 _y) {
    x = _x;
    y = _y;
  }
};

struct SDLWorldPoint {
  double x;
  double y;
  SDLWorldPoint(double _x, double _y) {
    x = _x; 
    y = _y;
  }
};

struct SDLColor {
  Uint8 r;
  Uint8 g;
  Uint8 b;
  Uint8 a;
  SDLColor() {
    r = 255; g = 255; b = 255; a = 255;
  }
  SDLColor(Uint8 _r, Uint8 _g, Uint8 _b, Uint8 _a) {
    r = _r; g = _g; b = _b; a = _a;
  }
  SDLColor(Uint8 _r, Uint8 _g, Uint8 _b) {
    r = _r; g = _g; b = _b; a = 255;
  }

  void set(Uint8 _r, Uint8 _g, Uint8 _b) {
    r = _r; g = _g; b = _b; a = 255;
  }

  void set(Uint8 _r, Uint8 _g, Uint8 _b, Uint8 _a) {
    r = _r; g = _g; b = _b; a = _a;
  }
};

#endif
