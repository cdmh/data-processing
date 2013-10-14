#include "stdafx.h"
#include <locale>
#include <map>
#include <memory>
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
    "him", "himself", "his", "how", "however", "hundred", "i", "ie", "if", "in", "inc", "indeed",
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
                return strcmp(first, second) < 0;
            }));

    return std::binary_search(english_stopwords, stopwords_end, word);
}

bool const is_word_char(char ch)
{
    return (ch >= '0'  &&  ch <= '9')
       ||  (ch >= 'a'  &&  ch <= 'z')
       ||  (ch >= 'A'  &&  ch <= 'Z')
       ||   ch == '-'  ||  ch == '_'  ||  ch == '\'';
}

template<typename It>
inline
It find_word_begin(It &it,It ite)
{
    while (it != ite  &&  !is_word_char(*it))
        ++it;
    return it;
}

string_view next_word(char const *&it, char const *ite, bool ignore_stopwords=true)
{
    auto begin = find_word_begin(it,ite);
    it = std::find_if(it, ite, [](char ch) { return !is_word_char(ch); });
    auto word = string_view(begin, it);
    return (ignore_stopwords  &&  is_stop_word(word))?
        next_word(it, ite)
      : word;
}

template<typename Out>
void calculate_hashes(string_view const &string, Out ito, bool ignore_stopwords=true)
{
    std::locale loc;
    std::collate<char> const &col = std::use_facet<std::collate<char>>(loc);
    auto it  = string.begin();
    auto ite = string.end();
    while (it != ite)
    {
        string_view const word = next_word(it, ite, ignore_stopwords);
        if (word.length() > 0)
            *ito++ = col.hash(word.begin(), word.end());
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


template<typename Words>
void count_words(string_view const &string, Words &words, bool ignore_stopwords)
{
    auto it  = string.begin();
    auto ite = string.end();
    while (it != ite)
    {
        string_view const word = next_word(it, ite, ignore_stopwords);
        if (word.length() > 0)
            words[word]++;
    }
}

void create_word_list_and_map(
    cdmh::data_processing::dataset const &ds,
    int                                   column,
    std::vector<string_view>             &list,
    std::map<string_view, int>           &indices,
    bool                                  ignore_stopwords=true)
{
    typedef std::map<string_view, unsigned> words_t;
    words_t word_map;

    // count up the words and
    for (size_t loop=0; loop<ds.rows(); ++loop)
        count_words(ds[loop][column].get<string_view>(), word_map, ignore_stopwords);

    // add the words to the list
    list.reserve(word_map.size());
    std::transform(
        word_map.begin(),
        word_map.end(),
        std::back_inserter(list),
        [](words_t::value_type const &word_and_count) {
            return word_and_count.first;
        });

    // map the words to the index into the list
    for (int loop=0; loop<list.size(); ++loop)
        indices[list[loop]] = loop;
}

}               // anonymous namespace


#include "naive-bayes-classifier/src/BayesianClassifier.h"
#include "naive-bayes-classifier/src/ActionClassifier.h"
#include "naive-bayes-classifier/src/Domain.h"

/*
  inspired by http://www.inf.ed.ac.uk/teaching/courses/inf2b/learnnotes/inf2b-learn-note07-2up.pdf
*/
class classifier
{
  public:
    explicit classifier(cdmh::data_processing::dataset const &ds) : ds_(ds)
    {
        create_word_list_and_map(ds, 1, title_words, title_word_indices);
        create_word_list_and_map(ds, 3, tag_words, tag_word_indices);

        // create domains:
        //     title_words.size() input domains
        //     tag_words.size() output domains
	    std::vector<Domain> domains;
        for (size_t d=0; d<title_words.size() + tag_words.size(); ++d)
	        domains.emplace_back(0.0f, 1.0f, 2);  // min, max, number of values
        classifier_.reset(new BayesianClassifier(domains));
    }

    void train(
        size_t training_rows_begin,
        size_t training_rows_end)
    {
        std::vector<float> training;
        training.resize(title_words.size() + tag_words.size(), 0.0);
        for (size_t loop=training_rows_begin; loop<training_rows_end; ++loop)
        {
            // reset the vector to contain zeros
            for (float &value : training)
                value = 0.0;

            auto string = ds_[loop][1].get<string_view>();
            auto it  = string.begin();
            auto ite = string.end();
            while (it != ite)
            {
                string_view const word = next_word(it, ite);
                if (word.length() > 0)
                {
                    auto it = title_word_indices.find(word);
                    assert(it != title_word_indices.end());
                    training[it->second] = 1.0;    // !!!binary title does/doesn't contain tag. Count might work better?
                }
            }
            std::cout << "\n\n" << string;

            string = ds_[loop][3].get<string_view>();
            it  = string.begin();
            ite = string.end();
            while (it != ite)
            {
                string_view const word = next_word(it, ite);
                if (word.length() > 0)
                {
                    auto it = tag_word_indices.find(word);
                    assert(it != tag_word_indices.end());
                    training[it->second + title_words.size()] = 1.0;    // !!!binary title does/doesn't contain tag. Count might work better?
                }
            }

#if 0 && !defined(NDEBUG)
            std::cout << "\n";
            for (float &value : training)
                std::cout << (int)value;
#endif

            std::cout << "\n";
            for (size_t loop=0; loop<training.size(); ++loop)
            {
                if (training[loop])
                {
                    if (loop < title_words.size())
                        std::cout << title_words[loop] << " ";
                    else
                        std::cout << "\n*** " << tag_words[loop - title_words.size()] << " ";
                }
            }
            std::cout << "\n\n";
            classifier_->addRawTrainingData(training);
        }
    }

    void classify(string_view const &title)
    {
        classifier_->calculateOutput({});
    }

  private:
    // Dataset format: id, title, body, tags
    cdmh::data_processing::dataset const &ds_;

    // train a bayesian classifier
    std::unique_ptr<BayesianClassifier> classifier_;

    std::vector<string_view>   title_words;
    std::map<string_view, int> title_word_indices;
    std::vector<string_view>   tag_words;
    std::map<string_view, int> tag_word_indices;
};

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
    cdmh::data_processing::dataset ds;
#ifdef NDEBUG
    size_t num_rows = 0;
#else
    size_t num_rows = 1000;
#endif
    ds.attach(mmf.get(), mmf.get() + mmf.size(), num_rows);

    std::cout << "\n";
    ds.write_column_info(std::cout);
    std::cout << "\n";

    // use two thirds for training and a third for testing
    size_t const training_rows_begin = 0;
    size_t const training_rows_end   = size_t(ds.rows() * 0.666667);
    size_t const test_rows_begin     = training_rows_end + 1;
    size_t const test_rows_end       = ds.rows();

    classifier bayesian(ds);
    bayesian.train(training_rows_begin, training_rows_end);
    bayesian.classify("Getting rid of site-specific hotkeys");
    bayesian.classify("Remove old vCenter servers from VMWare vSphere Client login window");
	return 0;
}
