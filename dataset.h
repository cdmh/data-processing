namespace cdmh {
namespace data_processing {

class dataset
{
  public:
    struct cell_value
    {
        type_mask_t type_;
        union {
            double      double_;
            size_t      integer_;
            char const *string_;
        };

        cell_value(type_mask_t type, double dbl)         : type_(type), double_(dbl)      { assert(type == null_type  ||  type == double_type);  }
        cell_value(type_mask_t type, size_t integer)     : type_(type), integer_(integer) { assert(type == null_type  ||  type == integer_type); }
        cell_value(type_mask_t type, char const *string) : type_(type), string_(string)   { assert(type == null_type  ||  type == string_type);  }
    };

    explicit dataset(size_t num_columns = 0);
    dataset(dataset &&other);
    ~dataset();
    dataset(dataset const &other)            = delete;  // defaults are not safe because
    dataset &operator=(dataset &&other)      = delete;  // of dynamic memory allocation
    dataset &operator=(dataset const &other) = delete;  // in the union

    size_t      const columns()                  const;
    size_t      const rows()                     const;
    type_mask_t const column_type(size_t column) const;

    template<typename U>
    U at(size_t row, size_t column) const;

    type_mask_t const type_at(size_t row, size_t column) const;

    std::function<void (std::pair<type_mask_t, std::pair<char const *, char const *>>)>
    create_column(type_mask_t type);

    class row_data
    {
      public:
        class cell
        {
          public:
            cell(cell const &) = default;
            cell &operator=(cell &&other) = delete;
            cell &operator=(cell const &) = delete;

            template<typename U>
            operator U() const
            {
                return ds_.at<U>(row_, column_);
            }

            type_mask_t const type() const
            {
                return ds_.type_at(row_, column_);
            }

          protected:
            cell(dataset const &ds,size_t row,size_t column): ds_(ds), row_(row),column_(column)
            {
            }

          private:
            dataset const &ds_;
            size_t         row_;
            size_t         column_;
        };

      private:
        class cell_constructor : public cell
        {
          public:
            cell_constructor(dataset const &ds,size_t row,size_t column) : cell(ds, row, column)
            {
            }
        };

      public:
        row_data(dataset const &ds, size_t row) : ds_(ds), row_(row) { }
        row_data(row_data const &)            = delete;
        row_data &operator=(row_data const &) = delete;

        cell operator[](size_t column) const
        {
            return cell_constructor(ds_, row_, column);
        }

        size_t size() const
        {
            return ds_.columns();
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
    void add_column_string_data(size_t index, std::pair<type_mask_t, std::pair<char const *, char const *>> value);
    void add_column_double_data(size_t index, std::pair<type_mask_t, std::pair<char const *, char const *>> value);
    void add_column_integer_data(size_t index, std::pair<type_mask_t, std::pair<char const *, char const *>> value);
    void assert_valid() const;

  private:
    std::vector<
        std::pair<
            type_mask_t,            // column type
            std::vector<cell_value> // values
        >
    > columns_;
};

inline dataset::dataset(size_t num_columns)
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

inline size_t const dataset::columns() const
{
    return columns_.size();
}

inline size_t const dataset::rows() const
{
    assert_valid();
    return columns_.size()==0? 0 : columns_[0].second.size();
}

inline type_mask_t const dataset::column_type(size_t column) const
{
    return columns_[column].first;
}

template<>
char const *dataset::at(size_t row, size_t column) const
{
    assert(type_at(row, column) ==string_type);
    return columns_[column].second[row].string_;
}

template<>
double dataset::at(size_t row, size_t column) const
{
    assert(type_at(row, column) == double_type);
    return columns_[column].second[row].double_;
}

template<>
size_t dataset::at(size_t row, size_t column) const
{
    assert(type_at(row, column) ==integer_type);
    return columns_[column].second[row].integer_;
}

type_mask_t const dataset::type_at(size_t row, size_t column) const
{
    return columns_[column].second[row].type_;
}

inline void dataset::add_column_string_data(size_t index, std::pair<type_mask_t, std::pair<char const *, char const *>> value)
{
    auto length = std::distance(value.second.first, value.second.second);
    char *string = new char[length+1];
    strncpy(string, value.second.first, length);
    string[length] = 0;;
    columns_[index].second.push_back(cell_value(value.first, string));
}

inline void dataset::add_column_double_data(size_t index, std::pair<type_mask_t, std::pair<char const *, char const *>> value)
{
    double d = strtod(value.second.first, nullptr);
    columns_[index].second.push_back(cell_value(value.first, d));
}

inline void dataset::add_column_integer_data(size_t index, std::pair<type_mask_t, std::pair<char const *, char const *>> value)
{
    size_t n = atol(value.second.first);
    columns_[index].second.push_back(cell_value(value.first, n));
}

inline void dataset::assert_valid() const
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
std::function<void (std::pair<type_mask_t, std::pair<char const *, char const *>>)>
dataset::create_column(type_mask_t type)
{
    columns_.push_back(std::make_pair(type, std::vector<cell_value>()));

    if (type == string_type)
        return std::bind(&dataset::add_column_string_data, this, columns_.size()-1, std::placeholders::_1);
    else if (type == double_type)
        return std::bind(&dataset::add_column_double_data, this, columns_.size()-1, std::placeholders::_1);

    assert(type == integer_type);
    return std::bind(&dataset::add_column_integer_data, this, columns_.size()-1, std::placeholders::_1);
}


template<typename E, typename T>
std::basic_ostream<E,T> &operator<<(std::basic_ostream<E,T> &o, dataset::row_data::cell const &value)
{
    switch (value.type())
    {
        case string_type:   o << (char const *)value;   break;
        case double_type:   o << (double)value;         break;
        case integer_type:  o << (size_t)value;         break;
        case null_type:                                 break;
        default:            assert(!"Unknown value type");
    }
    return o;
}

template<typename E, typename T>
std::basic_ostream<E,T> &operator<<(std::basic_ostream<E,T> &o, dataset::row_data const &row)
{
    bool first = true;
    for (size_t loop=0; loop<row.size(); ++loop)
    {
        if (first)
            first = false;
        else
            o << ',';

        auto const &value = row[loop];
        if (value.type() == string_type)
            o << '\"' << value << '\"';
        else
            o << value;
    }
    return o;
}

}   // namespace data_processing
}   // namespace cdmh
