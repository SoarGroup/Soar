#include "cliproxy.h"
#include "common.h"

using namespace std;

bool partition(const string &s, string &first, string &rest) {
	size_t i = s.find('.');
	if (i == string::npos) {
		first = s;
		rest.clear();
		return true;
	}
	first = s.substr(0, i);
	rest = s.substr(i + 1);
	return false;
}

cliproxy::~cliproxy() {}

void cliproxy::proxy_use(const string &path, const vector<std::string> &args, std::ostream &os) {
	map<string, cliproxy*> c;
	map<string, cliproxy*>::const_iterator i, iend;
	
	proxy_get_children(c);
	if (path.empty() || path == ".") {
		proxy_use_sub(args, os);
		if (!c.empty()) {
			os << endl << "children:";
			for (i = c.begin(), iend = c.end(); i != iend; ++i) {
				os << " " << i->first;
			}
			os << endl;
		}
	} else {
		string child, rest;
		partition(path, child, rest);
		if (has(c, child)) {
			c[child]->proxy_use(rest, args, os);
		} else {
			os << "path not found" << endl;
		}
	}
	for (i = c.begin(), iend = c.end(); i != iend; ++i) {
		if (i->second->temporary()) {
			delete i->second;
		}
	}
}

int_proxy::int_proxy(int *p) : p(p) {}

void int_proxy::proxy_use_sub(const vector<string> &args, ostream &os) {
	if (args.empty()) {
		os << *p << endl;
		return;
	}
	if (!parse_int(args[0], *p)) {
		os << "invalid integer" << endl;
	}
}

bool_proxy::bool_proxy(bool *p) : p(p) {}

void bool_proxy::proxy_use_sub(const vector<string> &args, ostream &os) {
	if (args.empty()) {
		os << (*p ? "true" : "false") << endl;
		return;
	}
	if (args[0] == "true" || args[0] == "on" || args[0] == "1") {
		*p = true;
	} else if (args[0] == "false" || args[0] == "off" || args[0] == "0") {
		*p = false;
	} else {
		os << "invalid boolean" << endl;
	}
}

void proxy_group::add(const std::string &name, cliproxy *p) {
	children[name] = p;
}

void proxy_group::proxy_get_children(std::map<std::string, cliproxy*> &c) {
	c = children;
}
