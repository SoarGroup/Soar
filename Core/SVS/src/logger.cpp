#include <iostream>
#include "soar_interface.h"
#include "logger.h"

using namespace std;

struct log_type_info {
	const char *name;
	bool active;
};

log_type_info logger_info[NUM_LOG_TYPES] = {
	{ "ERROR", true  },
	{ "CTRL",  false },
	{ "EM",    false },
	{ "SGEL",  false },
	{ "FOIL",  false },
};

logger::logger()
: bool_proxy(&active, "Print messages from this log."), si(NULL), active(false)
{}

void logger::init(soar_interface *si, const string &prefix, bool active) {
	this->si = si;
	this->prefix = prefix;
	this->active = active;
	ss.str("");
	ss << "SVS " << prefix << ": ";
}

logger& logger::operator<<(ostream& (*f)(ostream&)) {
	string s;
	
	f(ss);
	s = ss.str();
	if (active && si && s[s.size()-1] == '\n') {
		si->print(s);
	}
	ss.str("");
	ss << "SVS " << prefix << ": ";
	return *this;
}

void logger::proxy_use_sub(const vector<string> &args, ostream &os) {
	bool_proxy p(&active, "");
	p.proxy_use_sub(args, os);
}

logger_set::logger_set(soar_interface *si) {
	for (int i = 0; i < NUM_LOG_TYPES; ++i) {
		loggers[i].init(si, logger_info[i].name, logger_info[i].active);
	}
}

void logger_set::proxy_get_children(map<string, cliproxy*> &c) {
	for (int i = 0; i < NUM_LOG_TYPES; ++i) {
		c[logger_info[i].name] = &loggers[i];
	}
}
