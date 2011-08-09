/* Minimal Soar CLI suitable for scripting */
#include <ctype.h>
#include <string.h>
#include <signal.h>
#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include "sml_Client.h"

using namespace std;
using namespace sml;

const int stopsig = SIGINT;
Kernel* kernel = NULL;
Agent* agent = NULL;

string strip(string s, string lc, string rc) {
	size_t b, e;
	b = s.find_first_not_of(lc);
	e = s.find_last_not_of(rc);
	return s.substr(b, e - b + 1);
}

/*
 * Determine how many levels of brace nesting this line adds. For
 * example, the line "sp {" will return +1, and the line "{(somthing)}}"
 * will return -1. Will avoid counting braces in quoted strings.
 */
bool totalnesting(string line, int &result) {
	int nesting = 0;
	size_t p = line.find_first_of("{}|");
	
	while (p != string::npos) {
		switch (line[p]) {
			case '{':
				++nesting;
				break;
			case '}':
				--nesting;
				break;
			case '|':
				// skip over quoted string
				while (true) {
					p = line.find_first_of('|', p+1);
					if (p == string::npos) {
						// error, no closing quote pipe on line
						return false;
					}
					if (line[p-1] != '\\') {
						break;
					}
				}
				break;
		}
		p = line.find_first_of("{}|", p+1);
	}
	result = nesting;
	return true;
}

/* Read a command, spanning newlines if the command contains unclosed braces */
bool readcmd(string &result) {
	int nestlvl, i, n;
	string line;
	stringstream cmd;
	
	nestlvl = 0;
	while(getline(cin, line)) {
		if (!totalnesting(line, n)) {
			return false;
		}
		nestlvl += n;
		cmd << line << endl;
		if (nestlvl < 0) {
			return false;
		} else if (nestlvl == 0) {
			break;
		}
	}
	
	if (nestlvl > 0) {
		return false;
	}
	result = cmd.str();
	if (!result.empty() && result[result.size()-1] == '\n') {
		result.erase(result.size()-1);
	}
	return true;
}

void printcb(smlPrintEventId id, void *d, Agent *a, char const *m) {
	cout << strip(m, "\n", "\n\t ") << endl;
}

void execcmd(string c) {
	string out = agent->ExecuteCommandLine(c.c_str());
	if (out.size() > 0) {
		cout << strip(out, "\n", "\n\t ") << endl;
	}
}

void repl() {
	string cmd;
	
	while (cin) {
		cout << endl << "% ";
		if (!readcmd(cmd)) {
			cout << "?" << endl;
			continue;
		}
		if (!cin) {
			return;
		}
		if (!cmd.empty()) {
			execcmd(cmd);
		}
	}
}

void sighandler(int sig) {
	if (agent) {
		agent->StopSelf();
	}
	signal(stopsig, sighandler);
}

string exit_handler(smlRhsEventId id, void *userdata, Agent *agent, char const *fname, char const *args) {
	int code = atoi(args);
	exit(code);
}

string log_handler(smlRhsEventId id, void *userdata, Agent *agent, char const *fname, char const *args) {
	istringstream ss(args);
	ofstream f;
	bool iscmd, append;
	string fn, text;
	int c;
	
	iscmd = false; append = false;
	while (true) {
		ss >> fn;
		if (fn == "-c") {
			iscmd = true;
		} else if (fn == "-a") {
			append = true;
		} else {
			break;
		}
	}
	while (isblank(c = ss.get()));  // ignore leading whitespace
	ss.putback(c);
	
	if (iscmd) {
		text = agent->ExecuteCommandLine(args + ss.tellg());
	} else {
		text = (args + ss.tellg());
	}
	
	f.open(fn.c_str(), ios_base::out | (append ? ios_base::app : ios_base::trunc));
	if (!f) {
		cerr << "Failed to open " << fn << " for writing" << endl;
		return "";
	}
	if (!(f << text)) {
		cerr << "Write error " << fn << endl;
		return "";
	}
	f.close();
	return "";
}

int main(int argc, char *argv[]) {
	int i, port = 12121;
	bool newthread = false, parse_error = false;
	const char *agentname = "soar";
	vector<string> sources;
	vector<string>::iterator j;

	for (i = 1; i < argc; ++i) {
		if (strcmp(argv[i], "-h") == 0) {
			parse_error = true;
			break;
		} else if (strcmp(argv[i], "-t") == 0) {
			newthread = true;
		} else if (strcmp(argv[i], "-n") == 0) {
			if (i + 1 >= argc) {
				parse_error = true;
				break;
			}
			agentname = argv[++i];
		} else if (strcmp(argv[i], "-p") == 0) {
			if (i + 1 >= argc || !(stringstream(argv[++i]) >> port)) {
				parse_error = true;
				break;
			}
		} else {
			sources.push_back(argv[i]);
		}
	}

	if (parse_error) {
		cout << "usage: " << argv[0] << " [-o] [-n <agent name>] [-p <port>] <sources ...>" << endl;
		return 1;
	}

	if (newthread) {
		kernel = Kernel::CreateKernelInNewThread(Kernel::kDefaultLibraryName, port);
	} else {
		kernel = Kernel::CreateKernelInCurrentThread(Kernel::kDefaultLibraryName, true, port);
	}

	kernel->AddRhsFunction("exit", exit_handler, NULL);
	kernel->AddRhsFunction("log", log_handler, NULL);
	
	agent = kernel->CreateAgent(agentname);
	agent->RegisterForPrintEvent(smlEVENT_PRINT, printcb, NULL);
	agent->SetOutputLinkChangeTracking(false);
	
	if (signal(stopsig, SIG_IGN) != SIG_IGN) {
		signal(stopsig, sighandler);
	}
	
	for (j = sources.begin(); j != sources.end(); ++j) {
		execcmd("source " + *j);
	}

	repl();
	return 0;
}
