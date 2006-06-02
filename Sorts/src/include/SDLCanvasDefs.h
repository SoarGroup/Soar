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
};

#endif
