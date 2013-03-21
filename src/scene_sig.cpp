#include <sstream>
#include "scene_sig.h"
#include "serialize.h"
#include "params.h"
#include "common.h"

using namespace std;

void scene_sig::entry::serialize(ostream &os) const {
	serializer sr(os);
	sr << id << name << type << props.size();
	for (int i = 0, iend = props.size(); i < iend; ++i) {
		sr << props[i];
	}
}

void scene_sig::entry::unserialize(istream &is) {
	string line, prop;
	int nprops;
	
	unserializer unsr(is);
	unsr >> id >> name >> type >> nprops;
	
	props.clear();
	for (int i = 0; i < nprops; ++i) {
		unsr >> prop;
		props.push_back(prop);
	}
}

void scene_sig::serialize(ostream &os) const {
	serializer sr(os);
	sr << "SCENE_SIG" << s.size() << '\n';
	for (int i = 0, iend = s.size(); i < iend; ++i) {
		sr << s[i];
	}
}

void scene_sig::unserialize(istream &is) {
	int start = 0, nentries;
	string line, label;
	entry e;
	unserializer unsr(is);
	
	s.clear();
	unsr >> label >> nentries;
	assert(label == "SCENE_SIG");
	
	for (int i = 0; i < nentries; ++i) {
		unsr >> e;
		e.start = start;
		start += e.props.size();
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

