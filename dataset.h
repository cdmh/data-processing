#include <algorithm>    // count_if
#include <numeric>      // accumulate

#if _MSC_VER <= 1800
#define noexcept
#endif

namespace cdmh {
namespace data_processing {

class dataset
{
  public:
    explicit dataset(size_t num_columns = 0);
    dataset(dataset &&other) noexcept;
    dataset(dataset const &other)                = delete;  // defaults are not safe because
    dataset &operator=(dataset &&other) noexcept = delete;  // of dynamic memory allocation
    dataset &operator=(dataset const &other)     = delete;  // in the union
    ~dataset();

    class cell_value;
    class column_data;
    class row_data;

    template<typename U> U              at(size_t row, size_t column)         const;
    std::vector<cell_value> const      &at(size_t column)                     const;
    type_mask_t             const       column_type(size_t column)            const;
    column_data                         column(size_t column);
    size_t                  const       columns()                             const;
    void                                clear_column(size_t column);
    template<typename T> std::vector<T> detach_column(size_t column);
    void                                erase_column(size_t column);
    template<typename T> std::vector<T> extract_column(size_t column);
    row_data                            row(size_t row)                       const;
    size_t                  const       rows()                                const;
    type_mask_t             const       type_at(size_t row, size_t column)    const;
    row_data                            operator[](size_t n)                  const;

    std::function<void (std::pair<string_view, type_mask_t>)>
    create_column(type_mask_t type, std::string const &name);

    class cell_value
    {
      private:
        type_mask_t type_;
        union {
            double       double_;
            size_t       integer_;
            std::string *string_;
        };

      public:
        cell_value(type_mask_t type, double dbl)         : type_(type), double_(dbl)                        { assert(type == double_type  ||  (type == null_type  &&  double_  == 0.0));  }
        cell_value(type_mask_t type, size_t integer)     : type_(type), integer_(integer)                   { assert(type == integer_type ||  (type == null_type  &&  integer_ == 0));    }
        cell_value(type_mask_t type, std::string string) : type_(type), string_(new std::string(std::forward<std::string>(string)))    { assert(type == string_type  ||  type == null_type);                         }

                             type_mask_t   const  type()       const  { return type_;                 }
                             bool          const  is_double()  const  { return type_ == double_type;  }
                             bool          const  is_integer() const  { return type_ == integer_type; }
                             bool          const  is_string()  const  { return type_ == string_type;  }
                             bool          const  is_null()    const  { return type_ == null_type;    }

        template<typename T> T                    get()        const;
        template<>           double               get()        const  { assert(type_ == double_type  ||  type_ == null_type);   return double_;          }
        template<>           std::string          get()        const  { assert(type_ == string_type);                           return *string_;         }
        template<>           char const *         get()        const  { assert(type_ == string_type);                           return string_->c_str(); }
        template<>           std::uint32_t        get()        const  { assert(type_ == integer_type  ||  type_ == null_type);  return integer_;         }

        void clear()
        {
            if (type_ == string_type)
                delete string_;
            memset(this, 0, sizeof(*this));
            type_ = null_type;
        }
    };

    class column_data
    {
      public:
        column_data(dataset &ds, size_t column) : ds_(ds), column_(column)
        { }

        column_data(column_data const &other) : ds_(other.ds_), column_(other.column_)
        { }

        column_data(column_data &&other)                 = delete;
        column_data &operator=(column_data const &other) = delete;
        column_data &operator=(column_data &&other)      = delete;

        // returns the number of non-null values in the column
        size_t const count() const
        {
            auto const &values = ds_.at(column_);
            return std::count_if(
                values.begin(),
                values.end(),
                [](cell_value const &cell) {
                    return !cell.is_null();
                });
        }

        // returns the number of null values in the column
        size_t const count_null() const
        {
            auto const &values = ds_.at(column_);
            return std::count_if(
                values.begin(),
                values.end(),
                [](cell_value const &cell) {
                    return cell.is_null();
                });
        }

        void clear()
        {
            ds_.clear_column(column_);
        }

        template<typename T>
        std::vector<T> detach()
        {
            return ds_.detach_column<T>(column_);
        }

        template<typename T>
        std::vector<T> extract()
        {
            return ds_.extract_column<T>(column_);
        }

        void erase()
        {
            return ds_.erase_column(column_);
        }

        double const mean() const
        {
            assert(ds_.column_type(column_) == integer_type  ||  ds_.column_type(column_) == double_type);

            if (ds_.column_type(column_) == double_type)
                return sum<double>() / count();
            return (double)sum<std::uint32_t>() / count();
        }

        template<typename T>
        T const sum() const
        {
            auto const &values = ds_.at(column_);
            return std::accumulate(
                values.begin(),
                values.end(),
                T(),
                [](T sum, cell_value const &cell) {
                    return sum + cell.get<T>();
                });
        }

      private:
        dataset       &ds_;
        size_t  const  column_;
    };

    class row_data
    {
      public:
        row_data(dataset const &ds, size_t row) : ds_(ds), row_(row) { }
        row_data(row_data const &)            = delete;
        row_data &operator=(row_data const &) = delete;

        class cell;
        cell    operator[](size_t column) const { return cell(ds_, row_, column); }
        size_t  size()                    const { return ds_.columns();           }

        class cell
        {
          public:
            cell(cell const &) = default;
            cell &operator=(cell const &) = delete;

            cell(cell &&other) noexcept : ds_(other.ds_), row_(other.row_), column_(other.column_)
            { }

            cell &operator=(cell &&other) noexcept
            {
                assert(std::addressof(ds_) == std::addressof(other.ds_));
                row_    = other.row_;
                column_ = other.column_;
                return *this;
            }

            template<typename U>    operator U() const  { return ds_.at<U>(row_, column_);   }
            template<typename U>    U get()      const  { return ds_.at<U>(row_, column_);   }
            type_mask_t const type()             const  { return ds_.type_at(row_, column_); }

          protected:
            cell(dataset const &ds,size_t row,size_t column): ds_(ds), row_(row),column_(column)
            { }

            friend row_data;

          private:
            dataset const &ds_;
            size_t         row_;
            size_t         column_;
        };

      private:
        dataset const &ds_;
        size_t  const  row_;
    };

  private:
    void add_column_string_data(size_t index, std::pair<string_view, type_mask_t> value);
    void add_column_double_data(size_t index, std::pair<string_view, type_mask_t> value);
    void add_column_integer_data(size_t index, std::pair<string_view, type_mask_t> value);
    void assert_valid() const;

  private:
    struct column_info
    {
        type_mask_t             type;
        std::string             name;
        std::vector<cell_value> values;
    };

    std::vector<column_info> columns_;

    template<typename E, typename T>
    friend
    std::basic_ostream<E,T> &operator<<(std::basic_ostream<E,T> &o, dataset const &ds);
};

inline dataset::dataset(size_t num_columns)
{
    if (num_columns > 0)
        columns_.reserve(num_columns);
}

inline dataset::dataset(dataset &&other) noexcept
{
    assert_valid();
    std::swap(columns_, other.columns_);
}

inline dataset::~dataset()
{
    assert_valid();
    for (auto &column : columns_)
        if (column.type == string_type)
            for (auto &value : column.values)
                value.clear();
}

template<typename T>
inline
T dataset::at(size_t row, size_t column) const
{
    return columns_[column].values[row].get<T>();
}

inline std::vector<dataset::cell_value> const &dataset::at(size_t column) const
{
    return columns_[column].values;
}

inline
std::function<void (std::pair<string_view, type_mask_t>)>
dataset::create_column(type_mask_t type, std::string const &name)
{
    columns_.push_back(column_info{type, name, std::vector<cell_value>()});

    if (type == string_type)
        return std::bind(&dataset::add_column_string_data, this, columns_.size()-1, std::placeholders::_1);
    else if (type == double_type)
        return std::bind(&dataset::add_column_double_data, this, columns_.size()-1, std::placeholders::_1);

    assert(type == integer_type);
    return std::bind(&dataset::add_column_integer_data, this, columns_.size()-1, std::placeholders::_1);
}

inline dataset::column_data dataset::column(size_t column)
{
    return column_data(*this, column);
}

inline size_t const dataset::columns() const
{
    return columns_.size();
}

inline type_mask_t const dataset::column_type(size_t column) const
{
    return columns_[column].type;
}

inline void dataset::clear_column(size_t column)
{
    for (auto &value : columns_[column].values)
        value.clear();
}

template<typename T>
inline std::vector<T> dataset::extract_column(size_t column)
{
    std::vector<T> result;
    result.reserve(columns_[column].values.size());
    for (auto &value : columns_[column].values)
        result.push_back(value.get<T>());
    return result;
}

template<typename T>
inline std::vector<T> dataset::detach_column(size_t column)
{
    std::vector<T> result = extract_column<T>(column);
    erase_column(column);
    return result;
}

inline void dataset::erase_column(size_t column)
{
    columns_.erase(columns_.begin() + column);
}

inline dataset::row_data dataset::row(size_t row) const
{
    return row_data(*this, row);
}

inline size_t const dataset::rows() const
{
    assert_valid();
    return columns_.size()==0? 0 : columns_[0].values.size();
}

inline dataset::row_data dataset::operator[](size_t n) const
{
    return row(n);
}

inline type_mask_t const dataset::type_at(size_t row, size_t column) const
{
    return columns_[column].values[row].type();
}

inline void dataset::assert_valid() const
{
#ifndef NDEBUG
    if (columns_.size() > 0)
    {
        auto const size = columns_[0].values.size();
        for (size_t loop=1; loop<columns_.size(); ++loop)
            assert(columns_[loop].values.size() == size);
    }
#endif
}

inline void dataset::add_column_string_data(size_t index, std::pair<string_view, type_mask_t> value)
{
    columns_[index].values.push_back(cell_value(value.second, std::string(value.first.begin(), value.first.end())));
}

inline void dataset::add_column_double_data(size_t index, std::pair<string_view, type_mask_t> value)
{
    columns_[index].values.push_back(
        cell_value(
            value.second,
            strtod(value.first.begin(), nullptr)));
}

inline void dataset::add_column_integer_data(size_t index, std::pair<string_view, type_mask_t> value)
{
    columns_[index].values.push_back(
        cell_value(
            value.second,
            size_t(atol(value.first.begin()))));
}


template<typename E, typename T>
std::basic_ostream<E,T> &operator<<(std::basic_ostream<E,T> &o, dataset::row_data::cell const &value)
{
    switch (value.type())
    {
        case string_type:   o << value.get<std::string>();  break;
        case double_type:   o << value.get<double>();       break;
        case integer_type:  o << value.get<size_t>();       break;
        case null_type:                                     break;
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

template<typename E, typename T>
std::basic_ostream<E,T> &operator<<(std::basic_ostream<E,T> &o, dataset const &ds)
{
    bool first = true;
    for (auto const &column : ds.columns_)
    {
        if (first)
            first = false;
        else
            o << ',';
        o << '\"' << column.name << '\"';
    }
    o << "\n";

    for (size_t loop=0; loop<ds.rows(); ++loop)
        o << ds[loop] << "\n";
    return o;
}

}   // namespace data_processing
}   // namespace cdmh
