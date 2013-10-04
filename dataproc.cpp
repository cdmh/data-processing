#include "stdafx.h"
#include "csv_reader.h"

#if 1
#define CATCH_CONFIG_MAIN
#include "catch.hpp"

TEST_CASE("read_field", "Ensure reading of correct field types" )
{
    cdmh::csv_reader reader;

    REQUIRE(reader.read_field("Hello").second == cdmh::csv_reader::string_type);
    REQUIRE(reader.read_field("\"Hello World\"").second == cdmh::csv_reader::string_type);
    REQUIRE(reader.read_field("\"Hello \\\"World\\\"!\"").second == cdmh::csv_reader::string_type);
    REQUIRE(reader.read_field("8374").second == cdmh::csv_reader::integer_type);
    REQUIRE(reader.read_field("837.4").second == cdmh::csv_reader::double_type);
    REQUIRE(reader.read_field("+8374").second == cdmh::csv_reader::integer_type);
    REQUIRE(reader.read_field("+837.4").second == cdmh::csv_reader::double_type);
    REQUIRE(reader.read_field("-8374").second == cdmh::csv_reader::integer_type);
    REQUIRE(reader.read_field("-837.4").second == cdmh::csv_reader::double_type);
    REQUIRE(reader.read_field("83.7.4").second == cdmh::csv_reader::string_type);
    REQUIRE(reader.read_field("+83.7.4").second == cdmh::csv_reader::string_type);
    REQUIRE(reader.read_field("83a4").second == cdmh::csv_reader::string_type);
    REQUIRE(reader.read_field("8.3a4").second == cdmh::csv_reader::string_type);
    REQUIRE(reader.read_field("a8.34").second == cdmh::csv_reader::string_type);

    auto field = reader.read_field("Hello, World");
    REQUIRE(std::distance(field.first.first, field.first.second) == 5);
}
#else

int main()
{
    auto keypoints = cdmh::read_csv("data/facial-keypoints");

    return 0;
}
#endif
