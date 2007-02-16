#include <stdlib.h>
#include <iostream>
#include <list>
#include <map>
#include <utility>
#include <fstream>
#include <assert.h>
#include <pcrecpp.h>

#define MAZE_SIZE 9

using namespace std;
using pcrecpp::RE;

enum CardinalDir { Nowhere = 0, North, South, East, West };

enum MummyType { Horizontal, Vertical };

enum SearchResult { Eaten, DepthExceeded, Mixed, Success };

struct Coord {
  unsigned char x;
  unsigned char y;

  Coord(int _x, int _y) { x = _x; y = _y; }

  Coord(const Coord& other) { x = other.x; y = other.y; }

  Coord translate(CardinalDir m) const {
    switch (m) {
      case North:
        return Coord(x, y+1);
      case South:
        return Coord(x, y-1);
      case East:
        return Coord(x+1, y);
      case West:
        return Coord(x-1, y);
      case Nowhere:
        return Coord(x, y);
      default:
        assert(false);
    }
  }

  bool operator==(const Coord& other) const {
    return x == other.x and y == other.y;
  }

  bool operator!=(const Coord& other) const {
    return not operator==(other);
  }

	bool operator<(const Coord& other) const {
		if (x < other.x or (x == other.x and y < other.y)) {
			return true;
		}
		return false;
	}
};

typedef pair<Coord, Coord> TwoCoords;

struct Cell {
  bool walls[5];
};

class Board {
public:
  Board(int w, int h) : width(w), height(h) {
    cells = new Cell*[width];
    for (int i = 0; i < width; ++i) {
      cells[i] = new Cell[height];
      memset(cells[i], 0, sizeof(Cell) * height);
    }
  }

  void set_wall(int x, int y, CardinalDir dir) {
    assert(0 <= x && x < width);
    assert(0 <= y && y < height);
    if (dir == North) {
      cells[x][y].walls[North] = true;
      if (y < height - 1) {
        cells[x][y+1].walls[South] = true;
      }
    }
    if (dir == East) {
      cells[x][y].walls[East] = true;
      if (x < width - 1) {
        cells[x+1][y].walls[West] = true;
      }
    }
  }

  Coord newlocation1(Coord c, CardinalDir m) const {
    if (!cells[c.x][c.y].walls[m]) {
      return c.translate(m);
    }
    return c;
  }

private:
  int width;
  int height;
  Cell** cells;
};


struct Dir {
  CardinalDir h;
  CardinalDir v;
};

int ManhattanDist(Coord c1, Coord c2) {
  return abs(c1.x - c2.x) + abs(c1.y - c2.y);
}

Dir calc_facing(Coord mc, Coord ec) {
  Dir face;
  if (mc.x < ec.x) {
    face.h = East;
  }
  else if (mc.x > ec.x) {
    face.h = West;
  }
  else {
    face.h = Nowhere;
  }
  if (mc.y < ec.y) {
    face.v = North;
  }
  else if (mc.y > ec.y) {
    face.v = South;
  }
  else {
    face.v = Nowhere;
  }
  return face;
}


CardinalDir pick_one(Coord c, Dir face, MummyType t, const Board& b) {
  if ((face.h == Nowhere) && (face.v == Nowhere)) {
    return Nowhere;
  }
  if (face.h == Nowhere) {
    return face.v;
  }
  if (face.v == Nowhere) {
    return face.h;
  }
  if (t == Horizontal) {
    // if can move horizontal, then move horizontal
    if (b.newlocation1(c, face.h) != c) {
      return face.h;
    }
    return face.v;
  }
  else {
    // if can move vertical, then move vertical
    if (b.newlocation1(c, face.v) != c) {
      return face.v;
    }
    return face.h;
  }
}

Coord mummy_move(Coord ec, Coord mc, MummyType t, const Board& b) {
  Coord mc1 = b.newlocation1(mc, pick_one(mc, calc_facing(mc, ec), t, b));
  return b.newlocation1(mc1, pick_one(mc1, calc_facing(mc1, ec), t, b));
}

SearchResult dfs
( Coord              ec, 
  Coord              mc, 
  Coord              exit,
  MummyType          t,
  const Board&       b,
  int                depth,
  list<CardinalDir>& moves,
	map<TwoCoords, int>&    explored)
{
	if (explored.find(TwoCoords(ec, mc)) != explored.end() and 
			explored[TwoCoords(ec, mc)] >= depth) 
	{
		// already visited this state with more moves left,
		// this branch is doomed
		return DepthExceeded;
	}
	if (explored.find(TwoCoords(ec, mc)) != explored.end() and 
			explored[TwoCoords(ec, mc)] == -1)
  {
    // already visited this state, and all branches result
    // in being eaten
    return Eaten;
  }
  
  // assume depth exceeded first
	explored[TwoCoords(ec, mc)] = depth;

  if ((ec == exit) && (mc != ec)) {
    return Success;
  }
  if (depth < ManhattanDist(ec, exit)) {
    return DepthExceeded;
  }
  if (ec == mc) {
	  explored[TwoCoords(ec, mc)] = -1;
    return Eaten;
  }

  bool eatenOutcome = false;
  bool depthOutcome = false;
  for (int i = 0; i < 5; ++i) {
    Coord nec = b.newlocation1(ec, (CardinalDir) i);
    Coord nmc = mummy_move(nec, mc, t, b);
    SearchResult res = dfs(nec, nmc, exit, t, b, depth - 1, moves, explored);
    switch (res) {
      case Eaten:
        eatenOutcome = true;
        break;
      case DepthExceeded:
        depthOutcome = true;
        break;
      case Mixed:
        eatenOutcome = true;
        depthOutcome = true;
        break;
      case Success:
        moves.push_front((CardinalDir) i);
        return Success;
    }
  }
  if (eatenOutcome and depthOutcome) {
    return Mixed;
  }
  else if (eatenOutcome) {
    explored[TwoCoords(ec, mc)] = -1;
    return Eaten;
  }
  else {
    return DepthExceeded;
  }
}

int main(int argc, char** argv) {
  if (argc < 10) {
    cout << "Incorrect number of parameters (" << argc << ")" << endl;
    return 1;
  }

  int explorer_x, explorer_y, mummy_x, mummy_y, exit_x, exit_y, max_depth;
  string kif;
  MummyType type;

  explorer_x = atoi(argv[1]);
  explorer_y = atoi(argv[2]);
  mummy_x = atoi(argv[3]);
  mummy_y = atoi(argv[4]);
  exit_x = atoi(argv[5]);
  exit_y = atoi(argv[6]);

  if (strcmp(argv[7], "h") == 0) {
    type = Horizontal;
  }
  else {
    type = Vertical;
  }

  kif = argv[8];
  max_depth = atoi(argv[9]);

  Coord explorer(explorer_x, explorer_y);
  Coord mummy(mummy_x, mummy_y);
  Coord exit(exit_x, exit_y);

  Board board(MAZE_SIZE, MAZE_SIZE);
  RE wall_re("\\(init\\s+\\(wall\\s+(\\d+)\\s+(\\d+)\\s+(\\w+)\\)\\)");
  ifstream f(kif.c_str(), ios::in);
  
  while (!f.eof()) {
    string line;
    int x, y;
    string dir;
    getline(f, line);
    if (wall_re.PartialMatch(line, &x, &y, &dir)) {
      if (dir == "north") {
        board.set_wall(x, y, North);
      }
      else {
        board.set_wall(x, y, East);
      }
    }
  }

  list<CardinalDir> moves;
	map<TwoCoords, int> explored;
  SearchResult res = DepthExceeded;
  int d = 0;
  while (d <= max_depth and (res == Mixed or res == DepthExceeded)) {
    d++;
    res = dfs(explorer, mummy, exit, type, board, d, moves, explored);
  }
  switch (res) {
    case Success:
      for(list<CardinalDir>::iterator 
          i = moves.begin(); 
          i != moves.end(); 
          ++i) 
      {
        switch (*i) {
          case Nowhere:
            cout << "wait ";
            break;
          case North:
            cout << "north ";
            break;
          case South:
            cout << "south ";
            break;
          case East:
            cout << "east ";
            break;
          case West:
            cout << "west ";
            break;
        }
      }
      cout << endl;
      break;
    case Eaten:
      cout << "eaten" << endl;
      break;
    case Mixed:
    case DepthExceeded:
      cout << "depth" << endl;
      break;
  }

  return 0;
}
