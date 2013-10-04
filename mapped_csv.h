#include <string>
#include <vector>
#include <functional>       // std::function
#include <fstream>
#include <cstdint>
#include <locale>
#include <cassert>
#include "memmap.h"

namespace cdmh {

namespace data_processing {
static std::uint32_t const string_type  = 1;
static std::uint32_t const double_type  = 1 << 1;
static std::uint32_t const integer_type = 1 << 2;
static std::uint32_t const null_type    = 1 << 3;

namespace detail {

unsigned const bit_count(std::uint32_t i)
{
    i = i - ((i >> 1) & 0x55555555);
    i = (i & 0x33333333) + ((i >> 2) & 0x33333333);
    return (((i + (i >> 4)) & 0x0F0F0F0F) * 0x01010101) >> 24;
}

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
inline
std::pair<std::pair<It, It>, std::uint32_t>
read_field(It &begin, It end)
{
    assert(begin != end);
    using cdmh::data_processing::string_type;
    using cdmh::data_processing::double_type;
    using cdmh::data_processing::integer_type;
    using cdmh::data_processing::null_type;

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

class dataset {};

class mapped_csv
{
  public:
    mapped_csv(char const * const filename)
      : file_(filename, readonly),
        mmf_(file_, readonly)
    {
    }

    dataset read(void)
    {
        typedef 
        std::function<void (unsigned, std::pair<char const *, char const *> &, std::uint32_t)>
        store_fn_t;

        using std::placeholders::_1;
        using std::placeholders::_2;
        using std::placeholders::_3;
        store_fn_t store         = std::bind(&mapped_csv::create_column, this, _1, _2, _3);
        store_fn_t store_fields  = std::bind(&mapped_csv::store_field, this, _1, _2, _3);

        char const *it = mmf_.get();
        char const *ite = it + file_.size();

        record_count_ = 0;
        while (it != ite)
        {
            auto eol = std::find(it,ite,'\r');
            process_record(it, eol, store);
            assert(it == eol);
            detail::ltrim(it, ite);
            store = store_fields;
        }
        return dataset_;
    }

    std::uint64_t const size(void) { return record_count_; }


  protected:
    void store_field(unsigned index, std::pair<char const *, char const *> &value, std::uint32_t type)
    {
        assert(index < incl_type_mask_.size());

        if (index == 0)
            ++record_count_;

        if (type != null_type)
        {
            // if the column type doesn't match the new type, then
            // the column type will be a string
            if (incl_type_mask_[index] == 0)
                incl_type_mask_[index] = type;
            else if (incl_type_mask_[index] != type)
                incl_type_mask_[index] = string_type;
        }

        column_values_[index].push_back(value);
    }

    void create_column(unsigned index, std::pair<char const *, char const *> &name, std::uint32_t type)
    {
        assert(index == column_names_.size());
        column_names_.push_back(name);
        incl_type_mask_.push_back(0);
        column_values_.push_back(string_list_t());
    }

    template<typename It, typename Fn>
    bool const process_record(It &begin, It end, Fn fn)
    {
        for (unsigned index=0; begin!=end; ++index)
        {
            auto field = detail::read_field(begin, end);
            fn(index, field.first, field.second);
        }

        return true;
    }

  private:
    file<char>               file_;
    memory_mapped_file<char> mmf_;
    std::uint64_t            record_count_;

    typedef std::pair<char const *, char const *> string_t;
    typedef std::vector<string_t>                 string_list_t;

    string_list_t              column_names_;
    std::vector<std::uint32_t> incl_type_mask_;
    std::vector<string_list_t> column_values_;
    dataset dataset_;
};

}   // namespace data_processing
}   // namespace cdmh
