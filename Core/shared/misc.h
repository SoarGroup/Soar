/*************************************************************************
 * PLEASE SEE THE FILE "license.txt" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION.
 *************************************************************************/

/*************************************************************************
 *
 *  file:  misc.h
 *
 * =======================================================================
 */

#ifndef MISC_H_
#define MISC_H_

#include <iomanip>
#include <sstream>
#include <string>
#include <cstdio>

inline const char* get_directory_separator()
{
#ifdef WIN32
    return "\\";
#else //!WIN32
    return "/";
#endif
}

inline void normalize_separators(std::string& path)
{
    // Normalize separator chars.
    std::string::size_type j;
    while ((j = path.find('\\')) != std::string::npos)
    {
        path.replace(j, 1, "/");
    }
}

// Conversion of value to string
template<class T> std::string& to_string(const T& x, std::string& dest, int precision = 16, bool floatfixed = false)
{
    static std::ostringstream o;
    
    // get value into stream
    if (floatfixed)
    {
        o << std::fixed << std::setprecision(precision) << x;
    }
    else
    {
        o << std::setprecision(precision) << x;
    }
    
    dest.assign(o.str());
    o.str("");
    return dest;
}

// Conversion from string to value
template <class T> bool from_string(T& val, const std::string& str)
{
    std::istringstream i(str);
    i >> val;
    return !i.fail();
}

// compares two numbers stored as void pointers
// used for qsort calls
template <class T>
T compare_num(const void* arg1, const void* arg2)
{
    return *((T*) arg1) - *((T*) arg2);
}

template <class T> bool from_string(T& val, const char* const pStr)
{
    return from_string(val, std::string(pStr));
}

template <class T> inline T cast_and_possibly_truncate(void* ptr)
{
    return static_cast<T>(reinterpret_cast<uintptr_t>(ptr));
}

// These functions have proven to be much faster than the c++ style ones above.
// TO
const size_t TO_C_STRING_BUFSIZE = 24; // uint64: 18446744073709551615 plus a few extra
inline const char* const to_c_string(const int8_t& v, char* buf)
{
    SNPRINTF(buf, TO_C_STRING_BUFSIZE, "%hhi", v);
    return buf;
}
inline const char* const to_c_string(const uint8_t& v, char* buf)
{
    SNPRINTF(buf, TO_C_STRING_BUFSIZE, "%hhu", v);
    return buf;
}
inline const char* const to_c_string(const int16_t& v, char* buf)
{
    SNPRINTF(buf, TO_C_STRING_BUFSIZE, "%hd", v);
    return buf;
}
inline const char* const to_c_string(const uint16_t& v, char* buf)
{
    SNPRINTF(buf, TO_C_STRING_BUFSIZE, "%hu", v);
    return buf;
}
inline const char* const to_c_string(const int32_t& v, char* buf)
{
    SNPRINTF(buf, TO_C_STRING_BUFSIZE, "%d", v);
    return buf;
}
inline const char* const to_c_string(const uint32_t& v, char* buf)
{
    SNPRINTF(buf, TO_C_STRING_BUFSIZE, "%u", v);
    return buf;
}
inline const char* const to_c_string(const int64_t& v, char* buf)
{
    SNPRINTF(buf, TO_C_STRING_BUFSIZE, "%lld", static_cast<long long>(v));
    return buf;
}
inline const char* const to_c_string(const uint64_t& v, char* buf)
{
    SNPRINTF(buf, TO_C_STRING_BUFSIZE, "%llu", static_cast<long long unsigned>(v));
    return buf;
}
inline const char* const to_c_string(const float& v, char* buf)
{
    SNPRINTF(buf, TO_C_STRING_BUFSIZE, "%f", v);
    return buf;
}
inline const char* const to_c_string(const double& v, char* buf)
{
    SNPRINTF(buf, TO_C_STRING_BUFSIZE, "%lf", v);
    return buf;
}
inline const char* const to_c_string(const long double& v, char* buf)
{
    SNPRINTF(buf, TO_C_STRING_BUFSIZE, "%Lf", v);
    return buf;
}

// FROM
inline bool from_c_string(int8_t& v, const char* const str)
{
    return sscanf(str, "%hhd", &v) == 1;
}
inline bool from_c_string(uint8_t& v, const char* const str)
{
    return sscanf(str, "%hhu", &v) == 1;
}
inline bool from_c_string(int16_t& v, const char* const str)
{
    return sscanf(str, "%hd", &v) == 1;
}
inline bool from_c_string(uint16_t& v, const char* const str)
{
    return sscanf(str, "%hu", &v) == 1;
}
inline bool from_c_string(int32_t& v, const char* const str)
{
    //v = atoi(str);
    return sscanf(str, "%d", &v) == 1;
}
inline bool from_c_string(uint32_t& v, const char* const str)
{
    return sscanf(str, "%u", &v) == 1;
}
inline bool from_c_string(int64_t& v, const char* const str)
{
    long long vt = 0;
    bool ret = sscanf(str, "%lld", &vt) == 1;
    v = static_cast<int64_t>(vt);
    return ret;
}
inline bool from_c_string(uint64_t& v, const char* const str)
{
    long long unsigned vt = 0;
    bool ret = sscanf(str, "%llu", &vt) == 1;
    v = static_cast<uint64_t>(vt);
    return ret;
}
inline bool from_c_string(float& v, const char* const str)
{
    //v = strtof(str, NULL);
    return sscanf(str, "%f", &v) == 1;
}
inline bool from_c_string(double& v, const char* const str)
{
    //v = strtod(str, NULL);
    return sscanf(str, "%lf", &v) == 1;
}

inline bool from_c_string(long double& v, const char* const str)
{
    //v = strtold(str, NULL);
    return sscanf(str, "%Lf", &v) == 1;
}

/** Casting between pointer-to-function and pointer-to-object is hard to do... legally
 *
 *  reinterpret_cast<...>(...) isn't actually capable of performing such casts in many
 *  compilers, complaining that it isn't ISO C++.
 *
 *  This function doesn't really do anything to guarantee that the provided types are
 *  pointers, so use it appropriately.
 */

template <typename Goal_Type>
struct Dangerous_Pointer_Cast
{
    template <typename Given_Type>
    static Goal_Type from(Given_Type given)
    {
        union
        {
            Given_Type given;
            Goal_Type goal;
        } caster;
        
        caster.given = given;
        
        return caster.goal;
    }
};

// To use the timer, call start, then stop. get_usec() will return the
// amount of time in the previous start-stop period.
//
// Calling start -> stop -> start -> get_usec is legal to minimize the
// amount of time the timer is not running.
class soar_timer
{
    public:
        soar_timer()
            : enabled_ptr(NULL), t1(0), elapsed(0)
        {
            raw_per_usec = get_raw_time_per_usec();
        }
        
        void set_enabled(int64_t* new_enabled)
        {
            enabled_ptr = new_enabled;
        }
        
        void start()
        {
            if ((!enabled_ptr) || (*enabled_ptr))
            {
                t1 = get_raw_time();
            }
        }
        
        void stop()
        {
            if ((!enabled_ptr) || (*enabled_ptr))
            {
                uint64_t t2 = get_raw_time();
                elapsed = t2 - t1;
            }
        }
        
        void reset()
        {
            t1 = elapsed = 0;
        }
        
        uint64_t get_usec()
        {
            if ((!enabled_ptr) || (*enabled_ptr))
            {
                return static_cast<uint64_t>(elapsed / raw_per_usec);
            }
            return 0;
        }
        
    private:
        uint64_t t1, elapsed;
        double raw_per_usec;
        int64_t* enabled_ptr;
        
        soar_timer(const soar_timer&);
        soar_timer& operator=(const soar_timer&);
};

// Utility class to be used with soar_timer instances, keeps track of multiple
// intervals and has some simple time conversions.
class soar_timer_accumulator
{
    private:
        uint64_t total;
        
    public:
        soar_timer_accumulator() : total(0) {}
        
        // Reset the accumulated time to zero.
        void reset()
        {
            total = 0;
        }
        
        // Add the timer's last interval to the accumulated time.
        void update(soar_timer& timer)
        {
            total += timer.get_usec();
        }
        
        // Return seconds as a double.
        double get_sec()
        {
            return total / 1000000.0;
        }
        
        // Return microseconds.
        uint64_t get_usec()
        {
            return total;
        }
        
        // Return milliseconds, truncated by integer division (not rounded).
        uint64_t get_msec()
        {
            return total / 1000;
        }
};

#endif /*MISC_H_*/

