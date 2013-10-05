namespace cdmh {
namespace data_processing {

class dataset
{
  public:
    typedef union value
    {
        double        double_;
        std::uint32_t integer_;
        char const   *string_;

        value(double dbl)            : double_(dbl)      { }
        value(std::uint32_t integer) : integer_(integer) { }
        value(char const *string)    : string_(string)   { }
    } value_t;

    explicit dataset(std::uint32_t num_columns = 0);
    dataset(dataset &&other);
    ~dataset();
    dataset(dataset const &other)            = delete;  // defaults are not safe because
    dataset &operator=(dataset &&other)      = delete;  // of dynamic memory allocation
    dataset &operator=(dataset const &other) = delete;  // in the union

    size_t        const columns(void)              const;
    size_t        const rows(void)                 const;
    std::uint32_t const column_type(size_t column) const;

    template<typename U>
    U at(size_t row, size_t column) const;

    std::function<void (std::pair<char const *, char const *> value)>
    create_column(std::uint32_t type);

    class row_data
    {
      public:
        class value
        {
          public:
            value(value const &) = default;
            value &operator=(value &&other) = delete;
            value &operator=(value const &) = delete;

            template<typename U>
            operator U() const
            {
                return ds_.at<U>(row_, column_);
            }

            std::uint32_t const type(void) const
            {
                return ds_.column_type(column_);
            }

          protected:
            value(dataset const &ds,size_t row,size_t column): ds_(ds), row_(row),column_(column)
            {
            }

          private:
            dataset const &ds_;
            size_t         row_;
            size_t         column_;
        };

      private:
        class value_constructor : public value
        {
          public:
            value_constructor(dataset const &ds,size_t row,size_t column) : value(ds, row, column)
            {
            }
        };

      public:
        row_data(dataset const &ds, size_t row) : ds_(ds), row_(row) { }
        row_data(row_data const &)            = delete;
        row_data &operator=(row_data const &) = delete;

        value operator[](size_t column) const
        {
            return value_constructor(ds_, row_, column);
        }

      private:
        dataset const &ds_;
        size_t  const  row_;
    };

    row_data operator[](size_t row) const
    {
        return row_data(*this, row);
    }

  private:
    void add_column_string_data(std::uint32_t index, std::pair<char const *,char const *> value);
    void add_column_double_data(std::uint32_t index, std::pair<char const *, char const *> value);
    void add_column_integer_data(std::uint32_t index, std::pair<char const *, char const *> value);
    void assert_valid(void) const;

  private:
    std::vector<
        std::pair<
            std::uint32_t,          // column type
            std::vector<value_t>    // values
        >
    > columns_;
};

inline dataset::dataset(std::uint32_t num_columns)
{
    if (num_columns > 0)
        columns_.reserve(num_columns);
}

inline dataset::dataset(dataset &&other)
{
    assert_valid();
    std::swap(columns_, other.columns_);
}

inline dataset::~dataset()
{
    assert_valid();
    for (auto &column : columns_)
        if (column.first == string_type)
            for (auto &value : column.second)
                delete[] value.string_;
}

inline size_t const dataset::columns(void) const
{
    return columns_.size();
}

inline size_t const dataset::rows(void) const
{
    assert_valid();
    return columns_.size()==0? 0 : columns_[0].second.size();
}

inline std::uint32_t const dataset::column_type(size_t column) const
{
    return columns_[column].first;
}

template<>
char const *dataset::at(size_t row, size_t column) const
{
    assert(columns_[column].first == string_type);
    return columns_[column].second[row].string_;
}

template<>
double dataset::at(size_t row, size_t column) const
{
    assert(columns_[column].first == double_type);
    return columns_[column].second[row].double_;
}

template<>
std::uint32_t dataset::at(size_t row, size_t column) const
{
    assert(columns_[column].first == integer_type);
    return columns_[column].second[row].integer_;
}

inline void dataset::add_column_string_data(std::uint32_t index, std::pair<char const *,char const *> value)
{
    auto length = std::distance(value.first, value.second);
    char *string = new char[length+1];
    strncpy(string, value.first, length);
    string[length] = 0;;
    columns_[index].second.push_back(string);
}

inline void dataset::add_column_double_data(std::uint32_t index, std::pair<char const *, char const *> value)
{
    double d = strtod(value.first, nullptr);
    columns_[index].second.push_back(d);
}

inline void dataset::add_column_integer_data(std::uint32_t index, std::pair<char const *, char const *> value)
{
    std::uint32_t n = atol(value.first);
    columns_[index].second.push_back(n);
}

inline void dataset::assert_valid(void) const
{
#ifndef NDEBUG
    if (columns_.size() > 0)
    {
        auto const size = columns_[0].second.size();
        for (size_t loop=1; loop<columns_.size(); ++loop)
            assert(columns_[loop].second.size() == size);
    }
#endif
}

inline
std::function<void (std::pair<char const *, char const *> value)>
dataset::create_column(std::uint32_t type)
{
    columns_.push_back(std::make_pair(type, std::vector<value_t>()));

    if (type == string_type)
        return std::bind(&dataset::add_column_string_data, this, columns_.size()-1, std::placeholders::_1);
    else if (type == double_type)
        return std::bind(&dataset::add_column_double_data, this, columns_.size()-1, std::placeholders::_1);

    assert(type == integer_type);
    return std::bind(&dataset::add_column_integer_data, this, columns_.size()-1, std::placeholders::_1);
}


template<typename E, typename T>
std::basic_ostream<E,T> &operator<<(std::basic_ostream<E,T> &o, dataset::row_data::value const &value)
{
    switch (value.type())
    {
        case string_type:   o << (char const *)value;    break;
        case double_type:   o << (double)value;          break;
        case integer_type:  o << (std::uint32_t)value;   break;
        default:            assert(!"Unknown value type");
    }
    return o;
}

}   // namespace data_processing
}   // namespace cdmh
