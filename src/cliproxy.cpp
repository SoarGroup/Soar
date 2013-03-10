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

cliproxy::cliproxy() {}
cliproxy::~cliproxy() {}

void cliproxy::use(const vector<std::string> &args, std::ostream &os) {
	std::map<std::string, child>::const_iterator i, iend;
	for (i = children.begin(), iend = children.end(); i != iend; ++i) {
		os << i->first << endl;
	}
}

void cliproxy::add(const string &name, cliproxy *child, bool own) {
	string first, rest;
	bool last = partition(name, first, rest);
	cliproxy::child &c = children[first];
	if (last) {
		if (c.p && c.own) {
			delete c.p;
		}
		c.p = child;
		c.own = own;
	} else {
		if (!c.p) {
			c.p = new cliproxy;
			c.own = true;
		}
		c.p->add(rest, child);
	}
}

void cliproxy::add(const string &name, cliproxy *child) {
	add(name, child, true);
}

void cliproxy::add(const string &name, proxied *child) {
	add(name, child->get_proxy(), false);
}

void cliproxy::del(const string &name) {
	string first, rest;
	bool last = partition(name, first, rest);
	assert(has(children, first));
	if (last) {
		children.erase(first);
	} else {
		children[first].p->del(rest);
	}
}

void cliproxy::rename(const string &from, const string &to) {
	assert(has(children, from));
	children.erase(to);
	child &c = children[from];
	children[to] = c;
	c.own = false;
	children.erase(from);
}

cliproxy* cliproxy::find(const string &path) {
	vector<string> parts;
	split(path, ".", parts);
	return findp(0, parts);
}

cliproxy* cliproxy::findp(int i, const vector<string> &path) {
	cliproxy *c;
	if (i >= path.size()) {
		return this;
	}
	if (!has(children, path[i])) {
		return NULL;
	}
	return children[path[i]].p->findp(i+1, path);
}

int_proxy::int_proxy(int *p) : p(p) {}

void int_proxy::use(const vector<string> &args, ostream &os) {
	if (args.empty()) {
		os << *p << endl;
		return;
	}
	if (!parse_int(args[0], *p)) {
		os << "invalid integer" << endl;
	}
}

bool_proxy::bool_proxy(bool *p) : p(p) {}

void bool_proxy::use(const vector<string> &args, ostream &os) {
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
