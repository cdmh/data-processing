#include "stdafx.h"
#include <iostream>
#include "data-processing.h"

#define CATCH_CONFIG_RUNNER
#include "catch.hpp"

std::pair<std::pair<char const *, char const *>, std::uint32_t>
read_field(char const *record)
{
    return cdmh::data_processing::detail::read_field(record, record+strlen(record));
}

TEST_CASE("read_field/string fields", "Ensure reading of correct field types")
{
    CHECK(read_field("Hello").second == string_type);
    CHECK(read_field("\"Hello World\"").second == string_type);
    CHECK(read_field("\"Hello \\\"World\\\"!\"").second == string_type);
}

TEST_CASE("read_field/integer fields", "")
{
    CHECK(read_field("8374").second == integer_type);
    CHECK(read_field("837.4").second == double_type);
}

TEST_CASE("read_field/unary signs", "")
{
    CHECK(read_field("+8374").second == integer_type);
    CHECK(read_field("+837.4").second == double_type);
    CHECK(read_field("-8374").second == integer_type);
    CHECK(read_field("-837.4").second == double_type);
}

TEST_CASE("read_field/string fields starting with numerics", "")
{
    CHECK(read_field("83.7.4").second == string_type);
    CHECK(read_field("+83.7.4").second == string_type);
    CHECK(read_field("83a4").second == string_type);
    CHECK(read_field("8.3a4").second == string_type);
    CHECK(read_field("a8.34").second == string_type);
}

TEST_CASE("read_field/numerics with padding", "")
{
    CHECK(read_field("8374 ").second == integer_type);
    CHECK(read_field("+8374 ").second == integer_type);
    CHECK(read_field("-8374 ").second == integer_type);
    CHECK(read_field(" +8374").second == integer_type);
    CHECK(read_field(" +8374 ").second == integer_type);
}

TEST_CASE("read_field/comma separated fields with space padding", "")
{
    auto record = "      Hello, World   ";
    auto it = record;
    auto ite = record+strlen(record);
    auto field1 = cdmh::data_processing::detail::read_field(it, ite);
    auto field2 = cdmh::data_processing::detail::read_field(it, ite);
    CHECK(std::distance(field1.first.first, field1.first.second) == 5);
    CHECK(std::distance(field2.first.first, field2.first.second) == 5);
}

TEST_CASE("read_field/spaces around quoted string with leading & trailing spaces", "")
{
    auto record = "    \"  Hello, World \"  ";
    auto it = record;
    auto ite = record+strlen(record);
    auto field = cdmh::data_processing::detail::read_field(it, ite);
    CHECK(std::distance(field.first.first, field.first.second) == 15);
}




TEST_CASE("mapped_csv", "")
{
    cdmh::data_processing::mapped_csv csv("data/training.csv");

#ifdef NDEBUG
    size_t const rows_requested = 0;
    size_t const rows_expected  = 7049;
#else
    size_t const rows_requested = 220;
    size_t const rows_expected  = rows_requested;
#endif

    REQUIRE(csv.read(rows_requested));
    REQUIRE(csv.size() == rows_expected);

    auto keypoints = csv.create_dataset();
    std::cout << keypoints.rows() << " records with " << keypoints.columns() << " columns\n";
    CHECK(keypoints.rows() == rows_expected);
    REQUIRE(keypoints.columns() == 31);

    char const *image = keypoints[0][30];
    image = keypoints[1][30];   // access row data
    image = keypoints[2][30];
    image = keypoints[3][30];

    // test value serialisation
    auto a = keypoints[3];
    std::cout << "\n" << a[0] << " " << a[1];

    // test row serialisation
    std::cout << "\n" << keypoints[210];
    std::cout << "\n" << keypoints[211];
    std::cout << "\n" << keypoints[212];
}

int main(int argc, char * const argv[])
{
#if defined(_MSC_VER)  &&  defined(_DEBUG)
    _CrtSetDbgFlag(_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG) | _CRTDBG_LEAK_CHECK_DF);
#endif

    Catch::Session session;
    Catch::ConfigData &config_data = session.configData();
    config_data.showDurations = Catch::ShowDurations::OrNot::Always;

    int returnCode = session.applyCommandLine(argc, argv);
    if (returnCode != 0)
        return returnCode;

    auto const result = session.run();
    std::cout << "\n";
    return result;
}
