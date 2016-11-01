#ifndef _STACKTRACE_H_
#define _STACKTRACE_H_

#include <stdio.h>
#include <stdlib.h>
#include <execinfo.h>
#include <cxxabi.h>
#include <sstream>
#include <algorithm>
#include <iterator>
#include <vector>
#include <string>
#include <dlfcn.h>

/** Print a demangled stack backtrace of the caller function to FILE* out. */
static inline std::string stacktrace(unsigned int max_frames = 63)
{
    void *callstack[128];
    const int nMaxFrames = sizeof(callstack) / sizeof(callstack[0]);
    char buf[1024];
    int nFrames = backtrace(callstack, nMaxFrames);
    char **symbols = backtrace_symbols(callstack, nFrames);

    std::ostringstream trace_buf;
    for (int i = 1; i < nFrames; i++) {
        printf("%s\n", symbols[i]);

        Dl_info info;
        if (dladdr(callstack[i], &info) && info.dli_sname) {
            char *demangled = NULL;
            int status = -1;
            if (info.dli_sname[0] == '_')
                demangled = abi::__cxa_demangle(info.dli_sname, NULL, 0, &status);
            snprintf(buf, sizeof(buf), "%-3d %*p %s + %zd\n",
                     i, int(2 + sizeof(void*) * 2), callstack[i],
                     status == 0 ? demangled :
                     info.dli_sname == 0 ? symbols[i] : info.dli_sname,
                     (char *)callstack[i] - (char *)info.dli_saddr);
            free(demangled);
        } else {
            snprintf(buf, sizeof(buf), "%-3d %*p %s\n",
                     i, int(2 + sizeof(void*) * 2), callstack[i], symbols[i]);
        }
        trace_buf << buf;
    }
    free(symbols);
    if (nFrames == nMaxFrames)
        trace_buf << "[truncated]\n";
    return trace_buf.str();


//    std::ostringstream oss;
//    oss << "stack trace:" << std::endl;
//
//    // storage array for stack trace address data
//    void* addrlist[max_frames+1];
//
//    // retrieve current stack addresses
//    int addrlen = backtrace(addrlist, sizeof(addrlist) / sizeof(void*));
//
//    if (addrlen == 0) {
//        oss << "  <empty, possibly corrupt>" << std::endl;
//        return oss.str();
//    }
//
//    // resolve addresses into strings containing "filename(function+address)",
//    // this array must be free()-ed
//    char** symbollist = backtrace_symbols(addrlist, addrlen);
//
//    // allocate string which will be filled with the demangled function name
//    size_t funcnamesize = 256;
//    char* funcname = (char*)malloc(funcnamesize);
//
//    // iterate over the returned symbol lines. skip the first, it is the
//    // address of this function.
//    for (int i = 1; i < addrlen; i++)
//    {
//        char *begin_name = 0, *begin_offset = 0, *end_offset = 0;
//
//        std::istringstream iss(symbollist[i]);
//        std::vector<std::string> tokens{    std::istream_iterator<std::string>{iss},
//                                            std::istream_iterator<std::string>{}};
//
//        // should be 6 in length, e.g.
//        //   2   libSoar.dylib                       0x0000000101e43201 _ZNK6SQLite9Statement8checkRowEv + 145
//
//        if (tokens.size() == 6)
//        {
//            int status;
//            char* ret = abi::__cxa_demangle(tokens[3].c_str(), funcname, &funcnamesize, &status);
//            if (status == 0)
//                tokens[3] = ret;
//
//            oss << "  " << tokens[0] << " " << tokens[1] << " " << tokens[2] << " " << tokens[3] << " + " << tokens[5] << std::endl;
//        }
//        else
//        {
//            // couldn't parse the line? print the whole line.
//            oss << "  " << symbollist[i] << std::endl;
//        }
//    }
//
//    std::string result = oss.str();
//
//    free(funcname);
//    free(symbollist);
//
//    return result;
}

#endif // _STACKTRACE_H_
