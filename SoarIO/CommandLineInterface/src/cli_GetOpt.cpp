#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

#include "cli_GetOpt.h"

#include <string.h>

using namespace cli;

char*	GetOpt::optarg = 0;
int		GetOpt::optind = 1;
char*	GetOpt::nextchar = 0;
int		GetOpt::opterr = 1;
int		GetOpt::optopt = '?';
int 	GetOpt::first_nonopt;
int 	GetOpt::last_nonopt;
int		GetOpt::ordering;
int		GetOpt::argc = 0;
char**	GetOpt::argv = 0;

void GetOpt::Initialize() {
	optind = 0;
	opterr = 0;
}

char* GetOpt::my_index (const char *str, int chr)
{
	while (*str)
	{
		if (*str == chr)
			return (char *) str;
		str++;
	}
	return 0;
}

#  define SWAP_FLAGS(ch1, ch2)

void GetOpt::exchange (char **argv)
{
	int bottom = first_nonopt;
	int middle = last_nonopt;
	int top = optind;
	char *tem;

	/* Exchange the shorter segment with the far end of the longer segment.
	That puts the shorter segment into the right place.
	It leaves the longer segment in the right place overall,
	but it consists of two parts that need to be swapped next.  */

	while (top > middle && middle > bottom)
	{
		if (top - middle > middle - bottom)
		{
			/* Bottom segment is the short one.  */
			int len = middle - bottom;
			register int i;

			/* Swap it with the top part of the top segment.  */
			for (i = 0; i < len; i++)
			{
				tem = argv[bottom + i];
				argv[bottom + i] = argv[top - (middle - bottom) + i];
				argv[top - (middle - bottom) + i] = tem;
				SWAP_FLAGS (bottom + i, top - (middle - bottom) + i);
			}
			/* Exclude the moved bottom segment from further swapping.  */
			top -= len;
		}
		else
		{
			/* Top segment is the short one.  */
			int len = top - middle;
			register int i;

			/* Swap it with the bottom part of the bottom segment.  */
			for (i = 0; i < len; i++)
			{
				tem = argv[bottom + i];
				argv[bottom + i] = argv[middle + i];
				argv[middle + i] = tem;
				SWAP_FLAGS (bottom + i, middle + i);
			}
			/* Exclude the moved top segment from further swapping.  */
			bottom += len;
		}
	}

	/* Update records for the slots the non-options now occupy.  */

	first_nonopt += (optind - last_nonopt);
	last_nonopt = optind;
}

/* Initialize the internal data when the first call is made.  */


const char* GetOpt::_getopt_initialize (const char *optstring)
{
	/* Start processing options with ARGV-element 1 (since ARGV-element 0
	is the program name); the sequence of previously skipped
	non-option ARGV-elements is empty.  */

	first_nonopt = last_nonopt = optind;

	nextchar = 0;

	/* Determine how to handle the ordering of options and nonoptions.  */

	if (optstring[0] == '-')
	{
		ordering = RETURN_IN_ORDER;
		++optstring;
	}
	else if (optstring[0] == '+')
	{
		ordering = REQUIRE_ORDER;
		++optstring;
	}
	else 
		ordering = PERMUTE;

	return optstring;
}

int GetOpt::_getopt_internal (int argc, char *const *argv, const char *optstring, const struct option *longopts, int *longind, int long_only)
{
	if (argc < 1)
		return -1;

	optarg = 0;

	if (optind == 0)
	{
		if (optind == 0)
			optind = 1;	/* Don't scan ARGV[0], the program name.  */
		optstring = _getopt_initialize (optstring);
	}

	/* Test whether ARGV[optind] points to a non-option argument.
	Either it does not have option syntax, or there is an environment flag
	from the shell indicating it is not an option.  The later information
	is only used when the used in the GNU libc.  */
#if defined _LIBC && defined USE_NONOPTION_FLAGS
# define NONOPTION_P (argv[optind][0] != '-' || argv[optind][1] == '\0'	      \
	|| (optind < nonoption_flags_len			      \
	&& __getopt_nonoption_flags[optind] == '1'))
#else
# define NONOPTION_P (argv[optind][0] != '-' || argv[optind][1] == '\0')
#endif

	if (nextchar == 0 || *nextchar == '\0')
	{
		/* Advance to the next ARGV-element.  */

		/* Give FIRST_NONOPT & LAST_NONOPT rational values if OPTIND has been
		moved back by the user (who may also have changed the arguments).  */
		if (last_nonopt > optind)
			last_nonopt = optind;
		if (first_nonopt > optind)
			first_nonopt = optind;

		if (ordering == PERMUTE)
		{
			/* If we have just processed some options following some non-options,
			exchange them so that the options come first.  */

			if (first_nonopt != last_nonopt && last_nonopt != optind)
				exchange ((char **) argv);
			else if (last_nonopt != optind)
				first_nonopt = optind;

			/* Skip any additional non-options
			and extend the range of non-options previously skipped.  */

			while (optind < argc && NONOPTION_P)
				optind++;
			last_nonopt = optind;
		}

		/* The special ARGV-element `--' means premature end of options.
		Skip it like a null option,
		then exchange with previous non-options as if it were an option,
		then skip everything else like a non-option.  */

		if (optind != argc && !strcmp (argv[optind], "--"))
		{
			optind++;

			if (first_nonopt != last_nonopt && last_nonopt != optind)
				exchange ((char **) argv);
			else if (first_nonopt == last_nonopt)
				first_nonopt = optind;
			last_nonopt = argc;

			optind = argc;
		}

		/* If we have done all the ARGV-elements, stop the scan
		and back over any non-options that we skipped and permuted.  */

		if (optind == argc)
		{
			/* Set the next-arg-index to point at the non-options
			that we previously skipped, so the caller will digest them.  */
			if (first_nonopt != last_nonopt)
				optind = first_nonopt;
			return -1;
		}

		/* If we have come to a non-option and did not permute it,
		either stop the scan or describe it to the caller and pass it by.  */

		if (NONOPTION_P)
		{
			if (ordering == REQUIRE_ORDER)
				return -1;
			optarg = argv[optind++];
			return 1;
		}

		/* We have found another option-ARGV-element.
		Skip the initial punctuation.  */

		nextchar = (argv[optind] + 1
			+ (longopts != 0 && argv[optind][1] == '-'));
	}

	/* Decode the current option-ARGV-element.  */

	/* Check whether the ARGV-element is a long option.

	If long_only and the ARGV-element has the form "-f", where f is
	a valid short option, don't consider it an abbreviated form of
	a long option that starts with f.  Otherwise there would be no
	way to give the -f short option.

	On the other hand, if there's a long option "fubar" and
	the ARGV-element is "-fu", do consider that an abbreviation of
	the long option, just like "--fu", and not "-f" with arg "u".

	This distinction seems to be the most useful approach.  */

	if (longopts != 0
		&& (argv[optind][1] == '-'
		|| (long_only && (argv[optind][2] || !my_index (optstring, argv[optind][1])))))
	{
		char *nameend;
		const struct option *p;
		const struct option *pfound = 0;
		int exact = 0;
		int ambig = 0;
		int indfound = -1;
		int option_index;

		for (nameend = nextchar; *nameend && *nameend != '='; nameend++)
			/* Do nothing.  */ ;

			/* Test all long options for either exact match
			or abbreviated matches.  */
			for (p = longopts, option_index = 0; p->name; p++, option_index++)
				if (!strncmp (p->name, nextchar, nameend - nextchar))
				{
					if ((unsigned int) (nameend - nextchar)
						== (unsigned int) strlen (p->name))
					{
						/* Exact match found.  */
						pfound = p;
						indfound = option_index;
						exact = 1;
						break;
					}
					else if (pfound == 0)
					{
						/* First nonexact match found.  */
						pfound = p;
						indfound = option_index;
					}
					else if (long_only
						|| pfound->has_arg != p->has_arg
						|| pfound->flag != p->flag
						|| pfound->val != p->val)
						/* Second or later nonexact match found.  */
						ambig = 1;
				}

				if (ambig && !exact)
				{
					nextchar += strlen (nextchar);
					optind++;
					optopt = 0;
					return '?';
				}

				if (pfound != 0)
				{
					option_index = indfound;
					optind++;
					if (*nameend)
					{
						/* Don't test has_arg with >, because some C compilers don't
						allow it to be used on enums.  */
						if (pfound->has_arg)
							optarg = nameend + 1;
						else
						{

							nextchar += strlen (nextchar);

							optopt = pfound->val;
							return '?';
						}
					}
					else if (pfound->has_arg == 1)
					{
						if (optind < argc)
							optarg = argv[optind++];
						else
						{
							nextchar += strlen (nextchar);
							optopt = pfound->val;
							return optstring[0] == ':' ? ':' : '?';
						}
					}
					nextchar += strlen (nextchar);
					if (longind != 0)
						*longind = option_index;
					if (pfound->flag)
					{
						*(pfound->flag) = pfound->val;
						return 0;
					}
					return pfound->val;
				}

				/* Can't find it as a long option.  If this is not getopt_long_only,
				or the option starts with '--' or is not a valid short
				option, then it's an error.
				Otherwise interpret it as a short option.  */
				if (!long_only || argv[optind][1] == '-'
					|| my_index (optstring, *nextchar) == 0)
				{
					nextchar = (char *) "";
					optind++;
					optopt = 0;
					return '?';
				}
	}

	/* Look at and handle the next short option-character.  */

	{
		char c = *nextchar++;
		char *temp = my_index (optstring, c);

		/* Increment `optind' when we start to process its last character.  */
		if (*nextchar == '\0')
			++optind;

		if (temp == 0 || c == ':')
		{
			optopt = c;
			return '?';
		}
		/* Convenience. Treat POSIX -W foo same as long option --foo */
		if (temp[0] == 'W' && temp[1] == ';')
		{
			char *nameend;
			const struct option *p;
			const struct option *pfound = 0;
			int exact = 0;
			int ambig = 0;
			int indfound = 0;
			int option_index;

			/* This is an option that requires an argument.  */
			if (*nextchar != '\0')
			{
				optarg = nextchar;
				/* If we end this ARGV-element by taking the rest as an arg,
				we must advance to the next element now.  */
				optind++;
			}
			else if (optind == argc)
			{
				optopt = c;
				if (optstring[0] == ':')
					c = ':';
				else
					c = '?';
				return c;
			}
			else
				/* We already incremented `optind' once;
				increment it again when taking next ARGV-elt as argument.  */
				optarg = argv[optind++];

			/* optarg is now the argument, see if it's in the
			table of longopts.  */

			for (nextchar = nameend = optarg; *nameend && *nameend != '='; nameend++)
				/* Do nothing.  */ ;

				/* Test all long options for either exact match
				or abbreviated matches.  */
				for (p = longopts, option_index = 0; p->name; p++, option_index++)
					if (!strncmp (p->name, nextchar, nameend - nextchar))
					{
						if ((unsigned int) (nameend - nextchar) == strlen (p->name))
						{
							/* Exact match found.  */
							pfound = p;
							indfound = option_index;
							exact = 1;
							break;
						}
						else if (pfound == 0)
						{
							/* First nonexact match found.  */
							pfound = p;
							indfound = option_index;
						}
						else
							/* Second or later nonexact match found.  */
							ambig = 1;
					}
					if (ambig && !exact)
					{
						nextchar += strlen (nextchar);
						optind++;
						return '?';
					}
					if (pfound != 0)
					{
						option_index = indfound;
						if (*nameend)
						{
							/* Don't test has_arg with >, because some C compilers don't
							allow it to be used on enums.  */
							if (pfound->has_arg)
								optarg = nameend + 1;
							else
							{
								nextchar += strlen (nextchar);
								return '?';
							}
						}
						else if (pfound->has_arg == 1)
						{
							if (optind < argc)
								optarg = argv[optind++];
							else
							{
								nextchar += strlen (nextchar);
								return optstring[0] == ':' ? ':' : '?';
							}
						}
						nextchar += strlen (nextchar);
						if (longind != 0)
							*longind = option_index;
						if (pfound->flag)
						{
							*(pfound->flag) = pfound->val;
							return 0;
						}
						return pfound->val;
					}
					nextchar = 0;
					return 'W';	/* Let the application handle it.   */
		}
		if (temp[1] == ':')
		{
			if (temp[2] == ':')
			{
				/* This is an option that accepts an argument optionally.  */
				if (*nextchar != '\0')
				{
					optarg = nextchar;
					optind++;
				}
				//else
				//	optarg = 0;
//voigtjr start
				else 
				{
					if (!argv[optind])
						optarg = 0;
					else
					{
						if (argv[optind][0] == '-')
							optarg = 0;
						else
							optarg = argv[optind++];
					}

				}
//voigtjr end
				nextchar = 0;
			}
			else
			{
				/* This is an option that requires an argument.  */
				if (*nextchar != '\0')
				{
					optarg = nextchar;
					/* If we end this ARGV-element by taking the rest as an arg,
					we must advance to the next element now.  */
					optind++;
				}
				else if (optind == argc)
				{
					optopt = c;
					if (optstring[0] == ':')
						c = ':';
					else
						c = '?';
				}
				else
					/* We already incremented `optind' once;
					increment it again when taking next ARGV-elt as argument.  */
					optarg = argv[optind++];
				nextchar = 0;
			}
		}
		return c;
	}
}

int GetOpt::GetOpt_Long (std::vector<std::string>& argvector, const char *options, const struct option *long_options, int *opt_index)
{
	if (optind == 0) {
		DeleteArgvIfItExists();

		argc = argvector.size();

		argv = new char*[argc + 1]; // leave space for extra null
		int arglen;

		// For each arg
		for (int i = 0; i < argc; ++i) {
			// Save its length
			arglen = argvector[i].length();

			// Leave space for null
			argv[i] = new char[ arglen + 1 ];

			// Copy the string
			strncpy(argv[i], argvector[i].data(), arglen);

			// Set final index to null
			argv[i][ arglen ] = 0;
		}
		// Set final index to null
		argv[argc] = 0;
	} else {
		argc = argvector.size();
	}

	int ret = _getopt_internal (argc, argv, options, long_options, opt_index, 0);
	
	for (int j = 0; j < argc; ++j) {
		argvector[j] = argv[j];
	}

	return ret;
}

void GetOpt::DeleteArgvIfItExists() {
	if (!argv) return;

	for (int i = 0; i < argc; ++i) {
		delete [] argv[i];
		argv[i] = 0;
	}
	delete [] argv;
	argv = 0;
}
