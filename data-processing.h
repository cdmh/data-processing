// Copyright (c) 2013 Craig Henderson
// Part of the Data Processing Library
// https://github.com/cdmh/dataproc

#pragma once

// standard header files
#include <cstdint>          // std::uint8_t
#include <vector>
#include <iosfwd>           // basic_ostream
#include <functional>       // std::function
#include <locale>
#include <cassert>

// project "system" header files
#include "memmap.h"

#include "data_processing.detail.h"

namespace cdmh {

namespace data_processing {

template<typename T, typename U>
inline
std::vector<T> split_string(U const &string, char const delim)
{
    std::vector<T> result;

    auto it  = string.cbegin();
    auto ite = string.cend();
    while (detail::ltrim(it,ite) != ite)
    {
        auto sp = std::find_if(it, ite, [delim](char ch) { return ch == delim; });
        result.push_back(atol(std::string(it,sp).c_str()));
        it = sp;
    }
    return result;
}

}   // namespace data_processing
}   // namespace cdmh


namespace {

static cdmh::data_processing::type_mask_t const string_type  = 1;
static cdmh::data_processing::type_mask_t const double_type  = 1 << 1;
static cdmh::data_processing::type_mask_t const integer_type = 1 << 2;
static cdmh::data_processing::type_mask_t const null_type    = 1 << 3;

}   // anonymous namespace

// project header files
#include "string_view.h"
#include "dataset.h"
#include "mapped_csv.h"
#include "maths.h"
#include "porter_stemming.h"

#include "dataset.impl.h"
#include "dataset.column_data.h"
#include "dataset.row_data.h"

