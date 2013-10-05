#pragma once

// standard header files
#include <cstdint>          // std::uint32_t
#include <vector>
#include <iosfwd>           // basic_ostream
#include <functional>       // std::function
#include <locale>
#include <cassert>

// project "system" header files
#include "memmap.h"

namespace {

static std::uint32_t const string_type  = 1;
static std::uint32_t const double_type  = 1 << 1;
static std::uint32_t const integer_type = 1 << 2;
static std::uint32_t const null_type    = 1 << 3;

}   // anonymous namespace

// project header files
#include "dataset.h"
#include "mapped_csv.h"
