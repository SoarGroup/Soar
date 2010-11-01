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

// Conversion of value to string
template<class T> std::string& to_string( const T& x, std::string& dest )
{
	static std::ostringstream o;
	
	// get value into stream
	o << std::setprecision( 16 ) << x;
	
	dest.assign( o.str() );
	o.str("");
	return dest;
}

// Conversion from string to value
template <class T> bool from_string( T& val, const std::string& str )
{
	std::istringstream i( str );
	i >> val;
	return !i.fail();
}

// compares two numbers stored as void pointers
// used for qsort calls
template <class T>
T compare_num( const void *arg1, const void *arg2 )
{
    return *( (T *) arg1 ) - *( (T *) arg2 );
}

template <class T> bool from_string( T& val, const char* const pStr )
{
	return from_string( val, std::string( pStr ) );
}

template <class T> inline T cast_and_possibly_truncate( void* ptr )
{
	return static_cast<T>( reinterpret_cast<uintptr_t>( ptr ) );
}

// These functions have proven to be much faster than the c++ style ones above.
// TO
const size_t TO_C_STRING_BUFSIZE = 24; // uint64: 18446744073709551615 plus a few extra
inline const char* const to_c_string( const int8_t& v, char* buf ) 
{ 
	SNPRINTF( buf, TO_C_STRING_BUFSIZE, "%hhi", v ); 
	return buf; 
}
inline const char* const to_c_string( const uint8_t& v, char* buf ) 
{ 
	SNPRINTF( buf, TO_C_STRING_BUFSIZE, "%hhu", v ); 
	return buf; 
}
inline const char* const to_c_string( const int16_t& v, char* buf ) 
{ 
	SNPRINTF( buf, TO_C_STRING_BUFSIZE, "%hd", v ); 
	return buf; 
}
inline const char* const to_c_string( const uint16_t& v, char* buf ) 
{ 
	SNPRINTF( buf, TO_C_STRING_BUFSIZE, "%hu", v ); 
	return buf; 
}
inline const char* const to_c_string( const int32_t& v, char* buf ) 
{ 
	SNPRINTF( buf, TO_C_STRING_BUFSIZE, "%d", v ); 
	return buf; 
}
inline const char* const to_c_string( const uint32_t& v, char* buf ) 
{ 
	SNPRINTF( buf, TO_C_STRING_BUFSIZE, "%u", v ); 
	return buf; 
}
inline const char* const to_c_string( const int64_t& v, char* buf ) 
{ 
	SNPRINTF( buf, TO_C_STRING_BUFSIZE, "%lld", v ); 
	return buf; 
}
inline const char* const to_c_string( const uint64_t& v, char* buf ) 
{ 
	SNPRINTF( buf, TO_C_STRING_BUFSIZE, "%llu", v ); 
	return buf; 
}
inline const char* const to_c_string( const float& v, char* buf ) 
{ 
	SNPRINTF( buf, TO_C_STRING_BUFSIZE, "%f", v ); 
	return buf; 
}
inline const char* const to_c_string( const double& v, char* buf ) 
{ 
	SNPRINTF( buf, TO_C_STRING_BUFSIZE, "%lf", v ); 
	return buf; 
}
inline const char* const to_c_string( const long double& v, char* buf ) 
{ 
	SNPRINTF( buf, TO_C_STRING_BUFSIZE, "%Lf", v ); 
	return buf; 
}

// FROM
inline bool from_c_string( int8_t& v, const char* const str ) 
{ 
	return sscanf( str, "%hhd", &v ) == 1; 
}
inline bool from_c_string( uint8_t& v, const char* const str ) 
{ 
	return sscanf( str, "%hhu", &v ) == 1; 
}
inline bool from_c_string( int16_t& v, const char* const str ) 
{ 
	return sscanf( str, "%hd", &v ) == 1; 
}
inline bool from_c_string( uint16_t& v, const char* const str ) 
{ 
	return sscanf( str, "%hu", &v ) == 1; 
}
inline bool from_c_string( int32_t& v, const char* const str ) 
{ 
	//v = atoi(str);
	return sscanf( str, "%d", &v ) == 1; 
}
inline bool from_c_string( uint32_t& v, const char* const str ) 
{ 
	return sscanf( str, "%u", &v ) == 1; 
}
inline bool from_c_string( int64_t& v, const char* const str ) 
{ 
	//v = atol(str);
	return sscanf( str, "%lld", &v ) == 1; 
}
inline bool from_c_string( uint64_t& v, const char* const str ) 
{ 
	//v = strtoul(str, NULL, 10);
	return sscanf( str, "%llu", &v ) == 1; 
}
inline bool from_c_string( float& v, const char* const str ) 
{
	//v = strtof(str, NULL);
	return sscanf( str, "%f", &v ) == 1; 
}
inline bool from_c_string( double& v, const char* const str ) 
{
	//v = strtod(str, NULL);
	return sscanf( str, "%lf", &v ) == 1; 
}

inline bool from_c_string( long double& v, const char* const str ) 
{
	//v = strtold(str, NULL);
	return sscanf( str, "%Lf", &v ) == 1; 
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
struct Dangerous_Pointer_Cast {
	template <typename Given_Type>
	static Goal_Type from(Given_Type given) {
		union {
			Given_Type given;
			Goal_Type goal;
		} caster;

		caster.given = given;

		return caster.goal;
	}
};

//////////////////////////////////////////////////////////
// STLSoft Timers
//////////////////////////////////////////////////////////

// Usage:
//
// Instantiate soar_wallclock_timer or soar_process_timer objects to measure
// "wall" (real) time or process (cpu) time. Call reset if you are unsure of 
// the timer's state and want to make sure it is stopped. Call start and then
// stop to deliniate a timed period, and then call get_usec to get the value 
// of that period's length, in microseconds. It is OK to call get_usec after
// calling start again. Resolution varies by platform, and may be in the
// millisecond range on some.
//
// Use soar_timer_accumulator to rack up multiple timer periods. Instead of
// calling get_usec on the timer, simply pass the timer to a 
// soar_timer_accumulator instance with it's update call. Use reset to clear
// the accumulated time.

// Platform-specific inclusions and typedefs
//
// The STLSoft timers used in the kernel have platform-specific namespaces
// even though they share very similar interfaces. The typedefs here
// simplify the classes below by removing those namespaces. 
//
// We are using two different types of timers from STLSoft, 
// performance_counter and processtimes_counter. The performance timer is 
// a high-performance wall-clock timer. The processtimes_counter is a cpu-
// time timer. Unfortunately, the processtimes_counter on Windows has 
// unacceptable resolution, so the performance timer is used for both.
//
#ifdef WIN32
#include <winstl/performance/performance_counter.hpp>
typedef winstl::performance_counter performance_counter;
#define USE_PERFORMANCE_FOR_BOTH 1
#ifdef USE_PERFORMANCE_FOR_BOTH
typedef winstl::performance_counter processtimes_counter;	// it turns out this has higher resolution
#else // USE_PERFORMANCE_FOR_BOTH
#include <winstl/performance/processtimes_counter.hpp>
typedef winstl::processtimes_counter processtimes_counter;
#endif // USE_PERFORMANCE_FOR_BOTH
#else // WIN32
#include <unixstl/performance/performance_counter.hpp>
#include <unixstl/performance/processtimes_counter.hpp>
typedef unixstl::performance_counter performance_counter;
typedef unixstl::processtimes_counter processtimes_counter;
#endif // WIN32

// soar_timer is the basic timer interface, shared by both types of timers.
//
// To use the timer, call start, then stop. get_usec() will return the 
// amount of time in the previous start-stop period. 
//
// Calling start -> stop -> start -> get_usec is legal to minimize the
// amount of time the timer is not running. 
class soar_timer
{
public:
	// Starts the timer, does not affect get_usec
	virtual void start() = 0;

	// Stops the timer, recording the period for get_usec
	virtual void stop() = 0;

	// Simply calls start and stop to make sure the timer is not running.
	// Calling stop before start is illegal on some platforms. This behavior
	// is not well documented by STLSoft.
	virtual void reset() = 0;

	// Returns the period clocked by the last start -> stop cycle, in 
	// microseconds. This does not imply that the timer has microsecond-
	// capable resolution.
	virtual uint64_t get_usec() = 0;

protected:
	soar_timer() {}
	virtual ~soar_timer() {}
};

// soar_timer_impl is a template class implementing the common soar_timer
// interface for platform-specific timers.
template <class C>
class soar_timer_impl
	: public soar_timer
{
public:
	soar_timer_impl() { enabled_ptr=NULL; }
	~soar_timer_impl() {}

	void set_enabled( int64_t* new_enabled ) { enabled_ptr=new_enabled; }

	void start() { if ( (!enabled_ptr) || (*enabled_ptr) ) { timer.start(); } }
	void stop() { if ( (!enabled_ptr) || (*enabled_ptr) ) { timer.stop(); } }
	void reset() { if ( (!enabled_ptr) || (*enabled_ptr) ) { start(); stop(); } }
	uint64_t get_usec() { return static_cast<uint64_t>( ( ( (!enabled_ptr) || (*enabled_ptr) )?( timer.get_microseconds() ):( 0 ) ) ); }

private:
	C timer;
	int64_t* enabled_ptr;

	soar_timer_impl(const soar_timer_impl&);
	soar_timer_impl& operator=(const soar_timer_impl&);
};

// Define types and call them with more useful names such as wallclock or
// process timers.
typedef soar_timer_impl<performance_counter> soar_wallclock_timer;
typedef soar_timer_impl<processtimes_counter> soar_process_timer;

// Utility class to be used with soar_timer instances, keeps track of multiple
// intervals and has some simple time conversions.
class soar_timer_accumulator
{
private:
	uint64_t total;

public:
	soar_timer_accumulator() 
		: total(0) {}

	// Reset the accumulated time to zero.
	void reset() { total = 0; }

	// Add the timer's last interval to the accumulated time.
	void update(soar_timer& timer) { total += timer.get_usec(); }

	// Return seconds as a double.
	double get_sec() { return total / 1000000.0; }

	// Return microseconds.
	uint64_t get_usec() { return total; }

	// Return milliseconds, truncated by integer division (not rounded).
	uint64_t get_msec() { return total / 1000; }
};

#endif /*MISC_H_*/
