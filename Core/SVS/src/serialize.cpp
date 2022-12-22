#include <cstdio>
#include <cstring>
#include "common.h"
#include "serialize.h"

void serialize(const serializable& v, std::ostream& os)
{
    v.serialize(os);
}

void unserialize(serializable& v, std::istream& is)
{
    v.unserialize(is);
}

void serialize(char c, std::ostream& os)
{
    if (!os.put(c))
    {
        assert(false);
    }
}

void unserialize(char& c, std::istream& is)
{
    if (!(is >> c))
    {
        assert(false);
    }
}

void serialize(bool b, std::ostream& os)
{
    os << (b ? 't' : 'f');
}

void unserialize(bool& b, std::istream& is)
{
    char c;
    if (!(is >> c))
    {
        assert(false);
    }
    assert(c == 't' || c == 'f');
    b = (c == 't');
}

void serialize(int v, std::ostream& os)
{
    os << v;
}

void serialize(long v, std::ostream& os)
{
    os << v;
}

void serialize(size_t v, std::ostream& os)
{
    os << v;
}

void unserialize(int& v, std::istream& is)
{
    std::string buf;
    if (!(is >> buf))
    {
        assert(false);
    }
    if (!parse_int(buf, v))
    {
        assert(false);
    }
}

/*
 It's impossible to get an exact decimal representation of floating point
 numbers, so I use hexadecimal floating point representation instead. This
 gives up on readability but prevents rounding errors in the
 serialize/unserialize cycle.
*/
void serialize(double v, std::ostream& os)
{
    static const size_t buf_len = 100;
    static char buf[buf_len];

    int res = SNPRINTF(buf, buf_len, "%a", v);
    if (res >= buf_len)
    {
        std::cerr << "buffer overflow when serializing a double" << std::endl;
        assert(false);
    }
    os << buf;
}

/*
 The stream operator >> doesn't recognize hex float format, so use strtod
 instead.
*/
void unserialize(double& v, std::istream& is)
{
    std::string buf;

    if (!(is >> buf) || !parse_double(buf, v))
    {
        assert(false);
    }
}

/*
 Puts std::string in " ". Represent literal "'s with ""
*/
void serialize(const char* s, std::ostream& os)
{
    bool need_quotes = false;

    if (strlen(s) == 0)
    {
        need_quotes = true;
    }
    else
    {
        for (const char* p = s; *p; ++p)
        {
            if (*p == '"' || isspace(*p))
            {
                need_quotes = true;
            }
        }
    }

    if (need_quotes)
    {
        os << '"';
    }
    for (const char* p = s; *p; ++p)
    {
        if (*p == '"')
        {
            os << "\"\"";
        }
        else
        {
            os << *p;
        }
    }
    if (need_quotes)
    {
        os << '"';
    }
}

void serialize(const std::string& s, std::ostream& os)
{
    serialize(s.c_str(), os);
}

void unserialize(std::string& s, std::istream& is)
{
    char c;
    std::stringstream ss;
    bool quoted = false;

    while (is.get(c) && isspace(c))
        ;;

    assert(is);

    if (c == '"')
    {
        quoted = true;
    }
    else
    {
        ss << c;
    }

    while (is.get(c))
    {
        if ((quoted && c == '"' && is.get() != '"') ||
                (!quoted && isspace(c)))
        {
            is.unget();
            break;
        }
        ss << c;
    }
    if (quoted)
    {
        assert(c == '"');
    }
    s = ss.str();
}

serializer& serializer::operator<<(char c)
{
    if (isspace(c))
    {
        os.put(c);
        delim = true;
    }
    else
    {
        if (!delim)
        {
            os.put(' ');
        }
        ::serialize(c, os);
        delim = false;
    }
    return *this;
}
