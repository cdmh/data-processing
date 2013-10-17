#include "stdafx.h"
#include <locale>
#include <map>
#include <memory>
#include <atomic>
#include <thread>
#include "../../data-processing.h"

#define WRITE_PROGRESS    0
#define CALCULATE_STATS   1


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

inline
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

inline
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

inline
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
inline
double const mean(It begin, It end)
{
    using type = typename std::iterator_traits<It>::value_type;

    auto const count = std::distance(begin, end);
    // assert that the cast is safe
    assert(count <= std::numeric_limits<type>::max());
    return sum(begin, end) / type(count);
}


template<typename Words>
inline
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

void create_word_freq_map(
    cdmh::data_processing::dataset const &ds,
    char                           const *column,
    size_t                                rows,
    std::map<string_view, std::uint64_t> &word_map,
    bool                                  ignore_stopwords=true)
{
    // count up the words and
    for (size_t loop=0; loop<rows; ++loop)
        count_words(ds[loop][column].get<string_view>(), word_map, ignore_stopwords);
}

template<typename It, typename Offset>
It advance(It it, Offset offset)
{
    std::advance(it, offset);
    return it;
}

template<typename Map, typename Index>
typename Map::key_type const &
map_key(Map const &map, Index index)
{
    return advance(map.begin(), index)->first;
}

template<class InputIt1, class InputIt2, class OutputIt>
OutputIt set_intersection(InputIt1 first1, InputIt1 last1,
                          InputIt2 first2, InputIt2 last2,
                          OutputIt d_first)
{
    while (first1 != last1 && first2 != last2) {
        if (*first1 < first2->first) {
            ++first1;
        } else  {
            if (!(first2->first < *first1)) {
                *d_first++ = *first1++;
            }
            ++first2;
        }
    }
    return d_first;
}

}               // anonymous namespace


#include "naive-bayes-classifier/src/BayesianClassifier.h"
#include "naive-bayes-classifier/src/ActionClassifier.h"
#include "naive-bayes-classifier/src/Domain.h"

class thread_group : public std::vector<std::thread>
{
  public:
    void join_all()
    {
        for (auto &thread : *this)
        {
            try {
                thread.join();
            }
            catch (std::exception &) {
            }
        }
    }
};

/*
  inspired by http://www.inf.ed.ac.uk/teaching/courses/inf2b/learnnotes/inf2b-learn-note07-2up.pdf
*/
class classifier
{
  public:
    classifier(cdmh::data_processing::dataset const &ds) : ds_(ds)
    {
    }

    void train(size_t training_rows_begin, size_t training_rows_end, bool use_body)
    {
        create_word_freq_map(ds_, "title", training_rows_end-training_rows_begin, title_words_);
        create_word_freq_map(ds_, "tags", training_rows_end-training_rows_begin, tag_words_);
        if (use_body)
            create_word_freq_map(ds_, "body", training_rows_end-training_rows_begin, body_words_);

	    std::vector<Domain> domains;
        for (size_t d=0; d<title_words_.size() + body_words_.size(); ++d)
	        domains.emplace_back(0.0f, 1.0f, 2);  // min, max, number of values
        
        if (tag_words_.size() > std::numeric_limits<int>::max())  // ensure a safe cast
            throw overflow_exception();
        if (tag_words_.size()-1 > std::numeric_limits<float>::max())  // ensure a safe cast
            throw overflow_exception();

        domains.emplace_back(0.0f, (float)(tag_words_.size()-1), (int)tag_words_.size());
        classifier_.reset(new BayesianClassifier(domains));

        process_rows(
            training_rows_begin,
            training_rows_end,
            true,
            [this](size_t, std::vector<float> const &data) {
                classifier_->addRawTrainingData(data);
            });
    }

    void classify(size_t test_rows_begin, size_t test_rows_end)
    {
        auto cores = std::thread::hardware_concurrency();
        auto rows  = test_rows_end - test_rows_begin;
        auto size  = rows / cores;
        size_t offset = 0;

        thread_group threads;

        std::vector<std::pair<size_t, size_t>> results;
        results.resize(cores);
        for (size_t loop=0; loop<cores; ++loop, offset+=size)
        {
            auto begin = test_rows_begin + offset;
            auto end   = begin + size;
            if (loop == cores-1)
                end = test_rows_end;

            threads.emplace_back(
                std::bind(
                    &classifier::classify_partition,
                    this,
                    begin,
                    end,
                    std::ref(results[loop])));
        }
        threads.join_all();

        size_t cumm_success  = 0;
        size_t cumm_expected = 0;
        std::for_each(
            results.cbegin(),
            results.cend(),
            [&cumm_success, &cumm_expected](std::pair<size_t, size_t> const &result) {
                cumm_expected += result.first;
                cumm_success  += result.second;
            });

        std::cout << "\nAccuracy: " << ((cumm_success *100)/cumm_expected) << "% over " << rows << " rows";
    }

    class overflow_exception : public std::runtime_error
    {
      public:
        overflow_exception() : std::runtime_error("Overflow exception")
        { }
    };

  private:
    void classify_partition(size_t test_rows_begin, size_t test_rows_end, std::pair<size_t, size_t> &result)
    {
#if CALCULATE_STATS  &&  !WRITE_PROGRESS
                std::cout << "\nId\tExpected\tSuccess\tMissed\tFalse";
#endif

        size_t cumm_success  = 0;
        size_t cumm_expected = 0;
        process_rows(
            test_rows_begin,
            test_rows_end,
            false,
            [this, &cumm_success, &cumm_expected, test_rows_begin, test_rows_end](size_t row, std::vector<float> const &data) {
                std::vector<int> tag_indices;
                process_words(row, 3, tag_words_, [&tag_indices](int n) { tag_indices.emplace_back(n); });

#if WRITE_PROGRESS  ||  CALCULATE_STATS
                std::sort(tag_indices.begin(), tag_indices.end());
                auto const outputs = classifier_->calculatePossibleOutputs(data);
#endif

#if WRITE_PROGRESS
                std::cout << "\nExpected        : ";
                for (auto index : tag_indices)
                    std::cout << map_key(tag_words_, index) << " (" << index << ") ";

                std::cout << "\nActual          : ";
                for (auto const &output : outputs)
                    std::cout << map_key(tag_words_, output.first) << " [" << std::setprecision(3) <<  (100*output.second) << "%] ";
#endif

#if CALCULATE_STATS
                std::vector<int> correct;
                ::set_intersection(
                    tag_indices.begin(), tag_indices.end(),
                    outputs.begin(), outputs.end(),
                    std::back_inserter(correct));

                auto const expected        = tag_indices.size();
                auto const success         = correct.size();
                auto const missed          = tag_indices.size() - correct.size();
                auto const false_positives = outputs.size() - correct.size();
                auto const rate            = (expected==success) ? 1.0f : float(success) / expected;
                cumm_success += success;
                cumm_expected += expected;
#if WRITE_PROGRESS
                std::cout << "\nSuccess: " << success << "%\t";
                std::cout << "\nMissed: " << missed << "%\t";
                std::cout << "\nFalse: " << false_positives << "%\t";
#else
                std::cout << "\n" << ds_[row]["id"].get<string_view>()
                          << "\t" << std::setw(3) << std::right << expected
                          << "\t" << std::setw(3) << std::right << success
                          << "\t" << std::setw(3) << std::right << missed
                          << "\t" << std::setw(3) << std::right << false_positives
                          << "\t" << std::setw(3) << std::right << (rate * 100);
                if (cumm_success == 0)
                    std::cout << "\t0%";
                else
                    std::cout << "\t" << ((cumm_success*100)/cumm_expected) << "%";
#endif
#endif
            });

        result = std::make_pair(cumm_expected, cumm_success);
    }

    void process_rows(size_t begin, size_t end, bool training, std::function<void (size_t row, std::vector<float> const &)> fn)
    {
        std::vector<float> data;
        auto columns = title_words_.size() + body_words_.size();
        if (training)
            ++columns;
        data.resize(columns);
        for (size_t loop=begin; loop<end; ++loop)
        {
            // reset the vector to contain zeros
            for (float &value : data)
                value = 0.0;

#if WRITE_PROGRESS
            std::cout << "\n\n" << ds_[loop][1].get<string_view>();
            std::cout << "\nTitle:";
#endif
            // !!!binary title does/doesn't contain tag. Count might work better?
            process_words(loop, 1, title_words_, [&data](int n) { data[n] = 1.0; });
            if (body_words_.size() > 0)
                process_words(loop, 2, body_words_, [&data, this](int n) { data[n + title_words_.size()] = 1.0; });

#if WRITE_PROGRESS
            std::cout << "\nTags:";
#endif
            std::vector<int> tag_indices;
            if (training  ||  WRITE_PROGRESS)
                process_words(loop, 3, tag_words_, [&tag_indices](int n) { tag_indices.emplace_back(n); });

#if WRITE_PROGRESS
            std::cout << "\nConsidered Words: ";
            for (size_t loop=0; loop<data.size(); ++loop)
            {
                if (data[loop])
                {
                    if (loop < title_words_.size())
                        std::cout << map_key(title_words_, loop) << " ";
                    else
                    {
                        assert(loop < title_words_.size() + body_words_.size());
                        std::cout << map_key(body_words_, loop - title_words_.size()) << " ";
                    }
                }
            }

            for (auto index : tag_indices)
                std::cout << "\n*** " << std::setw(3) << std::right << index << " " << map_key(tag_words_, index) << " ";
#endif

#if 0 && !defined(NDEBUG)  &&  WRITE_PROGRESS
            std::cout << "\n";
            for (float &value : data)
                std::cout << (int)value;
#endif
            // for train each output
            if (training)
            {
                for (auto const &index : tag_indices)
                {
                    data[data.size()-1] = (float)index;
                    fn(loop, data);
                }
            }
            else
                fn(loop, data);
        }
    }

    void process_words(
        size_t                                row,
        size_t                                column,
        std::map<string_view, std::uint64_t> &word_map,
        std::function<void (int)>             fn)
    {
        process_words(ds_[row][column].get<string_view>(), word_map, fn);
    }

    void process_words(
        string_view                           string,
        std::map<string_view, std::uint64_t> &word_map,
        std::function<void (int)>             fn)
    {
        auto it  = string.begin();
        auto ite = string.end();
        while (it != ite)
        {
            string_view const word = next_word(it, ite);
            if (word.length() > 0)
            {
                auto it = word_map.find(word);
                if (it != word_map.end())
                {
                    auto offset = std::distance(word_map.begin(), it);
                    if (offset > std::numeric_limits<int>::max())  // ensure a safe cast
                        throw overflow_exception();
                    fn((int)offset);
                }
#if WRITE_PROGRESS
                else
                    std::cout << "\nUntrained words is ignored: " << word;
#endif
            }
        }
    }

  private:
    // Dataset format: id, title, body, tags
    cdmh::data_processing::dataset const &ds_;
    std::unique_ptr<BayesianClassifier>   classifier_;
    std::map<string_view, std::uint64_t>  title_words_;
    std::map<string_view, std::uint64_t>  tag_words_;
    std::map<string_view, std::uint64_t>  body_words_;
};

int main()
{
#if defined(_MSC_VER)  &&  defined(_DEBUG)
    _CrtSetDbgFlag(_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG) | _CRTDBG_LEAK_CHECK_DF);
#endif
    srand((unsigned)time(NULL));

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
    size_t num_rows = 2500;
#else
    size_t num_rows = 100;
#endif

    if (num_rows == 0)
    {
        ds.attach(mmf.get(), mmf.get() + mmf.size(), 0);
        num_rows = ds.rows();
    }

    // use two thirds for training and a third for testing
    size_t const training_rows_begin = 0;
    size_t const training_rows_end   = training_rows_begin + size_t(num_rows * 0.666667);
    size_t const test_rows_begin     = training_rows_begin; // training_rows_end;
    size_t const test_rows_end       = training_rows_end; // std::max(training_rows_begin + num_rows, ds.rows());

    // attach to the last of the test rows
    if (!ds.is_attached())
        ds.attach(mmf.get(), mmf.get() + mmf.size(), test_rows_end);

    std::cout << "\n";
    ds.write_column_info(std::cout);
    std::cout << "\n";

    classifier bayesian(ds);
    std::cout << "\nTraining ...";
    bayesian.train(training_rows_begin, training_rows_end, false);

    std::cout << "\nClassifying ...";
    bayesian.classify(test_rows_begin, test_rows_end);

    std::cout << "\n";
	return 0;
}
