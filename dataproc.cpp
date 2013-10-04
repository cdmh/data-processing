#include "stdafx.h"
#include "csv_reader.h"

#if 1
#define CATCH_CONFIG_MAIN
#include "catch.hpp"

std::pair<std::pair<char const *, char const *>, std::uint32_t>
read_field(char const *record)
{
    return cdmh::data_processing::detail::read_field(record, record+strlen(record));
}

TEST_CASE("read_field", "Ensure reading of correct field types" )
{
    using cdmh::data_processing::string_type;
    using cdmh::data_processing::double_type;
    using cdmh::data_processing::integer_type;

    REQUIRE(read_field("Hello").second == string_type);
    REQUIRE(read_field("\"Hello World\"").second == string_type);
    REQUIRE(read_field("\"Hello \\\"World\\\"!\"").second == string_type);
    REQUIRE(read_field("8374").second == integer_type);
    REQUIRE(read_field("837.4").second == double_type);
    REQUIRE(read_field("+8374").second == integer_type);
    REQUIRE(read_field("+837.4").second == double_type);
    REQUIRE(read_field("-8374").second == integer_type);
    REQUIRE(read_field("-837.4").second == double_type);
    REQUIRE(read_field("83.7.4").second == string_type);
    REQUIRE(read_field("+83.7.4").second == string_type);
    REQUIRE(read_field("83a4").second == string_type);
    REQUIRE(read_field("8.3a4").second == string_type);
    REQUIRE(read_field("a8.34").second == string_type);

    auto field = read_field("Hello, World");
    REQUIRE(std::distance(field.first.first, field.first.second) == 5);
}
#else

int main()
{
    auto keypoints = cdmh::read_csv("data/facial-keypoints");

    return 0;
}
#endif
