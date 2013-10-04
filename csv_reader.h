#include <string>
#include <vector>
#include <fstream>
#include <cstdint>
#include <cctype>
#include <cassert>

namespace cdmh {

class dataset {};

class csv_reader
{
  public:
    static std::uint32_t const string_type  = 1;
    static std::uint32_t const double_type  = 1 << 1;
    static std::uint32_t const integer_type = 1 << 2;

    dataset read(char const * const filename)
    {
        std::ifstream file(filename, std::ios_base::in);
        std::string record;
        while (!file.eof()  &&  !file.fail())
        {
            getline(file, record);
            if (!process_record(record.begin(), record.end()))
                break;
        }
        return dataset_;
    }

    static
    std::pair<std::pair<char const *, char const *>, std::uint32_t>
    read_field(char const *record)
    {
        return read_field(record, record+strlen(record));
    }

  protected:
    template<typename It>
    static
    std::pair<std::pair<It, It>, std::uint32_t>
    read_field(It &begin, It end)
    {
        assert(begin != end);

        bool in_quote  = false;
        if (*begin == '\"')
        {
            in_quote = true;
            ++begin;
        }

        auto it = begin;
        std::uint32_t inc_type_mask  = string_type;
        std::uint32_t excl_type_mask = 0;

        // special case for unary operators
        if (*it == '-'  ||  *it == '+')
        {
            inc_type_mask |= double_type | integer_type;
            ++it;
        }

        bool expect_esc  = false;
        bool seen_period = false;
        for (; it!=end  &&  *it != ','  &&  !(in_quote  &&  *it == '\"'  &&  !expect_esc); ++it)
        {
            if (std::isdigit(*it))
                inc_type_mask |= double_type | integer_type;
            else if (*it == '.')
            {
                if (seen_period)
                    excl_type_mask |= double_type;
                else
                {
                    inc_type_mask |= double_type;
                    excl_type_mask |= integer_type;
                    seen_period = true;
                }
            }
            else
            {
                excl_type_mask |= double_type | integer_type;
                if (!expect_esc  &&  *it == '\\')
                    expect_esc = true;
                else
                    expect_esc = false;
            }
        }

        // precedences
        inc_type_mask &= ~excl_type_mask;
        if (inc_type_mask & integer_type)   // prefer integer to double and string
            inc_type_mask &= ~(double_type | string_type);
        if (inc_type_mask & double_type)    // prefer double to string
            inc_type_mask &= ~string_type;

        assert(inc_type_mask == integer_type  ||  inc_type_mask == double_type  ||  inc_type_mask == string_type);
        auto result = std::make_pair(std::pair<It, It>(begin, it), inc_type_mask);

        // update returning 'begin' iterator to the start next field
        if (in_quote)
        {
            assert(*it == '\"');
            ++it;
        }
        assert(it == end  ||  *it == ',');
        begin = it;
        return result;
    }

    template<typename It>
    bool const process_record(It begin, It end)
    {
        It it = begin;
        while (it != end)
        {
            read_field(it, end);
        }

        return true;
    }

  private:
    std::vector<std::string> column_names_;
    dataset dataset_;
};

dataset read_csv(char const * const filename);
dataset read_csv(std::string const &filename);

dataset read_csv(std::string const &filename)
{
    return read_csv(filename.c_str());
}

dataset read_csv(char const * const filename)
{
    csv_reader reader;
    return reader.read(filename);
}

}   // namespace cdmh