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

inline
unsigned const bit_count(std::uint32_t n)
{
    n = n - ((n >> 1) & 0x55555555);
    n = (n & 0x33333333) + ((n >> 2) & 0x33333333);
    return (((n + (n >> 4)) & 0x0F0F0F0F) * 0x01010101) >> 24;
}

inline bool const single_bit_set(std::uint32_t n)
{
    return (!n && (!(n & (n-1))));
}

unsigned const bit_number(std::uint32_t n)
{
    assert(single_bit_set(n));

    std::uint32_t i = 1, pos = 1;
    while (!(i & n))
    {
        i <<= 1;
        ++pos;
    }
 
    return pos;
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

class dataset
{
  private:
    typedef union value
    {
        double        double_;
        std::uint32_t integer_;
        char const   *string_;

        value(double dbl)            : double_(dbl)      { }
        value(std::uint32_t integer) : integer_(integer) { }
        value(char const *string)    : string_(string)   { }
    } value_t;

  public:
    explicit dataset(std::uint32_t num_columns = 0)
    {
        if (num_columns > 0)
            columns_.reserve(num_columns);
    }

    dataset(dataset &&other)
    {
        assert_valid();
        std::swap(columns_, other.columns_);
    }

    // defaults are not safe because of dynamic memory allocation in the union
    dataset(dataset const &other)            = delete;
    dataset &operator=(dataset &&other)      = delete;
    dataset &operator=(dataset const &other) = delete;

    ~dataset()
    {
        assert_valid();
        for (auto &column : columns_)
            if (column.first == string_type)
                for (auto &value : column.second)
                    delete[] value.string_;
    }

    std::function<void (std::pair<char const *, char const *> value)>
    create_column(std::uint32_t type)
    {
        columns_.push_back(std::make_pair(type, std::vector<value_t>()));

        if (type == string_type)
            return std::bind(&dataset::add_column_string_data, this, columns_.size()-1, std::placeholders::_1);
        else if (type == double_type)
            return std::bind(&dataset::add_column_double_data, this, columns_.size()-1, std::placeholders::_1);

        assert(type == integer_type);
        return std::bind(&dataset::add_column_integer_data, this, columns_.size()-1, std::placeholders::_1);
    }

    size_t const columns(void) const
    {
        return columns_.size();
    }

    size_t const rows(void) const
    {
        assert_valid();
        return columns_.size()==0? 0 : columns_[0].second.size();
    }

    std::function<char const * (size_t)>
    operator[](size_t row) const
    {
        return std::bind(&dataset::at, this, row, std::placeholders::_1);
    }

    char const *at(size_t row, size_t column) const
    {
        return columns_[column].second[row].string_;
    }

  private:
    void add_column_string_data(std::uint32_t index, std::pair<char const *,char const *> value)
    {
        auto length = std::distance(value.first, value.second);
        char *string = new char[length+1];
        strncpy(string, value.first, length);
        string[length] = 0;;
        columns_[index].second.push_back(string);
    }

    void add_column_double_data(std::uint32_t index, std::pair<char const *, char const *> value)
    {
        double d = strtod(value.first, nullptr);
        columns_[index].second.push_back(d);
    }

    void add_column_integer_data(std::uint32_t index, std::pair<char const *, char const *> value)
    {
        std::uint32_t n = atol(value.first);
        columns_[index].second.push_back(n);
    }

    void assert_valid(void) const
    {
#ifndef NDEBUG
        if (columns_.size() > 0)
        {
            auto const size = columns_[0].second.size();
            for (size_t loop=1; loop<columns_.size(); ++loop)
                assert(columns_[loop].second.size() == size);
        }
#endif
    }

  private:
    std::vector<
        std::pair<
            std::uint32_t,          // column type
            std::vector<value_t>    // values
        >
    > columns_;
};

class mapped_csv
{
  public:
    mapped_csv(char const * const filename)
      : file_(filename, readonly),
        mmf_(file_, readonly)
    {
    }

    void read(std::uint64_t max_records=0)
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
        while (it != ite  &&  (max_records == 0  ||  size() < max_records))
        {
            auto eol = std::find(it,ite,'\r');
            process_record(it, eol, store);
            assert(it == eol);
            detail::ltrim(it, ite);
            store = store_fields;
        }
    }

    dataset create_dataset(bool destructive=true)
    {
        dataset ds(column_info_.size());
        for (unsigned index=0; index<column_info_.size(); ++index)
        {
            auto inserter = ds.create_column(column_info_[index].second);
            for (auto value : column_values_[index])
                inserter(value);

            if (destructive)
                string_list_t().swap(column_values_[index]);
        }

        if (destructive)
        {
            record_count_ = 0;
            std::vector<column_info_t>().swap(column_info_);
            std::vector<std::uint32_t>().swap(incl_type_mask_);
            std::vector<string_list_t>().swap(column_values_);
            file_.close();
            mmf_.release();
        }

        return ds;
    }

    std::uint64_t const size(void) { return record_count_; }


  protected:
    void store_field(unsigned index, std::pair<char const *, char const *> &value, std::uint32_t type)
    {
        assert(index < column_info_.size());

        if (index == 0)
            ++record_count_;

        if (type != null_type)
        {
            // if the column type doesn't match the new type, then
            // the column type will be a string
            if (column_info_[index].second == 0)
                column_info_[index].second = type;
            else if (column_info_[index].second != type)
                column_info_[index].second = string_type;
        }

        column_values_[index].push_back(value);
        assert(column_info_.size() == column_values_.size());
    }

    void create_column(unsigned /*index*/, std::pair<char const *, char const *> &name, std::uint32_t /*type*/)
    {
        assert(index == column_info_.size());
        column_info_.push_back(column_info_t(name,0));
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
    typedef std::pair<char const *, char const *> string_t;
    typedef std::vector<string_t>                 string_list_t;
    typedef std::pair<string_t, std::uint32_t>    column_info_t;

    file<char>                 file_;
    memory_mapped_file<char>   mmf_;
    std::uint64_t              record_count_;
    std::vector<column_info_t> column_info_;
    std::vector<std::uint32_t> incl_type_mask_;
    std::vector<string_list_t> column_values_;
};

}   // namespace data_processing
}   // namespace cdmh
