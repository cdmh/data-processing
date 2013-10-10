// Copyright (c) 2013 Craig Henderson
// Part of the Data Processing Library
// https://github.com/cdmh/dataproc

#pragma once

namespace cdmh {
namespace data_processing {

/*
    dataset::column_data member functions
*/
inline dataset::column_data::column_data(dataset &ds, size_t column)
  : ds_(ds), column_(column)
{
}

inline dataset::column_data::column_data(dataset &ds, char const *name)
  : ds_(ds), column_(ds_.lookup_column(name))
{
}

inline dataset::column_data::column_data(column_data const &other)
  : ds_(other.ds_), column_(other.column_)
{
}

inline void dataset::column_data::clear()
{
    ds_.clear_column(column_);
}

template<typename T>
inline
std::vector<T> dataset::column_data::detach()
{
    return ds_.detach_column<T>(column_);
}

inline void dataset::column_data::erase()
{
    return ds_.erase_column(column_);
}

template<typename T>
inline
std::vector<T> dataset::column_data::extract()
{
    return ds_.extract_column<T>(column_);
}

inline void dataset::column_data::swap(size_t column)
{
    return ds_.swap_columns(column_, column);
}

inline size_t const dataset::column_data::size() const
{
    return ds_.at(column_).size();
}

// returns the number of non-null values in the column
inline size_t const dataset::column_data::count() const
{
    auto const &values = ds_.at(column_);
    return std::count_if(
        values.begin(),
        values.end(),
        [](cell_value const &cell) {
            return !cell.is_null();
        });
}

// returns the number of null values in the column
inline size_t const dataset::column_data::count_null() const
{
    auto const &values = ds_.at(column_);
    return std::count_if(
        values.begin(),
        values.end(),
        [](cell_value const &cell) {
            return cell.is_null();
        });
}

inline size_t const dataset::column_data::count_unique() const
{
    assert(ds_.column_type(column_) == integer_type  ||  ds_.column_type(column_) == double_type);

    if (ds_.column_type(column_) == double_type)
        return count_unique<double>();
    return count_unique<std::uint32_t>();
}

template<typename T>
inline size_t const dataset::column_data::count_unique() const
{
    std::unordered_map<T, unsigned> counts;
    for (auto const &value : ds_.at(column_))
        if (!value.is_null())
            counts[value.get<T>()]++;
    return counts.size();
}

inline double const dataset::column_data::mean() const
{
    assert(ds_.column_type(column_) == integer_type  ||  ds_.column_type(column_) == double_type);

    if (ds_.column_type(column_) == double_type)
        return sum<double>() / count();
    return (double)sum<std::uint32_t>() / count();
}

inline double const dataset::column_data::median() const
{
    assert(ds_.column_type(column_) == integer_type  ||  ds_.column_type(column_) == double_type);

    if (ds_.column_type(column_) == double_type)
        return maths::median(ds_.extract_column<double>(column_, false));
    return (double)maths::median(ds_.extract_column<std::uint32_t>(column_, false));
}

inline double const dataset::column_data::mode() const
{
    assert(ds_.column_type(column_) == integer_type  ||  ds_.column_type(column_) == double_type);

    if (ds_.column_type(column_) == double_type)
        return maths::mode(ds_.extract_column<double>(column_, false));
    return (double)maths::mode(ds_.extract_column<std::uint32_t>(column_, false));
}

inline double const dataset::column_data::standard_deviation() const
{
    assert(ds_.column_type(column_) == integer_type  ||  ds_.column_type(column_) == double_type);

    if (ds_.column_type(column_) == double_type)
        return maths::standard_deviation(ds_.extract_column<double>(column_, false));
    return maths::standard_deviation(ds_.extract_column<std::uint32_t>(column_, false));
}

template<typename T>
inline T dataset::column_data::max() const
{
    assert(ds_.column_type(column_) == integer_type  ||  ds_.column_type(column_) == double_type);

    T max = std::numeric_limits<T>::min();
    for (auto const &value : ds_.at(column_))
    {
        if (!value.is_null())
        {
            T val = value.get<T>();
            if (val > max)
                max = val;
        }
    }
    return max;
}

template<typename T>
inline T dataset::column_data::min() const
{
    assert(ds_.column_type(column_) == integer_type  ||  ds_.column_type(column_) == double_type);

    T min = std::numeric_limits<T>::max();
    for (auto const &value : ds_.at(column_))
    {
        if (!value.is_null())
        {
            T val = value.get<T>();
            if (val < min)
                min = val;
        }
    }
    return min;
}

template<typename T>
inline T const dataset::column_data::sum() const
{
    auto const &values = ds_.at(column_);
    return std::accumulate(
        values.begin(),
        values.end(),
        T(),
        [](T sum, cell_value const &cell) {
            return sum + cell.get<T>();
        });
}

inline dataset::column_data dataset::column(int n)
{
    return column((size_t)n);
}

inline dataset::column_data dataset::column(size_t column)
{
    return column_data(*this, column);
}

inline dataset::column_data dataset::column(char const *name)
{
    return column_data(*this, name);
}

}   // namespace data_processing
}   // namespace cdmh
