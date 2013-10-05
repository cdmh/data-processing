namespace cdmh {
namespace data_processing {
namespace detail {

inline
unsigned const bit_count(std::uint32_t n)
{
    n = n - ((n >> 1) & 0x55555555);
    n = (n & 0x33333333) + ((n >> 2) & 0x33333333);
    return (((n + (n >> 4)) & 0x0F0F0F0F) * 0x01010101) >> 24;
}

template<std::size_t N, typename... U>
using select_type = typename std::tuple_element<N, std::tuple<U...> >::type;

template<typename It>
inline
void ltrim(It &it,It ite)
{
    while (it != ite  &&  std::isspace(*it, std::locale::classic()))
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
std::pair<std::pair<It,It>,std::uint32_t>&
trim(std::pair<std::pair<It,It>,std::uint32_t> &src)
{
    ltrim(src.first.first, src.first.second);
    rtrim(src.first.first, src.first.second);
    return src;
}

template<typename It>
std::pair<std::pair<It, It>, std::uint32_t>
read_field(It &begin, It end)
{
    assert(begin != end);

    bool in_quote  = false;
    if (*begin == '\"')
    {
        in_quote = true;
        ++begin;
    }

    auto it = begin;
    std::uint32_t incl_type_mask = string_type;
    std::uint32_t excl_type_mask = 0;

    // special case for unary operators
    if (*it == '-'  ||  *it == '+')
    {
        incl_type_mask |= double_type | integer_type;
        ++it;
    }

    bool expect_esc  = false;
    bool seen_period = false;
    for (; it!=end  &&  *it != ','  &&  !(in_quote  &&  *it == '\"'  &&  !expect_esc); ++it)
    {
        if (std::isdigit(*it, std::locale::classic()))
            incl_type_mask |= double_type | integer_type;
        else if (*it == '.')
        {
            if (seen_period)
                excl_type_mask |= double_type;
            else
            {
                incl_type_mask |= double_type;
                excl_type_mask |= integer_type;
                seen_period = true;
            }
        }
        else
        {
            excl_type_mask |= double_type | integer_type;
            if (!expect_esc  &&  *it == '\\')
                expect_esc = true;
            else
                expect_esc = false;
        }
    }

    ltrim(begin, it);
    rtrim(begin, it);
    if (begin == it)
        return std::make_pair(std::pair<It, It>(begin++, it), null_type);

    // precedences
    incl_type_mask &= ~excl_type_mask;
    if (incl_type_mask & integer_type)   // prefer integer to double and string
        incl_type_mask &= ~(double_type | string_type);
    if (incl_type_mask & double_type)    // prefer double to string
        incl_type_mask &= ~string_type;

    assert(detail::bit_count(incl_type_mask) == 1);
    auto result = std::make_pair(std::pair<It, It>(begin, it), incl_type_mask);

    // update returning 'begin' iterator to the start next field
    begin = it;
    if (in_quote)
    {
        assert(*begin == '\"');
        ++begin;
    }

    if (begin != end  &&  *begin == ',')
        ++begin;
    return result;
}

}   // namespace detail
}   // namespace data_processing
}   // namespace cdmh
