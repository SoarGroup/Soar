#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

#include "cli_CommandLineInterface.h"

#include "cli_GetOpt.h"
#include "cli_Constants.h"

#include "sml_StringOps.h"
#include "sml_Names.h"

#include "IgSKI_Agent.h"
#include "IgSKI_Kernel.h"
#include "IgSKI_DoNotTouch.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::ParseWatchWMEs(gSKI::IAgent* pAgent, std::vector<std::string>& argv) {

unused(argv);
	//static struct GetOpt::option longOptions[] = {
	//	{"add-filter",		0, 0, 'a'},
	//	{"remove-filter",	0, 0, 'r'},
	//	{"list-filter",		0, 0, 'l'},
	//	{"reset-filter",	0, 0, 'R'},
	//	{"type",			1, 0, 't'},
	//	{0, 0, 0, 0}
	//};

	//unsigned int options = 0;
	//unsigned int mode = 0;

	//for (;;) {
	//	int option = m_pGetOpt->GetOpt_Long(argv, ":arlRt:", longOptions, 0);
	//	if (option == -1) break;

	//const unsigned int OPTION_WATCH_WMES_MODE_ADD		= 0x0;
	//const unsigned int OPTION_WATCH_WMES_MODE_REMOVE	= 0x1;
	//const unsigned int OPTION_WATCH_WMES_MODE_LIST		= 0x2;
	//const unsigned int OPTION_WATCH_WMES_MODE_RESET		= 0x3;

	//	switch (option) {
	//		case 'a':
	//			mode = OPTION_WATCH_WMES_MODE_ADD;
	//			break;
	//		case 'r':
	//			mode = OPTION_WATCH_WMES_MODE_REMOVE;
	//			break;
	//		case 'l':
	//			mode = OPTION_WATCH_WMES_MODE_LIST;
	//			break;
	//		case 'R':
	//			mode = OPTION_WATCH_WMES_MODE_RESET;
	//			break;
	//		case 't':
	//			{
	//				string type = GetOpt::optarg;
	//				if (type == "adds") {
	//					options = OPTION_WATCH_WMES_TYPE_ADDS;
	//				} else if (type == "removes") {
	//					options = OPTION_WATCH_WMES_TYPE_REMOVES;
	//				} else if (type == "both") {
	//					options = OPTION_WATCH_WMES_TYPE_BOTH;
	//				} else {
	//					return m_Error.SetError(CLIError::kInvalidWMEFilterType);
	//				}
	//			}
	//			break;
	//		case '?':
	//			return m_Error.SetError(CLIError::kUnrecognizedOption);
	//		default:
	//			return m_Error.SetError(CLIError::kGetOptError);
	//	}
	//}

	//if (mode == 0) return m_Error.SetError(CLIError::kTooFewArgs);

	//if (mode == OPTION_WATCH_WMES_MODE_LIST || mode == OPTION_WATCH_WMES_MODE_RESET) {
	//	// no additional arguments
	//	if (static_cast<unsigned>(GetOpt::optind) != argv.size()) return m_Error.SetError(CLIError::kTooManyArgs);

	//} else {
	//	// Pattern args
	//	
	//}

	return DoWatchWMEs(pAgent);
}

bool CommandLineInterface::DoWatchWMEs(gSKI::IAgent* pAgent) {
	unused(pAgent);
	return false;
}

///* kjh(CUSP-B2) begin */
//                char *wmes_option_syntax_msg = "\
//watch wmes syntax:\n\
//   wmes [ -on |\n\
//          -off |\n\
//          -inc[lusive] |\n\
//         {-add-filter    type filter} |\n\
//         {-remove-filter type filter} |\n\
//         {-reset-filter  type} |\n\
//         {-list-filter   type} ]\n\
//        where\n\
//          type   = -adds|-removes|-both\n\
//          filter = {id|*} {attribute|*} {value|*}";
//
//                if (i + 1 >= argc) {    /* nothing else on cmd line, so it's inclusive */
//                    if (soar_ecWatchLevel(4)) {
//                        return SOAR_ERROR;
//                    }
//                } else if ((string_match(argv[i + 1], "-on")) ||
//                           (string_match(argv[i + 1], "-off")) || (string_match_up_to("-inclusive", argv[i + 1], 3))) {
//                    if (set_watch_setting(TRACE_WM_CHANGES_SYSPARAM, argv[i], argv[i + 1], res)
//                        != SOAR_OK)
//                        return SOAR_ERROR;
//                    else
//                        i += 1;
//                } else if (i + 2 >= argc) {
//                    setSoarResultResult(res, wmes_option_syntax_msg);
//                    return SOAR_ERROR;
//                } else if (string_match(argv[i + 1], "-add-filter")) {
//                    bool forAdds, forRemoves;
//                    if ((i + 5 >= argc)
//                        || (parse_filter_type(argv[i + 2], &forAdds, &forRemoves) == SOAR_ERROR)) {
//                        appendSoarResultResult(res, wmes_option_syntax_msg);
//                        return SOAR_ERROR;
//                    } else {
//                        if (soar_ecAddWmeFilter(argv[i + 3], argv[i + 4], argv[i + 5], forAdds, forRemoves) != 0) {
//                            setSoarResultResult(res, "Error: Filter not added.");
//                            return SOAR_ERROR;
//                        } else {
//                            setSoarResultResult(res, "Filter added.");
//                        }
//                    }
//                    i += 5;
//                } else if (string_match(argv[i + 1], "-remove-filter")) {
//                    bool forAdds, forRemoves;
//                    if ((i + 5 >= argc)
//                        || (parse_filter_type(argv[i + 2], &forAdds, &forRemoves) == SOAR_ERROR)) {
//                        appendSoarResultResult(res, wmes_option_syntax_msg);
//                        return SOAR_ERROR;
//                    } else {
//                        if (soar_ecRemoveWmeFilter(argv[i + 3], argv[i + 4], argv[i + 5], forAdds, forRemoves) != 0) {
//                            setSoarResultResult(res, "Error: Bad args or filter not found");
//                            return SOAR_ERROR;
//                        } else {
//                            appendSoarResultResult(res, "Filter removed.");
//                        }
//                    }
//                    i += 5;
//                } else if (string_match(argv[i + 1], "-reset-filter")) {
//                    bool forAdds, forRemoves;
//                    if ((i + 2 >= argc)
//                        || (parse_filter_type(argv[i + 2], &forAdds, &forRemoves) == SOAR_ERROR)) {
//                        appendSoarResultResult(res, wmes_option_syntax_msg);
//                        return SOAR_ERROR;
//                    } else {
//                        if (soar_ecResetWmeFilters(forAdds, forRemoves) != 0) {
//                            appendSoarResultResult(res, "No filters were removed.");
//                            return SOAR_ERROR;
//                        }
//                    }
//                    i += 2;
//                } else if (string_match(argv[i + 1], "-list-filter")) {
//                    bool forAdds, forRemoves;
//                    if ((i + 2 >= argc) || (parse_filter_type(argv[i + 2], &forAdds, &forRemoves) == SOAR_ERROR)) {
//
//                        appendSoarResultResult(res, wmes_option_syntax_msg);
//                        return SOAR_ERROR;
//                    } else {
//                        soar_ecListWmeFilters(forAdds, forRemoves);
//                    }
//                    i += 2;
//                }
//            }
///* kjh(CUSP-B2) end */
//            /*  kjc note:  not sure CUSP-B2 solution accounts for other
//             *  non-wme args following "wmes" which should make it -inc 
//             */

