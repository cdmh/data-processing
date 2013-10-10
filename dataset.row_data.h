// Copyright (c) 2013 Craig Henderson
// Part of the Data Processing Library
// https://github.com/cdmh/dataproc

#pragma once

namespace cdmh {
namespace data_processing {

/*
    dataset::row_data member functions
*/
inline dataset::row_data::cell::cell(dataset const &ds, size_t row, size_t column)
  : ds_(ds), row_(row),column_(column)
{
}

inline dataset::row_data::row_data(dataset const &ds, size_t row)
  : ds_(ds), row_(row)
{
}

inline dataset::row_data::cell dataset::row_data::operator[](int column) const
{
    return (*this)[(size_t)column];
}

inline dataset::row_data::cell dataset::row_data::operator[](size_t column) const
{
    return cell(ds_, row_, column);
}

inline dataset::row_data::cell dataset::row_data::operator[](char const *name) const
{
    return cell(ds_, row_, ds_.lookup_column(name));
}

inline size_t dataset::row_data::size() const
{
    return ds_.columns();
}

inline dataset::row_data::cell::cell(dataset::row_data::cell &&other) noexcept
  : ds_(other.ds_), row_(other.row_), column_(other.column_)
{
}

inline dataset::row_data::cell &dataset::row_data::cell::operator=(dataset::row_data::cell &&other) noexcept
{
    assert(std::addressof(ds_) == std::addressof(other.ds_));
    row_    = other.row_;
    column_ = other.column_;
    return *this;
}

template<typename U>
inline dataset::row_data::cell::operator U() const
{
    return ds_.at<U>(row_, column_);
}

template<typename U>
inline
U dataset::row_data::cell::get() const
{
    return ds_.at<U>(row_, column_);
}

inline type_mask_t const dataset::row_data::cell::type() const
{
    return ds_.type_at(row_, column_);
}

}   // namespace data_processing
}   // namespace cdmh
