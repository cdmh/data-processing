// Copyright (c) 2013 Craig Henderson
// Part of the Data Processing Library
// https://github.com/cdmh/dataproc

#pragma once

#include <cstring>
#include <algorithm>

namespace cdmh {

#ifdef strncasecmp
#undef strncasecmp
#endif

inline int __cdecl strcasecmp(const char *first, const char *second)
{
    int f;
    int s;

    do
    {
        if (((f = *(first++)) >= 'A')  &&  (f <= 'Z'))
            f -= 'A' - 'a';

        if (((s = *(second++)) >= 'A')  &&  (s <= 'Z'))
            s -= 'A' - 'a';
    }
    while (*first  &&  *second  &&  f  &&  (f == s));

    if (!*first  &&  !*second)
        return (f - s);
    else if (*first)
        return 1;
    return -1;
}

inline int __cdecl strncasecmp(const char * first, const char * second, size_t count)
{
    if (!count)
        return 0;

    int f;
    int s;

    do
    {
        if (((f = *(first++)) >= 'A')  &&  (f <= 'Z'))
            f -= 'A' - 'a';

        if (((s = *(second++)) >= 'A')  &&  (s <= 'Z'))
            s -= 'A' - 'a';
    }
    while (--count  &&  f  &&  (f == s));

    return (f - s);
}

namespace data_processing {

// a string type of a string of characters
// represented by a pair of iterators
class string_view
{
  public:
    string_view(char const *begin)
      : begin_(begin), end_(begin + strlen(begin))
    { }

    string_view(char const *begin,char const *end)
      : begin_(begin),end_(end)
    { }

    char const *begin() const { return begin_; }
    char const *end()   const { return end_; }

    size_t const length() const
    {
        return std::distance(begin_, end_);
    }

  private:
    char const *begin_;
    char const *end_;
};

inline
bool const operator==(string_view const &first, char const *second)
{
    auto const len1 = first.length();
    auto const len2 = strlen(second);
    if (len1 != len2)
        return false;

    return (strncasecmp(first.begin(), second, len1) == 0);
}

inline
bool const operator<(string_view const &first, string_view const &second)
{
    auto const len1 = first.length();
    auto const len2 = second.length();
    if (len1 < len2)
    {
        auto cmp = strncasecmp(first.begin(), second.begin(), len1);
        return (cmp <= 0);
    }
    else if (len1 > len2)
    {
        auto cmp = strncasecmp(first.begin(), second.begin(), len2);
        return (cmp < 0);
    }

    return strncasecmp(first.begin(), second.begin(), len1) < 0;
}

bool const operator==(string_view const &first, string_view const &second);

template<typename E, typename T>
inline
std::basic_ostream<E, T> &operator<<(std::basic_ostream<E, T> &o, string_view const &str)
{
    std::copy(str.begin(), str.end(), std::ostream_iterator<char>(o));
    return o;
}


}   // namespace data_processing
}   // namespace cdmh
