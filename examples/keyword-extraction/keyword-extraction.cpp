#include "stdafx.h"
#include <locale>
#include "../../data-processing.h"

namespace {     // anonymous namespace

using cdmh::data_processing::string_view;


// http://armandbrahaj.blog.al/2009/04/14/list-of-english-stop-words/
char const * const english_stopwords[] = {
    "a", "about", "above", "above", "across", "after", "afterwards", "again", "against",
    "all", "almost", "alone", "along", "already", "also","although","always","am","among",
    "amongst", "amoungst", "amount",  "an", "and", "another", "any","anyhow","anyone",
    "anything","anyway", "anywhere", "are", "around", "as",  "at", "back","be","became",
    "because","become","becomes", "becoming", "been", "before", "beforehand", "behind",
    "being", "below", "beside", "besides", "between", "beyond", "bill", "both", "bottom",
    "but", "by", "call", "can", "cannot", "cant", "co", "con", "could", "couldnt", "cry",
    "de", "describe", "detail", "do", "done", "down", "due", "during", "each", "eg", "eight",
    "either", "eleven","else", "elsewhere", "empty", "enough", "etc", "even", "ever", "every",
    "everyone", "everything", "everywhere", "except", "few", "fifteen", "fify", "fill",
    "find", "fire", "first", "five", "for", "former", "formerly", "forty", "found", "four",
    "from", "front", "full", "further", "get", "give", "go", "had", "has", "hasnt", "have",
    "he", "hence", "her", "here", "hereafter", "hereby", "herein", "hereupon", "hers", "herself",
    "him", "himself", "his", "how", "however", "hundred", "ie", "if", "in", "inc", "indeed",
    "interest", "into", "is", "it", "its", "itself", "keep", "last", "latter", "latterly",
    "least", "less", "ltd", "made", "many", "may", "me", "meanwhile", "might", "mill", "mine",
    "more", "moreover", "most", "mostly", "move", "much", "must", "my", "myself", "name",
    "namely", "neither", "never", "nevertheless", "next", "nine", "no", "nobody", "none",
    "noone", "nor", "not", "nothing", "now", "nowhere", "of", "off", "often", "on", "once",
    "one", "only", "onto", "or", "other", "others", "otherwise", "our", "ours", "ourselves",
    "out", "over", "own","part", "per", "perhaps", "please", "put", "rather", "re", "same",
    "see", "seem", "seemed", "seeming", "seems", "serious", "several", "she", "should",
    "show", "side", "since", "sincere", "six", "sixty", "so", "some", "somehow", "someone",
    "something", "sometime", "sometimes", "somewhere", "still", "such", "system", "take",
    "ten", "than", "that", "the", "their", "them", "themselves", "then", "thence", "there",
    "thereafter", "thereby", "therefore", "therein", "thereupon", "these", "they", "thickv",
    "thin", "third", "this", "those", "though", "three", "through", "throughout", "thru",
    "thus", "to", "together", "too", "top", "toward", "towards", "twelve", "twenty", "two",
    "un", "under", "until", "up", "upon", "us", "very", "via", "was", "we", "well", "were",
    "what", "whatever", "when", "whence", "whenever", "where", "whereafter", "whereas",
    "whereby", "wherein", "whereupon", "wherever", "whether", "which", "while", "whither",
    "who", "whoever", "whole", "whom", "whose", "why", "will", "with", "within", "without",
    "would", "yet", "you", "your", "yours", "yourself", "yourselves" };

bool const is_stop_word(string_view const &word)
{
    auto const stopwords_end = english_stopwords + (sizeof(english_stopwords) / sizeof(english_stopwords[0]));
    // stop word must be in ascending order
    assert(
        std::is_sorted(
            english_stopwords,
            stopwords_end,
            [](char const *first, char const *second) {
                if (strcmp(first, second) < 0)
                    std::cout << first << ", " << second << " " << strcmp(first, second) << "\n";
                return strcmp(first, second) < 0;
            }));

#if WIN32 && !defined(strncasecmp)
#define strncasecmp _strnicmp
#endif

    return
        std::binary_search(
            english_stopwords,
            stopwords_end,
            word,
            [](string_view const &first, string_view const &second) {
                auto const len1 = first.length();
                auto const len2 = second.length();
                if (len1 < len2)
                {
                    auto cmp = strncasecmp(first.begin(), second.begin(), len1);
                    return (cmp <= 0);
                }
                else if (len1 > len2)
                {
                    auto cmp = strncasecmp(first.begin(), second.begin(), len2);
                    return (cmp < 0);
                }
                return strncasecmp(first.begin(), second.begin(), len1) < 0;
            });
}

template<typename It>
inline
It find_word_begin(It &it,It ite)
{
    while (it != ite  &&  (*it<'a'  ||  *it>'z')  &&  (*it<'A'  ||  *it>'Z')  &&  (*it != '_'))
        ++it;

    return it;
}

string_view next_word(char const *&it, char const *ite)
{
    auto begin = find_word_begin(it,ite);
    it = std::find_if(it, ite, [](char ch) { return ch == ' '; });
    return string_view(begin, it);
}

template<typename Out>
void calculate_hashes(string_view const &string, Out ito, bool ignore_stopwords=true)
{
    std::locale loc;
    std::collate<char> const &col = std::use_facet<std::collate<char>>(loc);
    auto it  = string.begin();
    auto ite = string.end();
    std::cout << string << "\n";
    while (it != ite)
    {
        string_view const word = next_word(it, ite);
        if (!ignore_stopwords  ||  !is_stop_word(word))
        {
            std::cout << "    " << word << "\n";
            *ito++ = col.hash(word.begin(), word.end());
        }
    }
}


template<typename It>
inline
typename std::iterator_traits<It>::value_type const
sum(It begin, It end)
{
    using type = typename std::iterator_traits<It>::value_type;
    return std::accumulate(
        begin, end, type(),
        [](type sum, type const &value) {
            return sum + value;
        });
}


template<typename It>
double const mean(It begin, It end)
{
    using type = typename std::iterator_traits<It>::value_type;

    auto const count = std::distance(begin, end);
    // assert that the cast is safe
    assert(count <= std::numeric_limits<type>::max());
    return sum(begin, end) / type(count);
}

}               // anonymous namespace

int main(int argc, char const *argv[])
{
#if defined(_MSC_VER)  &&  defined(_DEBUG)
    _CrtSetDbgFlag(_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG) | _CRTDBG_LEAK_CHECK_DF);
#endif

    char const *filename = "\\test-data\\keyword-extraction\\train.csv";
    cdmh::memory_mapped_file<char> const mmf(filename);
    if (!mmf.is_open())
    {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return 1;
    }

    std::cout << "Loading file ...";
    cdmh::data_processing::dataset dd;
#ifdef NDEBUG
    size_t num_rows = 0;
#else
    size_t num_rows = 200;
#endif
    dd.attach(mmf.get(), mmf.get() + mmf.size(), num_rows);

    std::cout << "\n";
    for (size_t loop=0; loop<dd.columns(); ++loop)
    {
        std::cout << std::setw(2) << std::right << loop << ": " << std::setw(25) << std::left << dd.column_title(loop);
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
        auto title = dd[loop][1].get<string_view>();
        std::vector<long> title_hashes;
        calculate_hashes(title, std::back_inserter(title_hashes));

        auto tags  = dd[loop][3].get<string_view>();
        std::vector<long> tags_hashes;
        calculate_hashes(tags, std::back_inserter(tags_hashes), false);

        std::cout << title << '\n' << tags << '\n';
    }

	return 0;
}
