#ifndef GETOPT_H
#define GETOPT_H

#include <vector>
#include <string>

#define no_argument 0
#define required_argument 1
#define optional_argument 2

namespace cli {

class GetOpt {
public:

	struct option
	{
		const char *name;
		int has_arg;
		int *flag;
		int val;
	};

	~GetOpt() { DeleteArgvIfItExists(); }

	int GetOpt_Long(std::vector<std::string>& argvector, const char* shortopts, const struct option* longopts, int* longind);

	static char *optarg;
	static int optind;
	static int opterr;
	static int optopt;

private:

	enum { REQUIRE_ORDER, PERMUTE, RETURN_IN_ORDER };
	static int ordering;
	static char *nextchar;
	static int first_nonopt;
	static int last_nonopt;
	static int argc;
	static char** argv;

	int _getopt_internal(int argc, char *const *argv, const char *shortopts, const struct option *longopts, int *longind, int long_only);
	char* my_index (const char *str, int chr);
	void exchange (char **);
	const char *_getopt_initialize (const char *);
	void DeleteArgvIfItExists();

};

} // namespace cli

#endif // GETOPT_H