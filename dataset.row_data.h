// Copyright (c) 2013 Craig Henderson
// Part of the Data Processing Library
// https://github.com/cdmh/dataproc

#pragma once

namespace cdmh {
namespace data_processing {

/*
    dataset::row_data member functions
*/
inline dataset::row_data::cell_reference::cell_reference(dataset const &ds, size_t row, size_t column)
  : dd_(ds), row_(row),column_(column)
{
}

inline dataset::row_data::row_data(dataset const &ds, size_t row)
  : dd_(ds), row_(row)
{
}

inline dataset::row_data::cell_reference dataset::row_data::operator[](int column) const
{
    return (*this)[(size_t)column];
}

inline dataset::row_data::cell_reference dataset::row_data::operator[](size_t column) const
{
    return cell_reference(dd_, row_, column);
}

inline dataset::row_data::cell_reference dataset::row_data::operator[](char const *name) const
{
    return cell_reference(dd_, row_, dd_.lookup_column(name));
}

inline size_t dataset::row_data::size() const
{
    return dd_.columns();
}

inline dataset::row_data::cell_reference::cell_reference(dataset::row_data::cell_reference &&other) noexcept
  : dd_(other.dd_), row_(other.row_), column_(other.column_)
{
}

template<typename U>
inline dataset::row_data::cell_reference::operator U() const
{
    return dd_.at<U>(row_, column_);
}

template<typename U>
inline
U dataset::row_data::cell_reference::get() const
{
    return dd_.at<U>(row_, column_);
}

inline type_mask_t const dataset::row_data::cell_reference::type() const
{
    return dd_.column_type(column_);
}

}   // namespace data_processing
}   // namespace cdmh
