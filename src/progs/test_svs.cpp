#include <cstring>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <cstdlib>
#include <sml_Client.h>

using namespace std;
using namespace sml;

const char *failure_rule =
"sp {failure "
":default "
"   (state <s> ^superstate nil ^svs.command.extract <e> ^extract-info <i>) "
"   (<i> ^cmd <e> ^name <name> ^expected <posneg>) "
"   (<e> ^status success ^result <res>) "
"   (<res> ^{<negpos> <> <posneg>}.atom <a>) "
"--> "
"   (exec print |failure | <name> |, expected | <posneg> (crlf)) "
"   (exec count failure)}";

const char *success_rule =
"sp {success "
":default "
"   (state <s> ^superstate nil ^svs.command.extract <e> ^extract-info <i>) "
"   (<i> ^cmd <e> ^name <name> ^expected <posneg>) "
"   (<e> ^status success ^result <res>) "
"   (<res> ^<posneg>.atom <a>) "
"--> "
"   (exec print |success | <name> (crlf)) "
"   (exec count success)}";

const char *syntax_rule =
"sp {syntax "
":default "
"   (state <s> ^superstate nil ^svs.command.extract <e> ^extract-info <i>) "
"   (<i> ^cmd <e> ^name <name>) "
"   (<e> ^status {<stat> <> success}) "
"--> "
"   (exec print |syntax  | <name> |, | <stat> (crlf)) "
"   (exec count syntax)}";

void split(const string &s, const string &delim, vector<string> &fields) {
	int start, end = 0;
	while (end < s.size()) {
		start = s.find_first_not_of(delim, end);
		if (start == string::npos) {
			return;
		}
		end = s.find_first_of(delim, start);
		if (end == string::npos) {
			end = s.size();
		}
		fields.push_back(s.substr(start, end - start));
	}
}

bool parse_double(const string &s, double &v) {
	if (s.empty()) {
		return false;
	}
	
	char *end;
	v = strtod(s.c_str(), &end);
	if (*end != '\0') {
		return false;
	}
	return true;
}

bool parse_int(const string &s, int &v) {
	if (s.empty()) {
		return false;
	}
	
	char *end;
	v = strtol(s.c_str(), &end, 10);
	if (*end != '\0') {
		return false;
	}
	return true;
}

struct result_counts {
	int success;
	int failure;
	int syntax;
};

string rhs_count(smlRhsEventId id, void *data, Agent *agnt, char const *func, char const *arg) {
	result_counts *c = reinterpret_cast<result_counts*>(data);
	if (strcmp(arg, "success") == 0) {
		++c->success;
	} else if (strcmp(arg, "failure") == 0) {
		++c->failure;
	} else {
		++c->syntax;
	}
	return "";
}

string rhs_print(smlRhsEventId id, void *data, Agent *agnt, char const *func, char const *arg) {
	cout << arg;
	cout.flush();
	return "";
}

string strip(const string &s, const string &lc, const string &rc) {
	size_t b, e;
	b = s.find_first_not_of(lc);
	if (b == string::npos) {
		return "";
	}
	e = s.find_last_not_of(rc);
	return s.substr(b, e - b + 1);
}

string make_rule(const string &filter, const vector<string> &arg_names, const vector<string> &arg_vals, bool positive) {
	static int test_count = 0;
	stringstream ss, ss1;

	ss << filter << "(";
	for (int i = 0; i < arg_names.size(); ++i) {
		ss << arg_names[i] << ":" << arg_vals[i];
		if (i < arg_names.size() - 1) {
			ss << ",";
		}
	}
	ss << ")";
	string name = ss.str();

	ss.str("");
	ss << "sp {add-extract" << test_count++ << endl
	   << "(state <s> ^superstate nil ^svs.command <c>)" << endl
	   << "-->" << endl
	   << "(<s> ^extract-info <i>)"
	   << "(<i> ^cmd <e> ^name |" << name << "| ^expected " << (positive ? "positive" : "negative") << ")"
	   << "(<c> ^extract <e>)"
	   << "(<e> ^type " << filter;
	
	for (int i = 0; i < arg_names.size(); ++i) {
		ss << " ^" << arg_names[i] << " ";
		int iv;
		double dv;
		if (parse_int(arg_vals[i], iv)) {
			ss << iv;
		} else if (parse_double(arg_vals[i], dv)) {
			ss << dv;
		} else if (arg_vals[i].substr(0, 2) == "c:") {
			ss << arg_vals[i].substr(2); // constant string
		} else {
			ss << "<a" << i << ">";
			ss1 << "(<a" << i << "> ^type node ^name " << arg_vals[i] << ")" << endl;
		}
	}
	ss << ")" << endl << ss1.str() << "}";
	return ss.str();
}

void handle_print(smlPrintEventId id, void *d, Agent *a, char const *m) {
	cout << strip(m, "\n", "\n\t ") << endl;
}

void repl(Agent *a) {
	string line;
	int callback_id = a->RegisterForPrintEvent(smlEVENT_PRINT, handle_print, NULL);
	cout << "% ";
	cout.flush();
	while (getline(cin, line)) {
		string res = a->ExecuteCommandLine(line.c_str());
		res = strip(res, "\n\t ", "\n\t ");
		if (!res.empty()) {
			cout << res << endl;
		}
		cout << "% ";
		cout.flush();
	}
	a->UnregisterForPrintEvent(callback_id);
}

int main(int argc, char *argv[]) {
	int total = 0;
	bool needs_test = false;
	result_counts counts = {0, 0, 0};
	
	Kernel *k = Kernel::CreateKernelInCurrentThread();
	k->AddRhsFunction ("count", rhs_count, &counts);
	k->AddRhsFunction ("print", rhs_print, NULL);
	Agent *a = k->CreateAgent("arst");
	a->ExecuteCommandLine(failure_rule);
	a->ExecuteCommandLine(success_rule);
	a->ExecuteCommandLine(syntax_rule);
	a->ExecuteCommandLine("waitsnc -e");
	
	string line;
	istream *input;
	if (argc < 2) {
		input = &cin;
	} else {
		input = new ifstream(argv[1]);
	}
	while (getline(*input, line)) {
		line = line.substr(0, line.find_first_of('#'));
		line = strip(line, " \t", " \t");
		if (line.empty()) {
			continue;
		}

		
		if (line == "test") {
			a->ExecuteCommandLine("run 2");
			a->ExecuteCommandLine("excise -u");
			needs_test = false;
		} else if (line == "init") {
			a->ExecuteCommandLine("init");
			a->ExecuteCommandLine("excise -u");
			needs_test = false;
		} else if (line == "repl") {
			repl(a);
		} else {
			vector<string> fields;
			split(line, " \t", fields);
			if (fields[0] == "pos" || fields[0] == "neg") {
				if (fields.size() < 2 || fields.size () % 2 != 0) {
					cerr << "error in line '" << line << "'" << endl;
					return 1;
				}
				string filter = fields[1];
				vector<string> names, vals;
				for (int i = 2; i < fields.size(); i += 2) {
					names.push_back(fields[i]);
					vals.push_back(fields[i + 1]);
				}
				string rule = make_rule(filter, names, vals, fields[0] == "pos");
				a->ExecuteCommandLine(rule.c_str());
				++total;
			} else {
				a->SendSVSInput(line);
			}
			needs_test = true;
		}
	}
	if (needs_test) {
		a->ExecuteCommandLine("run 2");
	}
	
	cout << counts.success << " success, " 
	     << counts.failure << " failure, " 
	     << counts.syntax << " syntax, " 
	     << total << " total"<< endl;
	
	int sum = counts.success + counts.failure + counts.syntax;
	if (sum != total) {
		cout << "Warning: " << total - sum << " test rules did not fire" << endl;
	}
	
	return counts.failure + counts.syntax;
}

