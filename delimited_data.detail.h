// Copyright (c) 2013 Craig Henderson
// Part of the Data Processing Library
// https://github.com/cdmh/dataproc

namespace cdmh {
namespace data_processing {
namespace detail {

inline bool const isspace(char const ch)
{
    return ch == ' '  ||  ch == '\t'  ||  ch == '\r'  ||  ch == '\n';
}

template<typename It>
inline
void ltrim(It &it,It ite)
{
    while (it != ite  &&  isspace(*it))
        ++it;
}

template<typename It>
inline
void rtrim(It it,It &ite)
{
    std::reverse_iterator<It> rit(ite);
    std::reverse_iterator<It> rite(it);
    ltrim(rit, rite);
    ite = rit.base();
}

template<typename It>
inline
std::pair<std::pair<It,It>, type_mask_t>&
trim(std::pair<std::pair<It,It>, type_mask_t> &src)
{
    ltrim(src.first.first, src.first.second);
    rtrim(src.first.first, src.first.second);
    return src;
}

}   // namespace detail
}   // namespace data_processing
}   // namespace cdmh
