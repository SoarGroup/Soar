#include <string>
#include "model.h"
#include "soar_interface.h"
#include "svs.h"
#include "common.h"
#include "mat.h"

using namespace std;

const int CUBESIZE = 30;
const int HALFCUBE = CUBESIZE / 2;

class targets_model : public model {
public:
	targets_model(const string &name)
	: model(name, "targets", false)
	{}
	
	bool predict(int target, const scene_sig &sig, const relation_table &rels, const rvec &x, rvec &y) {
		y[0] = x[0] + x[4];
		y[1] = x[1] + x[5];
		y[2] = x[2];
		y[3] = x[3];
		
		bbox cur(vec3(y[0] - HALFCUBE, y[1] - HALFCUBE, -HALFCUBE),
		         vec3(y[0] + HALFCUBE, y[1] + HALFCUBE,  HALFCUBE));

		bbox box(vec3(x[2] - HALFCUBE, x[3] - HALFCUBE, -HALFCUBE),
		         vec3(x[2] + HALFCUBE, x[3] + HALFCUBE,  HALFCUBE));
		
		vec3 p0(x[2], x[3], 0);
		if (cur.intersects(box)) {
			double bestdist = -1.0;
			vec3 bestpos;
			for (int i = 0; i < 2; ++i) {
				vec3 p1(p0);
				vec3 p2(p0);
				p1[i] = y[i] - CUBESIZE;
				p2[i] = y[i] + CUBESIZE;
				double d1 = (p1 - p2).norm();
				double d2 = (p2 - p2).norm();
				
				if (bestdist < 0 || d1 < bestdist) {
					bestpos = p1;
					bestdist = d1;
				}
				if (bestdist < 0 || d2 < bestdist) {
					bestpos = p2;
					bestdist = d2;
				}
			}
			y[2] = bestpos[0];
			y[3] = bestpos[1];
		}
		
		return true;
	}
	
	int get_input_size() const {
		return 6;
	}
	
	int get_output_size() const {
		return 4;
	}
};

model *_make_targets_model_(soar_interface *si, Symbol *root, svs_state *state, const string &name) {
	return new targets_model(name);
}
