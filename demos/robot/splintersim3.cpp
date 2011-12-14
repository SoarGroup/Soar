#include <stdlib.h>
#include <iostream>
#include <sstream>
#include <vector>
#include "common.h"
#include "ipcsocket.h"
#include "splinterenv.h"

using namespace std;

void test() {
	splinterenv env1, env2;
	vector<float> traj;
	int i;
	for (i = 0; i < 100; ++i) {
		traj.push_back((rand() / ((float) RAND_MAX) - 0.5) * 2);
		traj.push_back((rand() / ((float) RAND_MAX) - 0.5) * 2);
	}
	
	floatvec x(env2.get_input_size()), u(2), y(env2.get_output_size()), x1;
	env1.get_flat_state(x1);
	env2.get_flat_state(y);
	for (i = 0; i < 200; i += 2) {
		env1.step(traj[i], traj[i+1]);
		u[0] = traj[i];
		u[1] = traj[i+1];
		x.graft(0, u);
		x.graft(u.size(), y);
		if (!env2.predict(x, y)) {
			assert(false);
		}
		env1.get_flat_state(x1);
		/*
		cout << "A " << x1 << endl;
		cout << "B " << y << endl << "C ";
		for (int j = 0; j < y.size(); ++j) {
			cout << y[j] - x1[j] << " ";
		}
		*/
		cout << y.dist(x1) << endl;
	}
}

void run() {
	ipcsocket sock('c', "env", false, true);
	splinterenv env;
	
	while (true) {
		stringstream sgel_ss;
		string recvmsg;
		vector<string> lines;
		vector<string>::iterator i;
		float lvolts = 0.0, rvolts = 0.0;
		
		env.get_disp(cout);
		env.get_sgel(sgel_ss);
		sock.send(sgel_ss.str());
		
		if (!sock.receive(recvmsg)) {
			cerr << "receive error" << endl;
			exit(1);
		}
		
		split(recvmsg, "\n", lines);
		for (i = lines.begin(); i != lines.end(); ++i) {
			stringstream parse(*i);
			string name;
			float val;
			
			if (!(parse >> name) || !(parse >> val)) {
				continue;
			}
			
			if (name == "left") {
				lvolts = val;
			} else if (name == "right") {
				rvolts = val;
			}
		}
		
		env.step(lvolts, rvolts);
	}
}

int main(int argc, char *argv[]) {
	run();
	return 0;
}
