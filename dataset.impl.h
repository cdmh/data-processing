// Copyright (c) 2013 Craig Henderson
// Part of the Data Processing Library
// https://github.com/cdmh/dataproc

namespace cdmh {

namespace data_processing {

inline std::vector<dataset::cell_value> const &dataset::cells(size_t column) const
{
    return column_values_[column];
}

template<typename T>
inline
T dataset::at(size_t row, size_t column) const
{
    return cell(row, column).get<T>();
}

inline
dataset::cell_value const &dataset::cell(size_t row, size_t column) const
{
    return column_values_[column][row];
}

inline size_t const dataset::columns() const
{
    return column_values_.size();
}

inline void dataset::erase_column(size_t column)
{
    column_values_.erase(column_values_.begin() + column);
}

inline bool const dataset::row_data::cell_reference::is_null() const
{
    return dd_.cell(row_, column_).is_null();
}

inline dataset::row_data dataset::row(size_t row) const
{
    return row_data(*this, row);
}

inline dataset::row_data dataset::operator[](size_t n) const
{
    return row(n);
}

inline size_t const dataset::rows() const
{
    if (columns() == 0)
        return 0;

#ifndef NDEBUG
    for (size_t loop=1; loop<column_values_.size(); ++loop)
        assert(column_values_[loop].size() == column_values_[0].size());
#endif
    return column_values_[0].size();
}

inline type_mask_t const dataset::column_type(size_t column) const
{
    return column_info_[column].second;
}

template<typename T>
inline std::vector<T> dataset::extract_column(size_t column, bool include_nulls) const
{
    std::vector<T> result;
    result.reserve(column_values_[column].size());
    for (auto &value : column_values_[column])
        if (include_nulls  ||  !value.is_null())
            result.push_back(value.get<T>());
    return result;
}

inline size_t const dataset::lookup_column(char const *name) const
{
    size_t index = 0;
    for (auto const &column : column_info_)
    {
        if (column.first == name)
            return index;
        ++index;
    }
    throw invalid_column_name();
}





inline dataset::cell_value::cell_value(string_view string)
  : string_(string)
{
}

template<>
inline
double dataset::cell_value::get() const
{
    return strtod(string_.begin(), nullptr);
}

template<>
inline
std::uint32_t dataset::cell_value::get() const
{
    return strtol(string_.begin(), nullptr, 10);
}

template<>
inline
string_view dataset::cell_value::get() const
{
    return string_;
}

template<>
inline
std::string dataset::cell_value::get() const
{
    return std::string(string_.begin(), string_.end());
}









namespace { // anonymous namespace

template<typename It>
inline
It find_eol(It it, It ite)
{
    while (it != ite)
    {
        detail::read_field(it, ite);
        if (it == ite  ||  *it == '\r'  ||  *it == '\n')
            return it;
        assert(*it == ',');
        ++it;
    }
    return it;
}

}   // anonymous namespace

template<typename It>
inline
bool const dataset::attach(It begin, It end, std::uint64_t max_records)
{
    typedef 
    std::function<void (unsigned, string_view &, type_mask_t)>
    store_fn_t;

    using std::placeholders::_1;
    using std::placeholders::_2;
    using std::placeholders::_3;
    store_fn_t store         = std::bind(&dataset::create_column, this, _1, _2, _3);
    store_fn_t store_fields  = std::bind(&dataset::store_field, this, _1, _2, _3);

    while (begin != end  &&  (max_records == 0  ||  rows() < max_records))
    {
        auto eol = find_eol(detail::ltrim(begin, end), end);
        if (begin != end)
        {
            process_record(begin, eol, store);
            assert(begin == eol);
            store = store_fields;
        }
    }

    return true;
}

inline bool const dataset::attach(char const *data, std::uint64_t max_records)
{
    return attach(data, data+strlen(data), max_records);
}


inline void dataset::create_column(unsigned index, string_view const &name, type_mask_t /*type*/)
{
#ifdef NDEBUG
    index;
#else
    assert(index == column_info_.size());
#endif
    column_info_.push_back(column_info_t(name, 0));
    column_values_.push_back(string_list_t());
}

template<typename It, typename Fn>
inline
bool const dataset::process_record(It &begin, It end, Fn fn)
{
    for (unsigned index=0; begin!=end; ++index)
    {
        auto field = detail::read_field(begin, end);
        fn(index, field.first, field.second);
        if (begin!=end)
        {
            assert(*begin == ',');
            ++begin;
        }
    }

    return true;
}

inline void dataset::store_field(unsigned index, string_view const &value, type_mask_t type)
{
    assert(index < column_info_.size());

    if (type != null_type)
    {
        // if the column type doesn't match the new type, then
        // the column type will be a string
        if (column_info_[index].second == 0)
            column_info_[index].second = type;
        else if (column_info_[index].second != type)
            column_info_[index].second = string_type;
    }

    column_values_[index].emplace_back(value);
    assert(column_info_.size() == column_values_.size());
}



/*
    serialization free functions
*/

template<typename E, typename T>
inline
std::basic_ostream<E, T> &operator<<(std::basic_ostream<E,T> &o, dataset::row_data::cell_reference const &value)
{
    if (!value.is_null())
    {
        switch (value.type())
        {
            case string_type:   o << value.get<std::string>();      break;  /// !!! escape " quote with "" 
            case double_type:   o << value.get<double>();           break;
            case integer_type:  o << value.get<std::uint32_t>();    break;
            default:            assert(!"Unknown value type");
        }
    }

    return o;
}

template<typename E, typename T>
inline
std::basic_ostream<E,T> &operator<<(std::basic_ostream<E,T> &o, dataset::row_data const &row)
{
    bool first = true;
    for (size_t loop=0; loop<row.size(); ++loop)
    {
        if (first)
            first = false;
        else
            o << ',';

        auto const &value = row[loop];
        if (value.type() == string_type)
            o << '\"' << value << '\"';
        else
            o << value;
    }
    return o;
}

template<typename E, typename T>
inline
std::basic_ostream<E,T> &operator<<(std::basic_ostream<E,T> &o, dataset const &dd)
{
    bool first = true;
    for (auto const &column : dd.column_info_)
    {
        if (first)
            first = false;
        else
            o << ',';
        o << '\"' << column.first << '\"';
    }
    o << "\n";

    for (size_t loop=0; loop<dd.rows(); ++loop)
        o << dd[loop] << "\n";
    return o;
}

}   // namespace data_processing
}   // namespace cdmh
