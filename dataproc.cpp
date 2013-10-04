#include "stdafx.h"
#include <iostream>
#include "mapped_csv.h"

#if 0
#define CATCH_CONFIG_MAIN
#include "catch.hpp"

std::pair<std::pair<char const *, char const *>, std::uint32_t>
read_field(char const *record)
{
    return cdmh::data_processing::detail::read_field(record, record+strlen(record));
}

TEST_CASE("read_field", "Ensure reading of correct field types")
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

    auto record = "      Hello, World   ";
    auto it = record;
    auto ite = record+strlen(record);
    auto field1 = cdmh::data_processing::detail::read_field(it, ite);
    auto field2 = cdmh::data_processing::detail::read_field(it, ite);
    REQUIRE(std::distance(field1.first.first, field1.first.second) == 5);
    REQUIRE(std::distance(field2.first.first, field2.first.second) == 5);
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
    std::cout << "\n" << a[0] << " " << (double)a[1];

    return 0;
}
#endif
