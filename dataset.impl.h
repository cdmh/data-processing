// Copyright (c) 2013 Craig Henderson
// Part of the Data Processing Library
// https://github.com/cdmh/dataproc

#include <algorithm>        // count_if
#include <numeric>          // accumulate
#include "maths.h"

#include "dataset.cell_value.h"
#include "dataset.column_data.h"
#include "dataset.row_data.h"

namespace cdmh {
namespace data_processing {

class invalid_column_name : public std::runtime_error
{
  public:
    invalid_column_name() : std::runtime_error("Invalid column name")
    { }
};


/*
  dataset member functions
*/
inline dataset::dataset(size_t num_columns)
{
    if (num_columns > 0)
        columns_.reserve(num_columns);
}

inline dataset::dataset(dataset &&other) noexcept
{
    assert_valid();
    std::swap(columns_, other.columns_);
}

inline dataset &dataset::operator=(dataset &&other) noexcept
{
    std::swap(columns_, other.columns_);
    return *this;
}

template<typename T>
inline
T dataset::at(size_t row, int column) const
{
    return columns_[column].values[row].get<T>();
}

inline std::vector<dataset::cell_value> const &dataset::at(int column) const
{
    return columns_[column].values;
}

inline
std::function<void (std::pair<string_view, type_mask_t>)>
dataset::create_column(type_mask_t type, std::string const &name)
{
    columns_.push_back(column_info{type, name, std::vector<cell_value>()});

    if (type == string_type)
        return std::bind(&dataset::add_column_string_data, this, columns_.size()-1, std::placeholders::_1);
    else if (type == double_type)
        return std::bind(&dataset::add_column_double_data, this, columns_.size()-1, std::placeholders::_1);

    assert(type == integer_type);
    return std::bind(&dataset::add_column_integer_data, this, columns_.size()-1, std::placeholders::_1);
}

inline size_t const dataset::columns() const
{
    return columns_.size();
}

inline type_mask_t const dataset::column_type(int column) const
{
    return columns_[column].type;
}

inline void dataset::clear_column(int column)
{
    for (auto &value : columns_[column].values)
        value.clear();
}

template<typename T>
inline std::vector<T> dataset::extract_column(int column, bool include_nulls)
{
    std::vector<T> result;
    result.reserve(columns_[column].values.size());
    for (auto &value : columns_[column].values)
        if (include_nulls  ||  !value.is_null())
            result.push_back(value.get<T>());
    return result;
}

template<typename T>
inline std::vector<T> dataset::detach_column(int column)
{
    std::vector<T> result = extract_column<T>(column);
    erase_column(column);
    return result;
}

inline void dataset::erase_column(int column)
{
    columns_.erase(columns_.begin() + column);
}

inline size_t const dataset::lookup_column(char const *name) const
{
    size_t index = 0;
    for (auto const &column : columns_)
    {
        if (column.name == name)
            return index;
        ++index;
    }
    throw invalid_column_name();
}

inline bool const dataset::import_csv(std::string const &filename)
{
    mapped_csv csv(filename);
    if (!csv.read())
        return false;
    *this = csv.create_dataset();
    return true;
}

inline bool const dataset::import_csv(char const *filename)
{
    mapped_csv csv(filename);
    if (!csv.read())
        return false;
    *this = csv.create_dataset();
    return true;
}

inline size_t const dataset::rows() const
{
    assert_valid();
    return columns_.size()==0? 0 : columns_[0].values.size();
}

inline void dataset::swap_columns(int column1, int column2)
{
    std::swap(columns_[column1], columns_[column2]);
}

inline type_mask_t const dataset::type_at(size_t row, int column) const
{
    return columns_[column].values[row].type();
}

inline void dataset::assert_valid() const
{
#ifndef NDEBUG
    if (columns_.size() > 0)
    {
        auto const size = columns_[0].values.size();
        for (size_t loop=1; loop<columns_.size(); ++loop)
            assert(columns_[loop].values.size() == size);
    }
#endif
}

inline void dataset::add_column_string_data(size_t index, std::pair<string_view, type_mask_t> value)
{
    columns_[index].values.emplace_back(value.second, std::string(value.first.begin(), value.first.end()));
}

inline void dataset::add_column_double_data(size_t index, std::pair<string_view, type_mask_t> value)
{
    columns_[index].values.emplace_back(
        value.second,
        strtod(value.first.begin(), nullptr));
}

inline void dataset::add_column_integer_data(size_t index, std::pair<string_view, type_mask_t> value)
{
    columns_[index].values.emplace_back(
        value.second,
        size_t(atol(value.first.begin())));
}

inline dataset::row_data dataset::row(size_t row) const
{
    return row_data(*this, row);
}

inline dataset::row_data dataset::operator[](size_t n) const
{
    return row(n);
}

/*
    serialization free functions
*/

template<typename E, typename T>
inline
std::basic_ostream<E,T> &operator<<(std::basic_ostream<E,T> &o, dataset::row_data::cell const &value)
{
    switch (value.type())
    {
        case string_type:   o << value.get<std::string>();  break;
        case double_type:   o << value.get<double>();       break;
        case integer_type:  o << value.get<size_t>();       break;
        case null_type:                                     break;
        default:            assert(!"Unknown value type");
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
std::basic_ostream<E,T> &operator<<(std::basic_ostream<E,T> &o, dataset const &ds)
{
    bool first = true;
    for (auto const &column : ds.columns_)
    {
        if (first)
            first = false;
        else
            o << ',';
        o << '\"' << column.name << '\"';
    }
    o << "\n";

    for (size_t loop=0; loop<ds.rows(); ++loop)
        o << ds[loop] << "\n";
    return o;
}

}   // namespace data_processing
}   // namespace cdmh
