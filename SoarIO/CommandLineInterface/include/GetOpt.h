#ifndef GETOPT_H
#define GETOPT_H

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

	static int GetOpt_Long(int argc, char *const *argv, const char* shortopts, const struct option* longopts, int* longind);

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


	static int _getopt_internal(int argc, char *const *argv, const char *shortopts, const struct option *longopts, int *longind, int long_only);
	static char* my_index (const char *str, int chr);
	static void exchange (char **);
	static const char *_getopt_initialize (int, char *const *, const char *);

};

} // namespace cli

#endif // GETOPT_H