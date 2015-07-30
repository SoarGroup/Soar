#ifdef DEBUG_MAC_STACKTRACE
#ifndef WIN32
/*
 * Simple and lightweight memory leak detector
 * Copyright (c) 2011,2012,2013 Mario 'rlyeh' Rodriguez

 * Callstack code is based on code by Magnus Norddahl (ClanLib)

 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.

 * Features:
 * - tiny
 * - clean
 * - callstack based
 * - lazy. should work with no source modification
 * - macroless API. no new/delete macros
 * - embeddable. just include this source file in your project

 * To do:
 * - support for malloc/realloc/free

 * References:
 * - http://www.codeproject.com/KB/cpp/allocator.aspx
 * - http://drdobbs.com/cpp/184403759

 * - rlyeh ~~ listening to Long Distance Calling / Metulsky Curse Revisited
 */

#include <cassert>
#include <cstddef>
#include <cstdlib>
#include <cstdio>
#include <cstring>

#include <algorithm>
#include <cctype>
#include <deque>
#include <iomanip>
#include <iostream>
#include <limits>
#include <map>
#include <memory>
#include <new>
#include <sstream>
#include <string>
#include <vector>

#include "debug_trace.h"

#include "debug_thread.h" // header not found? copy tinythread++ sources to tracey's folder!
namespace std
{
    using namespace tthread;
}

#   include <unistd.h>
#   include <sys/time.h>
#   include <sys/types.h>
#   include <execinfo.h>
#   include <cxxabi.h>

// Messages

// OS utils

#define $no(...)
#define $yes(...)     __VA_ARGS__

#if defined(_WIN32) || defined(_WIN64)
#   define $windows   $yes
#   define $welse     $no
#else
#   define $windows   $no
#   define $welse     $yes
#endif

#ifdef __APPLE__
#   define $apple     $yes
#   define $aelse     $no
#else
#   define $apple     $no
#   define $aelse     $yes
#endif

#ifdef __linux__
#   define $linux     $yes
#   define $lelse     $no
#else
#   define $linux     $no
#   define $lelse     $yes
#endif

// Compiler utils

#if defined(NDEBUG) || defined(_NDEBUG)
#   define $release   $yes
#   define $debug     $no
#else
#   define $release   $no
#   define $debug     $yes
#endif

#ifdef __GNUC__
#   define $gnuc      $yes
#   define $gelse     $no
#else
#   define $gnuc      $no
#   define $gelse     $yes
#endif

#ifdef _MSC_VER
#   define $msvc      $yes
#   define $melse     $no
#else
#   define $msvc      $no
#   define $melse     $yes
#endif

// Implementation

namespace tracey
{
    class string : public std::string
    {
        public:

            // basic constructors

            string() : std::string()
            {}

            template<size_t N>
            string(const char(&cstr)[N]) : std::string(cstr)
            {}

            // constructor sugar

            template< typename T >
            string(const T& t) : std::string()
            {
                operator=(t);
            }

            // extended constructors; safe formatting

        private:
            template<unsigned N>
            void safefmt(const std::string& fmt, std::string* t)
            {
                for (std::string::const_iterator it = fmt.begin(), end = fmt.end(); it != end; ++it)
                {
                    unsigned index(*it);
                    if (index <= N)
                    {
                        t[0] += t[index];
                    }
                    else
                    {
                        t[0] += *it;
                    }
                }
                assign(t[0]);
            }
        public:

            template< typename T1 >
            string(const std::string& fmt, const T1& t1) : std::string()
            {
                std::string t[] = { std::string(), string(t1) };
                safefmt<1>(fmt, t);
            }

            template< typename T1, typename T2 >
            string(const std::string& fmt, const T1& t1, const T2& t2) : std::string()
            {
                std::string t[] = { std::string(), string(t1), string(t2) };
                safefmt<2>(fmt, t);
            }

            template< typename T1, typename T2, typename T3 >
            string(const std::string& fmt, const T1& t1, const T2& t2, const T3& t3) : std::string()
            {
                std::string t[] = { std::string(), string(t1), string(t2), string(t3) };
                safefmt<3>(fmt, t);
            }

            template< typename T1, typename T2, typename T3, typename T4 >
            string(const std::string& fmt, const T1& t1, const T2& t2, const T3& t3, const T4& t4) : std::string()
            {
                std::string t[] = { std::string(), string(t1), string(t2), string(t3), string(t4) };
                safefmt<4>(fmt, t);
            }

            template< typename T1, typename T2, typename T3, typename T4, typename T5 >
            string(const std::string& fmt, const T1& t1, const T2& t2, const T3& t3, const T4& t4, const T5& t5) : std::string()
            {
                std::string t[] = { std::string(), string(t1), string(t2), string(t3), string(t4), string(t5) };
                safefmt<5>(fmt, t);
            }

            template< typename T1, typename T2, typename T3, typename T4, typename T5, typename T6 >
            string(const std::string& fmt, const T1& t1, const T2& t2, const T3& t3, const T4& t4, const T5& t5, const T6& t6) : std::string()
            {
                std::string t[] = { std::string(), string(t1), string(t2), string(t3), string(t4), string(t5), string(t6) };
                safefmt<6>(fmt, t);
            }

            template< typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7 >
            string(const std::string& fmt, const T1& t1, const T2& t2, const T3& t3, const T4& t4, const T5& t5, const T6& t6, const T7& t7) : std::string()
            {
                std::string t[] = { std::string(), string(t1), string(t2), string(t3), string(t4), string(t5), string(t6), string(t7) };
                safefmt<7>(fmt, t);
            }

            // chaining operators

            template< typename T >
            string& operator +=(const T& t)
            {
                this->assign(this->str() + string(t));
                return *this;
            }

            // assignment sugars

            template< typename T >
            string& operator=(const T& t)
            {
                std::stringstream ss;
                ss.precision(std::numeric_limits< long double >::digits10 + 1);
                if (ss << t)
                {
                    this->assign(ss.str());
                }
                return *this;
            }

            string& operator=(const char* t)
            {
                this->assign(t ? t : "");
                return *this;
            }

            std::string str() const
            {
                return *this;
            }

            size_t count(const std::string& substr) const
            {
                size_t n = 0;
                std::string::size_type pos = 0;
                while ((pos = this->find(substr, pos)) != std::string::npos)
                {
                    n++;
                    pos += substr.size();
                }
                return n;
            }

            std::deque< string > tokenize(const std::string& chars) const
            {
                std::string map(256, '\0');

                for (std::string::const_iterator it = chars.begin(), end = chars.end(); it != end; ++it)
                {
                    map[ *it ] = '\1';
                }

                std::deque< string > tokens;

                tokens.push_back(string());

                for (const_iterator it = this->begin(), end = this->end(); it != end; ++it)
                {
                    if (!map[*it])
                    {
                        tokens.back().push_back(*it);
                    }
                    else if (tokens.back().size())
                    {
                        tokens.push_back(string());
                    }
                }

                while (tokens.size() && !tokens.back().size())
                {
                    tokens.pop_back();
                }

                return tokens;
            }

    };

    class strings : public std::deque< string >
    {
        public:

            strings()
            {}

            template <typename contained>
            strings(const std::deque<contained>& c)
            {
                operator=(c);
            }

            template <typename contained>
            strings& operator =(const std::deque<contained>& c)
            {
                this->resize(c.size());
                std::copy(c.begin(), c.end(), this->begin());
                return *this;
            }

            std::string str(const char* format1 = "\1\n") const
            {
                if (this->size() == 1)
                {
                    return *this->begin();
                }

                std::string out;

                for (const_iterator it = this->begin(); it != this->end(); ++it)
                {
                    out += string(format1, (*it));
                }

                return out;
            }
    };

    class mutex
    {
        public:

            mutex() : locked_by(nobody())
            {
                static struct once
                {
                    once(mutex* self)
                    {
                        if (self->me() == self->nobody())
                        {
                            std::fprintf(stderr, "%s", "<tracey/tracey.cpp> says: g++ user? forgot to add -lpthread?\n");
                            std::exit(-1);
                        }
                    }
                } check(this);
            }

            void lock()
            {
                if (locked_by != me())
                {
                    self.lock();
                    locked_by = me();
                }
            }

            void unlock()
            {
                if (locked_by == me())
                {
                    locked_by = nobody();
                    self.unlock();
                }
            }

            bool try_lock()
            {
                if (locked_by != me())
                {
                    if (!self.try_lock())
                    {
                        return false;
                    }
                    locked_by = me();
                }
                return true;
            }

            bool is_locked() const
            {
                return locked_by != nobody();
            }

            bool is_locked_by_me() const
            {
                return locked_by == me();
            }

        private:

            std::thread::id locked_by;

            std::mutex self;

            std::thread::id nobody() const   // return empty/invalid thread
            {
                return std::thread::id();
            }

            std::thread::id me() const   // return current thread id
            {
                return std::this_thread::get_id();
            }

            // Private copy-constructor, copy-assignment

            mutex(const mutex&);
            mutex& operator=(const mutex&);
    };

    std::string demangle(const std::string& name)
    {
        // format: number  filename  address  funcname + offset
        // find beginning and end of function name
        const char* b = name.data();
        for (; *b == ' '; ++b);      // skip to first non-space char
        for (int i = 0; i < 3; ++i)
        {
            for (; *b != ' '; ++b);  // skip to end of this word
            for (; *b == ' '; ++b);  // skip to beginning of next word
        }
        char* e = strrchr((char*) b, '+'); // look for last '+'
        if (!e || e <= b || *--e != ' ')
        {
            return name;    // abort if bad format
        }
        static size_t alloc_size = 1024;
        char* demangled = static_cast<char*>(malloc(alloc_size));
        size_t sz = alloc_size;
        int status;
        *e = '\0';  // terminate string for __cxa_demangle
        abi::__cxa_demangle(b, demangled, &sz, &status);
        *e = ' ';   // restore original
        if (sz > alloc_size)
        {
            alloc_size = sz;    // update alloc_size if __cxa_demangle called realloc
        }
        if (!status)
        {
            /* Remove what is between parentheses.  Don't need parameters. */
            size_t paren_start, paren_end;
            std::string returnString, demangledString(demangled);
            paren_start = demangledString.find_first_of('(');
            paren_end = demangledString.find_first_of(')');
            returnString = demangledString.erase(paren_start, paren_end) + std::string(e);
            free(demangled);
            return returnString;
        }
        else
        {
            return name;
        }
    }

    class callstack
    {
        public:

            // save
            callstack();

            // print
            std::string str(const char* format12 = "#\1 \2\n", size_t skip_begin = 0, size_t skip_end = 0);

        private:

            enum { max_frames = 32 };
            void* frames[max_frames];
            size_t num_frames;
    };

    namespace
    {
        size_t capture_stack_trace(int frames_to_skip, int max_frames, void** out_frames)
        {
            return 0;
        }
        tracey::strings resolve_stack_trace(void** frames, unsigned num_frames)
        {
            return tracey::strings();
        }
    }

    callstack::callstack() // save
    {
        num_frames = tracey::capture_stack_trace(1, max_frames, frames);
        for (int i = num_frames; i < max_frames; ++i)
        {
            frames[ i ] = 0;
        }
    }

    std::string callstack::str(const char* format12, size_t skip_begin, size_t skip_end)
    {
        tracey::string out;

        tracey::strings stack_trace = tracey::resolve_stack_trace(frames, num_frames);

        for (size_t i = skip_begin, end = stack_trace.size() - skip_end; i < end; i++)
        {
            out += tracey::string(format12, (int)i, stack_trace[i]);
        }

        return out;
    }
}

#ifndef kTraceyAllocMultiplier
/*  Used to increase memory requirements, and to simulate and to debug worse memory conditions */
/*  Should be equal or bigger than 1.0                                                         */
#   define kTraceyAllocMultiplier  1.0
#endif

#ifndef kTraceyAlloc
/*  Behaviour is undefined at end of program if kTraceyAlloc() implementation uses any global C++ symbol */
#   define kTraceyAlloc(size)      std::malloc(size)
#endif

#ifndef kTraceyFree
/*  Behaviour is undefined at end of program if kTraceyFree() implementation uses any global C++ symbol */
#   define kTraceyFree(ptr)        std::free(ptr)
#endif

#ifndef kTraceyPrint
/*  Behaviour is undefined at end of program if kTraceyPrint() implementation uses any C++ global symbol, like std::cout */
#   define kTraceyPrint(str)       std::fprintf( stderr, "%s", tracey::string( str ).c_str() )
#endif

#ifndef kTraceyAssert
/*  Behaviour is undefined at end of program if kTraceyAssert() implementation uses any C++ global symbol */
#   define kTraceyAssert(expr)     assert(expr)
#endif

#ifndef kTraceyBadAlloc
#   define kTraceyBadAlloc()       std::bad_alloc()
#endif

#ifndef kTraceyReportWildPointers
#   define kTraceyReportWildPointers 1
#endif

#ifndef kTraceyReportNullPointers
#   define kTraceyReportNullPointers 0
#endif

#ifndef kTraceyDefineMemoryOperators
#   define kTraceyDefineMemoryOperators 1
#endif

#ifndef kTraceyReportOnExit
#   define kTraceyReportOnExit 1
#endif

#ifndef kTraceyEnabledOnStart
#   define kTraceyEnabledOnStart 1
#endif

namespace tracey
{
    namespace
    {
        void show_report(const std::string& header, const std::string& body)
        {
            kTraceyPrint(header);
            kTraceyPrint(body);
        }

        void show_report(const std::string& header, const std::string& body, const std::string& footer)
        {
            kTraceyPrint(header);
            kTraceyPrint(body);
            kTraceyPrint(footer);
        }

        volatile size_t timestamp_id = 0;

        size_t create_id()
        {
            static size_t id = 0;
            return ++id;
        }

        struct leak
        {
            size_t size, id;

            leak(const size_t _size = 0) : size(_size), id(create_id())
            {}
        };

        typedef std::pair< leak*, tracey::callstack* > leak_full;

        volatile bool enabled = kTraceyEnabledOnStart;

        void* trace(void* ptr, const size_t& size)
        {
            //* Implementation details:
            //* - mutex is used to discard any coming recursive allocation call
            //* - mutex is constructed thru std::malloc+placement-new to avoid infinite recursion
            return ptr;
            static volatile bool initialized = false;
            static tracey::mutex* mutex = 0;

            if (!ptr)
            {
                return ptr;
            }

            if (!mutex)
            {
                static char placement[ sizeof(tracey::mutex) ];
                mutex = (tracey::mutex*)placement;
                new(mutex) tracey::mutex();     // recursion safe; we don't track placement-news
                initialized = true;
            }

            if (!initialized)
            {
                return ptr;
            }

            if (!enabled)
            {
                return ptr;
            }

            if (mutex->is_locked_by_me())
            {
                return ptr;
            }

            mutex->lock();

            {
                static
                class container : public std::map< const void*, leak_full, std::less< const void* > >   //, tracey::malloc_allocator< std::pair< const void *, leak * > > >
                {
                    public:

                        container()
                        {}

                        ~container()
                        {
                            enabled = false;

                            if (kTraceyReportOnExit)
                            {
                                _report();
                            }

                            _clear();
                        }

                        void _clear()
                        {
                            for (iterator it = this->begin(), end = this->end(); it != end; ++it)
                            {
                                if (it->second.first)
                                {
                                    delete it->second.first, it->second.first = 0;
                                }

                                if (it->second.second)
                                {
                                    delete it->second.second, it->second.second = 0;
                                }
                            }

                            this->clear();
                        }

                        void _report() const
                        {
                            // this should happen at the very end of a program (even *after* static memory unallocation)
                            // @todo: avoid using any global object like std::cout/cerr (because some implementations like "cl /MT" will crash)

                            // count number of leaks, filter them and sort them.

                            struct tuple
                            {
                                const void* addr;
                                leak* lk;
                                tracey::callstack* cs;
                            };

                            typedef std::map< size_t, tuple > map;
                            map sort_by_id;

                            size_t n_leak = 0, wasted = 0;
                            for (const_iterator it = this->begin(), end = this->end(); it != end; ++it)
                            {
                                const void* my_address = it->first;
                                leak* my_leak = it->second.first;
                                tracey::callstack* my_callstack = it->second.second;

                                if (my_leak && my_callstack /* && it->second->size != ~0 */)
                                {
                                    if (my_leak->id >= timestamp_id)
                                    {
                                        ++n_leak;
                                        wasted += my_leak->size;

                                        tuple t = { my_address, my_leak, my_callstack };

                                        (sort_by_id[ my_leak->id ] = sort_by_id[ my_leak->id ]) = t;
                                    }
                                }
                            }

                            // show score

                            double leaks_pct = this->size() ? n_leak * 100.0 / this->size() : 0.0;
                            std::string score = "perfect!";
                            if (leaks_pct >  0.00)
                            {
                                score = "excellent";
                            }
                            if (leaks_pct >  1.25)
                            {
                                score = "good";
                            }
                            if (leaks_pct >  2.50)
                            {
                                score = "poor";
                            }
                            if (leaks_pct >  5.00)
                            {
                                score = "mediocre";
                            }
                            if (leaks_pct > 10.00)
                            {
                                score = "lame";
                            }

                            tracey::string header, body, footer;

                            header = tracey::string("<tracey/tracey.cpp> says: Beginning of report. \1, \2 leaks found; \3 bytes wasted ('\4' score)\r\n", !n_leak ? "Ok" : "Error", n_leak, wasted, score);

                            kTraceyPrint(header);

                            // inspect leaks (may take a while)
                            // @todo: shrink report by skipping ending lines in stacktrace which executed before main() or WinMain()
                            //        callstack->str( ..., skip_end = any.stacktrace.find_first(WinMain()) || .find_first( main() ) )

                            size_t ibegin = 0, iend = sort_by_id.size(), percent = ~0, current = 0;
                            for (map::const_iterator it = sort_by_id.begin(), end = sort_by_id.end(); it != end; ++it)
                            {
                                const void* my_address = it->second.addr;
                                leak* my_leak = it->second.lk;
                                tracey::callstack* my_callstack = it->second.cs;

                                current = (size_t)(++ibegin * 100.0 / iend);

                                /* @todo: move this to an user-defined callback { */

                                tracey::string  line("\1) Leak \2 bytes [\3] backtrace \4/\5 (\6%)\r\n", ibegin, my_leak->size, my_address, ibegin, iend, percent = current);
                                tracey::strings lines = tracey::string(my_callstack->str("\2\n", 4)).tokenize("\n");

                                for (size_t i = 0; i < lines.size(); ++i)
                                {
                                    line += tracey::string("\t\1\r\n", lines[i]);
                                }

                                kTraceyPrint(line);

                                /* }: @todo */

                                body += line;
                            }

                            footer = tracey::string("<tracey/tracey.cpp> says: End of report. \1, \2 leaks found; \3 bytes wasted ('\4' score)\r\n", !n_leak ? "Ok" : "Error", n_leak, wasted, score);

                            kTraceyPrint(footer);

                            // body report (linux & macosx will display windows line endings right)

                            sort_by_id.clear();

                            // show_report( header, body, footer );
                        }
                } map;

                container::iterator it = map.find(ptr);
                bool found = (it != map.end());

                if (size == ~0)
                {
                    if (found)
                    {
                        leak_full& map_ptr = it->second;

                        //map_ptr->~leak();
                        //kTraceyFree( map_ptr );

                        if (map_ptr.first)
                        {
                            delete map_ptr.first, map_ptr.first = 0;
                        }

                        if (map_ptr.second)
                        {
                            delete map_ptr.second, map_ptr.second = 0;
                        }
                    }
                    else
                    {
                        // 1st) wild/null pointer deallocation found; warn user

                        if (kTraceyReportWildPointers && ptr)
                        {
                            show_report(tracey::string("<tracey/tracey.cpp> says: Error, wild pointer deallocation.\r\n"), tracey::callstack().str("\t\1\r\n", 4));
                        }

                        if (kTraceyReportNullPointers && !ptr)
                        {
                            show_report(tracey::string("<tracey/tracey.cpp> says: Error, null pointer deallocation.\r\n"), tracey::callstack().str("\t\1\r\n", 4));
                        }

                        // 2nd) normalize ptr for further deallocation (deallocating null pointers is ok)
                        ptr = 0;
                    }
                }
                else if (size == (~0) - 1)
                {
                    map._report();
                }
                else if (size == (~0) - 2)
                {
                    map._clear();
                    timestamp_id = create_id();
                }
                else
                {
                    if (found /* && map[ ptr ] */)
                    {
                        // 1st) double pointer allocation (why?); warn user
                        // kTraceyFree( map[ ptr ] );
                    }

                    // create a leak and (re)insert it into map

                    map[ ptr ] = map[ ptr ];
                    map[ ptr ] = std::make_pair< leak*, tracey::callstack* >(new leak(size), new tracey::callstack());
                }

                mutex->unlock();
            }

            return ptr;
        }
    }

    namespace tracey
    {
        void watch(const void* ptr, size_t size)
        {
            trace((void*)ptr, size);
        }
        void forget(const void* ptr)
        {
            trace((void*)ptr, ~0);
        }
        void enable(bool on)
        {
            enabled = on;
        }
        void disable()
        {
            enabled = false;
        }
        bool is_enabled()
        {
            return enabled;
        }
        void invalidate()
        {
            char unused;
            trace(&unused, (~0) - 2);
        }
        void report()
        {
            char unused;
            trace(&unused, (~0) - 1);
        }
        const char* version()
        {
            return "0.1-a";
        }
    }
}

#if kTraceyDefineMemoryOperators

//* Custom memory operators

void* operator new(size_t size, const std::nothrow_t& t) throw()
{
    void* ptr = tracey::trace(kTraceyAlloc((size_t)(size * kTraceyAllocMultiplier)), size);

    kTraceyAssert(ptr);
    return ptr;
}

void* operator new[](size_t size, const std::nothrow_t& t) throw()
{
    void* ptr = tracey::trace(kTraceyAlloc((size_t)(size * kTraceyAllocMultiplier)), size);

    kTraceyAssert(ptr);
    return ptr;
}

void operator delete(void* ptr, const std::nothrow_t& t) throw()
{
    if (ptr)
    {
        kTraceyFree(tracey::trace(ptr, ~0));
    }
}

void operator delete[](void* ptr, const std::nothrow_t& t) throw()
{
    if (ptr)
    {
        kTraceyFree(tracey::trace(ptr, ~0));
    }
}

//* Custom memory operators (w/ exceptions)
/* Note: original version of this had the throw commented out, but it was giving a warning.  Maybe
         bad idea to uncomment?  Then again, we're not using this memory checking stuff. */

void* operator new(size_t size) throw(std::bad_alloc)
{
    void* ptr = kTraceyAlloc((size_t)(size * kTraceyAllocMultiplier));

    if (!ptr)
    {
        throw kTraceyBadAlloc();
    }

    return tracey::trace(ptr, size);
}

void* operator new[](size_t size) throw(std::bad_alloc)
{
    void* ptr = kTraceyAlloc((size_t)(size * kTraceyAllocMultiplier));

    if (!ptr)
    {
        throw kTraceyBadAlloc();
    }

    return tracey::trace(ptr, size);
}

void operator delete(void* ptr) throw()
{
    if (ptr)
    {
        kTraceyFree(tracey::trace(ptr, ~0));
    }
}

void operator delete[](void* ptr) throw()
{
    if (ptr)
    {
        kTraceyFree(tracey::trace(ptr, ~0));
    }
}

#endif

#undef kTraceyAllocMultiplier
#undef kTraceyAlloc
#undef kTraceyFree
#undef kTraceyPrint
#undef kTraceyAssert
#undef kTraceyBadAlloc

#undef kTraceyDefineMemoryOperators
#undef kTraceyReportWildPointers
#undef kTraceyReportNullPointers
#undef kTraceyReportOnExit
#undef kTraceyEnabledOnStart

#undef $melse
#undef $msvc
#undef $gelse
#undef $gnuc
#undef $release
#undef $debug

#undef $lelse
#undef $linux
#undef $aelse
#undef $apple
#undef $welse
#undef $windows

#undef $yes
#undef $no
#endif
#endif
