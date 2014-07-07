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

int var_count = 0;
int test_count = 0;

const char *rules[] = {
"sp {failure-value "
":default "
"   (state <s> ^superstate nil ^svs.command.extract <e> ^extract-info <i>) "
"   (<i> ^cmd <e> ^count <c> ^expected <val>) "
"   (<e> ^status success ^result <res>) "
" -{(<res> ^record <r>) "
"   (<r> ^value <val>)} "
"--> "
"   (exec count <c> | failure|)}",

"sp {failure-param "
":default "
"   (state <s> ^superstate nil ^svs.command.extract <e> ^extract-info <i>) "
"   (<i> ^cmd <e> ^count <c> ^expected-params.<name> <val>) "
"   (<e> ^status success ^result <res>) "
" -{(<res> ^record <r>) "
"   (<r> ^params <p>) "
"   (<p> ^<name> <val>)} "
"--> "
"   (exec count <c> | failure|)}",

"sp {executed "
":default "
"   (state <s> ^superstate nil ^extract-info <i>) "
"   (<i> ^cmd.status success ^count <c>) "
"--> "
"   (exec count <c> | executed|)}",

"sp {syntax "
":default "
"   (state <s> ^superstate nil ^svs.command.extract <e> ^extract-info <i>) "
"   (<i> ^cmd <e> ^count <c>) "
"   (<e> ^status {<stat> <> success}) "
"--> "
"   (exec count <c> | syntax|)}",

NULL};

size_t get_close_paren(const string &s, size_t start) {
	int depth = 1;
	size_t i = start;
	while (i != string::npos) {
		i = s.find_first_of("()", i + 1);
		if (i == string::npos) {
			return string::npos;
		} else if (s[i] == '(') {
			++depth;
		} else if (s[i] == ')') {
			if (--depth == 0) {
				return ((i + 1 >= s.size()) ? string::npos : (i + 1));
			}
		}
	}
	return string::npos;
}

void split(const string &s, vector<string> &fields) {
	const char *delims = " \t";
	int start, end = 0;
	while (end < s.size()) {
		start = s.find_first_not_of(delims, end);
		if (start == string::npos) {
			return;
		}
		if (s[start] == '(') {
			end = get_close_paren(s, start);
		} else {
			end = s.find_first_of(delims, start);
		}
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

struct test_info_struct {
	int executed;
	int failure;
	int syntax;
	map<int, string> tests;
	
	test_info_struct() {
		executed = 0;
		failure = 0;
		syntax = 0;
	}
};

test_info_struct test_info;

string rhs_count(smlRhsEventId id, void *data, Agent *thisAgent, char const *func, char const *arg) {
	vector<string> fields;
	int count;
	
	split(arg, fields);
	if (!parse_int(fields[0], count)) {
		assert(false);
	}
	
	if (fields[1] == "executed") {
		++test_info.executed;
	} else if (fields[1] == "failure") {
		++test_info.failure;
		cout << "failure " << test_info.tests[count] << endl;
	} else {
		++test_info.syntax;
		cout << "syntax  " << test_info.tests[count] << endl;
	}
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

bool parse_filter(const vector<string> &fields, stringstream &ss) {
	stringstream rest;
	
	if (fields.size() < 1 || (fields.size() - 1) % 2 != 0) {
		return false;
	}
	ss << "(<a" << var_count++ << "> ^type " << fields[0];
	
	for (int i = 1; i < fields.size(); i += 2) {
		string v = fields[i+1];
		ss << " ^" << fields[i] << " ";
		if (v[0] == '(') {
			ss << "<a" << var_count << "> ";
			//rest << "(";
			vector<string> subfields;
			split(v.substr(1, v.size() - 2), subfields);
			if (!parse_filter(subfields, rest)) {
				return false;
			}
			//rest << ") ";
		} else {
			if (v.substr(0, 2) == "c:") {
				ss << v.substr(2) << " ";
			} else {  // it's a node
				ss << "<a" << var_count << "> ";
				rest << "(<a" << var_count++ << "> ^type node ^id " << v << ") ";
			}
		}
	}
	ss << ")" << endl;
	ss << rest.str() << endl;
	return true;
}

string make_rule(
	int test_count,
	const string &val,
	const vector<pair<string,string> > &params,
	const vector<string> &fields)
{
	stringstream ss, ss1;

	ss.str("");
	ss << "sp {add-extract*" << test_count << endl
	   << "(state <s> ^superstate nil ^svs.command <c>)" << endl
	   << "-->" << endl
	   << "(<s> ^extract-info <i>)"
	   << "(<i> ^cmd <a" << var_count << "> ^count " << test_count;
	if (val != "*") {
		ss << " ^expected " << val;
	}
	if (!params.empty()) {
		ss << " ^expected-params <p>";
	}
	ss << ")" << endl;
	for (int i = 0; i < params.size(); ++i) {
		ss << "(<p> ^" << params[i].first << " " << params[i].second << ")" << endl;
	}
	
	ss << "(<c> ^extract <a" << var_count << ">)" << endl;
	
	if (!parse_filter(fields, ss)) {
		cerr << "error" << endl;
		exit(1);
	}
	ss << "}";
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

void print_usage(const char *prog) {
	cerr << "SVS filter unit testing program" << endl << endl
	     << "usage: " << prog << " [-v] [-h] [config]" << endl << endl
		 << "If [config] is specified, read configuration from that file." << endl
		 << "Otherwise read configuration from stdin." << endl
		 << "-v enables printing of input lines and generated test rules" << endl
		 << "-h prints this help text" << endl;
	exit(0);
}

int main(int argc, char *argv[]) {
	bool needs_test = false, verbose = false;
	
	Kernel *k = Kernel::CreateKernelInCurrentThread();
	k->AddRhsFunction ("count", rhs_count, NULL);
	Agent *a = k->CreateAgent("arst");
	
	for (int i = 0; rules[i]; ++i) {
		a->ExecuteCommandLine(rules[i]);
	}
	
	a->ExecuteCommandLine("waitsnc -e");
	
	string line, line_in;
	istream *input = &cin;
	for (int i = 1; i < argc; ++i) {
		if (strcmp(argv[i], "-v") == 0) {
			verbose = true;
		} else if (strcmp(argv[i], "-h") == 0) {
			print_usage(argv[0]);
		} else {
			ifstream *f = new ifstream(argv[i]);
			if (!f->is_open()) {
				cerr << "invalid config file " << argv[i] << endl;
				exit(1);
			}
			input = f;
		}
	}

	while (getline(*input, line_in)) {
		bool cont = false;
		line_in = strip(line_in, " \t", " \t");
		if (line_in.size() > 0 && line_in[line_in.size() - 1] == '\\') {
			cont = true;
			line_in.erase(line_in.size() - 1);
		}
		line_in = line_in.substr(0, line_in.find_first_of('#'));
		line += line_in;
		if (cont || line.empty()) {
			continue;
		}
		
		if (verbose) {
			cout << "INPUT: " << line << endl;
		}
		
		vector<string> fields;
		split(line, fields);
		
		if (fields[0] == "want") {
			string expected_val = fields[1];
			vector<pair<string, string> > param_tests;
			string ptstr = fields[2];
			if (ptstr != "*") {
				if (ptstr[0] != '(' || ptstr[ptstr.size()-1] != ')') {
					cerr << "expecting * or (<param tests>) in field 3" << endl;
					exit(1);
				}
				vector<string> param_fields;
				split(ptstr.substr(1, ptstr.size() - 2), param_fields);
				if (param_fields.size() % 2 != 0) {
					cerr << "parameter test should have the form (<name> <value> <name> <value> ...)" << endl;
					exit(1);
				}
				for (int i = 0; i < param_fields.size(); i += 2) {
					param_tests.push_back(make_pair(param_fields[i], param_fields[i+1]));
				}
			}
			fields.erase(fields.begin(), fields.begin() + 3);
			test_info.tests[test_count] = line;
			string rule = make_rule(test_count++, expected_val, param_tests, fields);
			if (verbose) {
				cout << "RULE: " << rule << endl;
			}
			a->ExecuteCommandLine(rule.c_str());
			needs_test = true;
		} else if (fields[0] == "repl") {
			repl(a);
		} else {
			if (needs_test) {
				a->ExecuteCommandLine("run 2");
				a->ExecuteCommandLine("excise -u");
				needs_test = false;
			}
			if (fields[0] == "test") {
				a->ExecuteCommandLine("run 2");
				a->ExecuteCommandLine("excise -u");
				needs_test = false;
			} else if (fields[0] == "init") {
				a->ExecuteCommandLine("init");
			} else {
				a->SendSVSInput(line);
			}
		}
		line.clear();
	}
	if (needs_test) {
		a->ExecuteCommandLine("run 2");
	}
	
	cout << test_info.executed - test_info.failure << " success, " 
	     << test_info.failure << " failure, " 
	     << test_info.syntax  << " syntax, " 
	     << test_count << " total"<< endl;
	
	int sum = test_info.executed + test_info.syntax;
	if (sum != test_count) {
		cout << "Warning: " << test_count - sum << " test rules did not fire" << endl;
	}
	
	return test_info.failure + test_info.syntax;
}

