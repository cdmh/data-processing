#include "stdafx.h"

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
    cdmh::data_processing::dataset ds;
//    ds.import_csv("/test-data/keyword-extraction/tmp.csv");
    ds.import_csv("/test-data/keyword-extraction/train.csv");

    // id, title, body, tags
    for (size_t loop=0; loop<ds.rows(); ++loop)
    {
        auto title = ds[loop][3].get<std::string>();
        std::vector<std::string> words = extract_words(title);
    }

	return 0;
}
