#include "stdafx.h"
#include <iomanip>
#include "../../data-processing.h"

namespace {     // anonymous namespace

std::vector<std::string> extract_words(std::string const &string)
{
    using cdmh::data_processing::detail::ltrim;

    std::vector<std::string> words;

    auto it  = string.begin();
    auto ite = string.end();
    for (auto begin=ltrim(it,ite); it!=ite; ++it)
    {
        words.push_back(std::string(begin,it));
    }

    return words;
}

}               // anonymous namespace

int main(int argc, char const *argv[])
{
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
    dd.attach(it, ite);

    std::cout << "\n";
    for (size_t loop=0; loop<dd.columns(); ++loop)
    {
        std::cout << loop << ": " << std::setw(15) << std::left << dd.column_title(loop);
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
        //auto title = dd[loop][3].get<std::string>();
        //std::vector<std::string> words = extract_words(title);
    }

	return 0;
}
