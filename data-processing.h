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

namespace cdmh {
namespace data_processing {

typedef std::uint8_t type_mask_t;

}   // namespace data_processing
}   // namespace cdmh


namespace {

static cdmh::data_processing::type_mask_t const string_type  = 1;
static cdmh::data_processing::type_mask_t const double_type  = 1 << 1;
static cdmh::data_processing::type_mask_t const integer_type = 1 << 2;
static cdmh::data_processing::type_mask_t const null_type    = 1 << 3;

}   // anonymous namespace

// project header files
#include "dataset.h"
#include "mapped_csv.h"
