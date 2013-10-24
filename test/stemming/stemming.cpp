// Copyright (c) 2013 Craig Henderson
// Part of the Data Processing Library
// https://github.com/cdmh/dataproc

#include"stdafx.h"
#include"porter_stemming.h"

#define CATCH_CONFIG_RUNNER
#include"catch.hpp"

// defined in stemming_c_thread_safe.cpp
namespace original {
std::string stem(char const * const word);
}

using cdmh::data_processing::porter_stemmer::stem;

TEST_CASE(">", "")
{
    CHECK(original::stem("agree") == "agre");
    CHECK(stem("agree") == "agre");
    CHECK(stem("AGREE") == "AGRE");
    CHECK(cdmh::strcasecmp(stem("FALSE").c_str(), stem("false").c_str()) == 0);
}

int main(int argc, char *const argv[])
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
