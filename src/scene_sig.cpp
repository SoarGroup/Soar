#include <sstream>
#include "scene_sig.h"
#include "serialize.h"
#include "params.h"

using namespace std;

void scene_sig::entry::serialize(ostream &os) const {
	assert(name.find(' ') == string::npos && type.find(' ') == string::npos);
	os << id << " " << name << " " << type;
	for (int i = 0, iend = props.size(); i < iend; ++i) {
		assert(props[i].find(' ') == string::npos);
		os << " " << props[i];
	}
	os << endl;
}

void scene_sig::entry::unserialize(istream &is) {
	string line, prop;
	stringstream ss;
	
	if (!getline(is, line)) {
		assert(false);
	}
	ss.str(line);
	ss >> id >> name >> type;
	
	props.clear();
	while (ss >> prop) {
		props.push_back(prop);
	}
}

void scene_sig::serialize(ostream &os) const {
	for (int i = 0, iend = s.size(); i < iend; ++i) {
		s[i].serialize(os);
	}
	os << endl;  // blank line indicates end
}

void scene_sig::unserialize(istream &is) {
	int start = 0;
	string line;
	stringstream ss;
	entry e;
	
	s.clear();
	while (getline(is, line)) {
		if (line.empty()) {
			break;
		}
		
		ss.str(line);
		e.unserialize(ss);
		e.start = start;
		start += e.props.size() + NUM_NATIVE_PROPS;
		s.push_back(e);
	}
}

void scene_sig::add(const scene_sig::entry &e) {
	int curr_dim = dim();
	s.push_back(e);
	s.back().start = curr_dim;
}

int scene_sig::dim() const {
	int d = 0;
	for (int i = 0; i < s.size(); ++i) {
		d += s[i].props.size();
	}
	return d;
}

bool scene_sig::get_dim(const string &obj, const string &prop, int &obj_ind, int &prop_ind) const {
	for (int i = 0; i < s.size(); ++i) {
		const entry &e = s[i];
		if (e.name == obj) {
			for (int j = 0; j < e.props.size(); ++j) {
				if (e.props[j] == prop) {
					obj_ind = i;
					prop_ind = e.start + j;
					return true;
				}
			}
			return false;
		}
	}
	return false;
}

bool scene_sig::similar(const scene_sig &sig) const {
	if (s.size() != sig.s.size()) {
		return false;
	}
	for (int i = 0; i < s.size(); ++i) {
		if (!s[i].similar(sig.s[i])) {
			return false;
		}
	}
	return true;
}

int scene_sig::find_id(int id) const {
	for (int i = 0; i < s.size(); ++i) {
		if (s[i].id == id) {
			return i;
		}
	}
	return -1;
}

