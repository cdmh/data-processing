// Copyright (c) 2013 Craig Henderson
// Part of the Data Processing Library
// https://github.com/cdmh/dataproc

#pragma once

namespace cdmh {
namespace data_processing {

/*
    dataset::cell_value
*/
inline dataset::cell_value::cell_value(type_mask_t type, double dbl)
    : type_(type), double_(dbl)
{
    assert(type == double_type  ||  (type == null_type  &&  double_  == 0.0)); 
}

inline dataset::cell_value::cell_value(type_mask_t type, std::uint32_t integer)
  : type_(type), integer_(integer)
{
    assert(type == integer_type ||  (type == null_type  &&  integer_ == 0));
}

inline dataset::cell_value::cell_value(type_mask_t type, std::string string)
  : type_(type), string_(new std::string(std::forward<std::string>(string)))
{
    assert(type == string_type  ||  type == null_type);
}

inline dataset::cell_value::cell_value(dataset::cell_value &&other)
{
    memcpy(this, &other, sizeof(*this));
    memset(&other, 0, sizeof(*this));
}

inline dataset::cell_value::cell_value(dataset::cell_value const &other)
{
    *this = other;
}

inline dataset::cell_value::~cell_value()
{
    clear();
}

inline dataset::cell_value &dataset::cell_value::operator=(dataset::cell_value const &other)
{
    memcpy(this, &other, sizeof(*this));
    if (is_string())
        string_ = new std::string(*other.string_);
    return *this;
}

inline void dataset::cell_value::clear()
{
    if (type_ == string_type)
        delete string_;
    memset(this, 0, sizeof(*this));
    type_ = null_type;
}

inline type_mask_t const dataset::cell_value::type() const
{
    return type_;
}

inline bool const dataset::cell_value::is_double() const
{
    return type_ == double_type;
}

inline bool const dataset::cell_value::is_integer() const
{
    return type_ == integer_type;
}

inline bool const dataset::cell_value::is_string() const
{
    return type_ == string_type;
}

inline bool const dataset::cell_value::is_null() const
{
    return type_ == null_type;
}

template<typename T>
T dataset::cell_value::get() const
{
    static_assert(false, "Unsupported type: try char const *, std::string, double or std::uint32_t");
}

template<>
inline
double dataset::cell_value::get() const
{
    assert(type_ == double_type  ||  type_ == null_type);
    return double_;
}

template<>
inline
std::string dataset::cell_value::get() const
{
    assert(type_ == string_type);
    return *string_;
}

template<>
inline
char const *  dataset::cell_value::get() const
{
    assert(type_ == string_type);
    return string_->c_str();
}

template<>
inline
std::uint32_t dataset::cell_value::get() const
{
    assert(type_ == integer_type  ||  type_ == null_type);
    return integer_;
}

}   // namespace data_processing
}   // namespace cdmh
