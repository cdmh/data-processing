#include "stdafx.h"
#include <iomanip>
#include "../../data-processing.h"

namespace {     // anonymous namespace

using cdmh::data_processing::string_view;

std::vector<string_view> extract_words(string_view const &string)
{
    using cdmh::data_processing::detail::ltrim;

    std::vector<string_view> words;

    auto it  = string.begin();
    auto ite = string.end();
    while (it!=ite)
    {
        auto begin = it;
        it = std::find_if(it, ite, [](char ch) { return ch == ' '; });
        words.emplace_back(begin, it);
        if (it != ite)
            ltrim(++it,ite);
    }

    return words;
}

}               // anonymous namespace

int main(int argc, char const *argv[])
{
#if defined(_MSC_VER)  &&  defined(_DEBUG)
    _CrtSetDbgFlag(_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG) | _CRTDBG_LEAK_CHECK_DF);
#endif

    char const *filename = "\\test-data\\keyword-extraction\\train.csv";
    cdmh::memory_mapped_file<char> mmf(filename);
    if (!mmf.is_open())
    {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return 1;
    }

    std::cout << "Loading file ...";
    char const *it = mmf.get();
    char const *ite = it + mmf.size();
    cdmh::data_processing::dataset dd;
#ifdef NDEBUG
    size_t num_rows = 0;
#else
    size_t num_rows = 200;
#endif
    dd.attach(it, ite, num_rows);

    std::cout << "\n";
    for (size_t loop=0; loop<dd.columns(); ++loop)
    {
        std::cout << std::setw(2) << loop << ": " << std::setw(25) << std::left << dd.column_title(loop);
        switch (dd.column_type(loop))
        {
            case string_type:   std::cout << "\tstring";    break;
            case double_type:   std::cout << "\tdouble";    break;
            case integer_type:  std::cout << "\tinteger";   break;
        }
        std::cout << "\n";
    }

    // id, title, body, tags
    for (size_t loop=0; loop<dd.rows(); ++loop)
    {
        auto title = dd[loop][3].get<string_view>();
        auto words = extract_words(title);
        for (auto const &word : words)
            ;
    }

	return 0;
}
