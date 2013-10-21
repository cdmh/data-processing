// Copyright (c) 2013 Craig Henderson
// Part of the Data Processing Library
// https://github.com/cdmh/dataproc

#include "stdafx.h"
#include <iostream>
#include "data-processing.h"

#define CATCH_CONFIG_RUNNER
#include "catch.hpp"

namespace { // anonymous namespace

inline
std::pair<cdmh::data_processing::string_view, cdmh::data_processing::type_mask_t>
read_field(char const *record)
{
    return cdmh::data_processing::detail::read_field(record, record+strlen(record));
}

TEST_CASE("read_field/string fields", "Ensure reading of correct field types")
{
    CHECK(read_field("Hello").second == string_type);
    CHECK(read_field("\"Hello World\"").second == string_type);
    CHECK(read_field("\"Hello \"\"World\"\"!\"").second == string_type);
    CHECK(read_field("\"Hello \"\"World\"\"!\"").first.length() == 16);
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
    auto field2 = cdmh::data_processing::detail::read_field(++it, ite);
    CHECK(field1.second == string_type);
    CHECK(field1.first.length() == 5);
    CHECK(field2.second == string_type);
    CHECK(field2.first.length() == 5);
}

TEST_CASE("read_field/spaces around quoted string with leading & trailing spaces", "")
{
    auto record = "    \"  Hello, World \"  ";
    auto it = record;
    auto ite = record+strlen(record);
    auto field = cdmh::data_processing::detail::read_field(it, ite);
    CHECK(std::distance(field.first.begin(), field.first.end()) == 15);
}

TEST_CASE("dataset/attach to string")
{
    char const *data =
        "col1,col2,col3,col4,col5\n"
        "193,2982,83.326,9838,\"243 837 636 233 222 829\"\n"
        "193,1229,22.345,2437,\"230 389 198 827 273 536\"\n"
        "837,1229,83.326,9838,\"233 222 243 837 636 829\"\n"
        ;

    cdmh::data_processing::dataset ds;
    ds.attach(data);
    CHECK(ds.columns() == 5);
    CHECK_THROWS_AS(ds.column("column333"), cdmh::data_processing::dataset::invalid_column_name);

    CHECK(ds.column("col1").is_integer());
    CHECK(ds.column("col2").is_integer());
    CHECK(ds.column("col3").is_double());
    CHECK(ds.column("col4").is_integer());
    CHECK(ds.column("col5").is_string());

    auto p1 = ds[0];
    auto p2 = p1[0];

    SECTION("data access") {
        CHECK((std::uint32_t)ds[0][0] == 193);
        CHECK((std::uint32_t)ds[0]["col2"] == 2982);
        CHECK((double)ds[0][2] == 83.326);
        CHECK(ds[0][4].get<std::string>().length() == 23);
    }

    SECTION("averages") {
        CHECK(ds.column(0).mean() == 407.6666666666667);
        CHECK(fabs(ds.column(2).mean() - 62.999) < 0.00001);

        CHECK_THROWS_AS(cdmh::data_processing::maths::median<double>({}), cdmh::data_processing::maths::math_error);
        CHECK(ds.column(0).median() == 193);
        CHECK(fabs(ds.column(2).median() - 83.326) < 0.00001);

        CHECK_THROWS_AS(cdmh::data_processing::maths::median<double>({}), cdmh::data_processing::maths::math_error);
        CHECK_THROWS_AS(cdmh::data_processing::maths::mode<double>({ 1,2,3,4,5,6,7 }), cdmh::data_processing::maths::math_error);
        CHECK(ds.column(1).mode() == 1229);
        CHECK(fabs(ds.column(2).mode() - 83.326) < 0.00001);

        CHECK_THROWS_AS(cdmh::data_processing::maths::standard_deviation<double>({}), cdmh::data_processing::maths::math_error);
        CHECK(cdmh::data_processing::maths::standard_deviation<double>({ 2,4,4,4,5,5,7,9 }) == 2.0);
        CHECK(cdmh::data_processing::maths::standard_deviation<int>({ 2,4,4,4,5,5,7,9 }) == 2.0);
    }

    SECTION("count") {
        CHECK(ds.column(0).count_null() == 0);
        CHECK(ds.column(0).count_unique() == 2);
    }

    SECTION("min/max") {
        CHECK(ds.column(2).min<double>() == 22.345);
        CHECK(ds.column(1).min<std::uint32_t>() == 1229);
        CHECK(ds.column(2).max<double>() == 83.326);
        CHECK(ds.column(1).max<std::uint32_t>() == 2982);
    }

    SECTION("averages") {
        CHECK(ds.column(0).count_null() == 0);
    }

    SECTION("extracting data") {
        auto extracted_data = ds.column(0).extract<std::uint32_t>();
        CHECK(extracted_data.size() == ds.rows());
    }
}

TEST_CASE("mapped_csv", "")
{
#ifdef NDEBUG
    size_t const rows_requested = 0;
    size_t const rows_expected  = 7049;
#else
    size_t const rows_requested = 220;
    size_t const rows_expected  = rows_requested;
#endif

    std::string filename("/test-data/facial-keypoints-detection/training.csv");
    cdmh::memory_mapped_file<char> mmf(filename);
    REQUIRE(mmf.is_open());

    char const *it = mmf.get();
    char const *ite = it + mmf.size();
    cdmh::data_processing::dataset dd;
    dd.attach(it, ite, rows_requested);

    CHECK(dd.rows() == rows_expected);

    std::cout << dd.rows() << " records with " << dd.columns() << " columns\n";
    CHECK(dd.rows() == rows_expected);
    CHECK(dd.columns() == 31);

    SECTION("string data access") {
        // access to string data through casting or calling get<>()
        std::string image = (std::string)dd[0][30];
        CHECK(!image.empty());
        image = dd[1][30].get<std::string>();   // access row data
        CHECK(!image.empty());
        image = dd[2][30].get<std::string>();
        CHECK(!image.empty());
        image = dd.row(3)[30].get<std::string>();
        CHECK(!image.empty());
    }

    SECTION("output stream tests") {
        std::ostringstream stream;
        auto a = dd[3];
        stream << a[0] << " " << a[1] << " ";   // test value serialisation
        stream << dd[210];                      // test row serialisation

        std::ofstream f("out.csv");
        dd.erase_column(30);
        f << dd;
    }

    SECTION("count") {
        CHECK((dd.column(0).count() + dd.column(0).count_null()) == dd.column(0).size());
        CHECK((dd.column(0).count() + dd.column(0).count_null()) == dd.rows());
    }

    SECTION("averages") {
        // the column mean ignores null values, so will can't be less
        CHECK(dd.column(7).mean() >= (dd.column(7).sum<double>() / dd.rows()));
        std::cout << "Mean without NULLs: " << dd.column("right_eye_outer_corner_x").mean() << "\n";
        std::cout << "Mean with NULLs   : " << dd.column("right_eye_outer_corner_x").sum<double>() / dd.rows() << "\n";
        std::cout << "Median            : " << dd.column(0).median() << "\n";
//        std::cout << "Mode              : " << dd.column(0).mode() << "\n";
        std::cout << "Standard Deviation: " << dd.column(0).standard_deviation() << "\n";
        std::cout << "Min               : " << dd.column(0).min<double>() << "\n";
        std::cout << "Max               : " << dd.column(0).max<double>() << "\n";
        CHECK(dd.column(0).min<double>() <= dd.column(0).max<double>());
    }

    SECTION("split text string") {
        for (size_t loop=0; loop<dd.rows(); ++loop)
        {
            auto integers = cdmh::data_processing::split_string<std::uint32_t>(dd[loop][30].get<std::string>(), ' ');
            //for (auto const value : integers)
            //    std::cout << value << ",";
            if ((loop % 1000) == 0)
                std::cout << loop << " : " << integers.size() << "\n";
        }
    }

    std::cout << "\n";
}

}   // anonymous namespace

int main(int argc, char * const argv[])
{
#if defined(_MSC_VER)  &&  defined(_DEBUG)
    _CrtSetDbgFlag(_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG) | _CRTDBG_LEAK_CHECK_DF);
//    _CrtSetBreakAlloc(6012);
#endif

    Catch::Session session;
    Catch::ConfigData &config_data = session.configData();
    config_data.showDurations = Catch::ShowDurations::OrNot::Always;

    int returnCode = session.applyCommandLine(argc, argv);
    if (returnCode != 0)
        return returnCode;

    return session.run();
}
