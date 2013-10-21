// An implementation of the Porter Stemming Algorithm
// For a description, see http://tartarus.org/martin/PorterStemmer/
// 
// Copyright (c) 2013 Craig Henderson
// Part of the Data Processing Library
// https://github.com/cdmh/dataproc

#include <string>

namespace cdmh {
namespace data_processing {

template<typename It>
std::string porter_stemmer(It it, It ite)
{
    return std::string();
}

std::string porter_stemmer(char const *string)
{
    auto stem = porter_stemmer(string, string + strlen(string));
    return std::string(stem.begin(), stem.end());
}

std::string porter_stemmer(std::string const &string)
{
    auto stem = porter_stemmer(string.cbegin(), string.cend());
    return std::string(stem.begin(), stem.end());
}

}   // namespace data_processing
}   // namespace cdmh
