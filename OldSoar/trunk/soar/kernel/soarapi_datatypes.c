#include "soarkernel.h"
#include "soarapi_datatypes.h"

#ifdef USE_STDARGS
void setSoarResultResult(soarResult * res, const char *format, ...)
{
    va_list args;

    va_start(args, format);

#else
void setSoarResultResult(va_alist)
va_dcl
{
    va_list args;
    char *format;
    soarResult *res;

    va_start(args);
    res = va_arg(args, soarResult *);
    format = va_arg(args, char *);
#endif

    vsnprintf(res->result, SOARRESULT_RESULT_LENGTH, format, args);
    res->result[SOARRESULT_RESULT_LENGTH - 1] = 0;      /* vsnprintf doesn't set last char to null if output is truncated */
}

#ifdef USE_STDARGS
void appendSoarResultResult(soarResult * res, const char *format, ...)
{
    va_list args;
    int i;

    va_start(args, format);
#else
void appendSoarResultResult(va_alist)
va_dcl
{
    va_list args;
    char *format;
    soarResult *res;
    int i;

    va_start(args);
    res = va_arg(args, soarResult *);
    format = va_arg(args, char *);
#endif

    i = 0;
    while (res->result[i])
        i++;

    vsnprintf(&res->result[i], SOARRESULT_RESULT_LENGTH - i, format, args);
    res->result[SOARRESULT_RESULT_LENGTH - 1] = 0;      /* vsnprintf doesn't set last char to null if output is truncated */

}

#ifdef USE_STDARGS
void appendSymbolsToSoarResultResult(soarResult * res, const char *format, ...)
{
    va_list args;
    char *r, *end;

    va_start(args, format);
#else
void appendSymbolsToSoarResultResult(va_alist)
va_dcl
{
    va_list args;
    char *format;
    soarResult *res;
    char *r, *end;

    va_start(args);
    res = va_arg(args, soarResult *);
    format = va_arg(args, char *);
#endif

    r = res->result;

    end = r + res->resultLength;
    while (*r)
        r++;

    while (r < end) {

        while ((*format != '%') && (*format))
            *(r++) = *(format++);
        if (*format == 0)
            break;

        if (*(format + 1) == 'y') {
            /* Bug: Hope we have enough room! */
            symbol_to_string(va_arg(args, Symbol *), TRUE, r, SOARRESULT_RESULT_LENGTH - (r - res->result));
            while (*r)
                r++;
        } else {
            *(r++) = '%';
        }
        format += 2;
    }
    va_end(args);

    *r = 0;

}
