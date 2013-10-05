#include "stdafx.h"
#include <iostream>
#include "dataproc.h"

#if 1
#define CATCH_CONFIG_MAIN
#include "catch.hpp"

std::pair<std::pair<char const *, char const *>, std::uint32_t>
read_field(char const *record)
{
    return cdmh::data_processing::detail::read_field(record, record+strlen(record));
}

TEST_CASE("read_field/string fields", "Ensure reading of correct field types")
{
    REQUIRE(read_field("Hello").second == string_type);
    REQUIRE(read_field("\"Hello World\"").second == string_type);
    REQUIRE(read_field("\"Hello \\\"World\\\"!\"").second == string_type);
}

TEST_CASE("read_field/integer fields", "")
{
    REQUIRE(read_field("8374").second == integer_type);
    REQUIRE(read_field("837.4").second == double_type);
}

TEST_CASE("read_field/unary signs", "")
{
    REQUIRE(read_field("+8374").second == integer_type);
    REQUIRE(read_field("+837.4").second == double_type);
    REQUIRE(read_field("-8374").second == integer_type);
    REQUIRE(read_field("-837.4").second == double_type);
}

TEST_CASE("read_field/string fields starting with numerics", "")
{
    REQUIRE(read_field("83.7.4").second == string_type);
    REQUIRE(read_field("+83.7.4").second == string_type);
    REQUIRE(read_field("83a4").second == string_type);
    REQUIRE(read_field("8.3a4").second == string_type);
    REQUIRE(read_field("a8.34").second == string_type);
}

TEST_CASE("read_field/numerics with padding", "")
{
    REQUIRE(read_field("8374 ").second == integer_type);
    REQUIRE(read_field("+8374 ").second == integer_type);
    REQUIRE(read_field("-8374 ").second == integer_type);
    REQUIRE(read_field(" +8374").second == integer_type);
    REQUIRE(read_field(" +8374 ").second == integer_type);
}

TEST_CASE("read_field/comma separated fields with space padding", "")
{
    auto record = "      Hello, World   ";
    auto it = record;
    auto ite = record+strlen(record);
    auto field1 = cdmh::data_processing::detail::read_field(it, ite);
    auto field2 = cdmh::data_processing::detail::read_field(it, ite);
    REQUIRE(std::distance(field1.first.first, field1.first.second) == 5);
    REQUIRE(std::distance(field2.first.first, field2.first.second) == 5);
}

TEST_CASE("read_field/spaces around quoted string with leading & trailing spaces", "")
{
    auto record = "    \"  Hello, World \"  ";
    auto it = record;
    auto ite = record+strlen(record);
    auto field = cdmh::data_processing::detail::read_field(it, ite);
    REQUIRE(std::distance(field.first.first, field.first.second) == 15);
}
#else

int main()
{
#if defined(_MSC_VER)  &&  defined(_DEBUG)
    _CrtSetDbgFlag(_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG) | _CRTDBG_LEAK_CHECK_DF);
#endif

    cdmh::data_processing::mapped_csv csv("data/training.csv");
    csv.read(100);
    auto keypoints = csv.create_dataset();
    std::cout << keypoints.rows() << " records with " << keypoints.columns() << " columns";
    char const *image = keypoints[0][30];
    image = keypoints[1][30];
    image = keypoints[2][30];
    image = keypoints[3][30];
    auto a = keypoints[3];
    auto b = a[0];
    std::cout << "\n" << a[0] << " " << a[1];

    return 0;
}
#endif
