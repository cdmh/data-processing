// Copyright (c) 2013 Craig Henderson
// Part of the Data Processing Library
// https://github.com/cdmh/dataproc

#include <algorithm>        // count_if
#include <numeric>          // accumulate
#include "maths.h"

#if _MSC_VER <= 1800
#define noexcept
#endif

namespace cdmh {
namespace data_processing {

class invalid_column_name : public std::runtime_error
{
  public:
    invalid_column_name() : std::runtime_error("Invalid column name")
    { }
};

class dataset
{
  public:
    explicit dataset(size_t num_columns = 0);
    dataset(dataset &&other) noexcept;
    dataset(dataset const &other)                = delete;  // defaults are not safe because
    dataset &operator=(dataset &&other) noexcept = delete;  // of dynamic memory allocation
    dataset &operator=(dataset const &other)     = delete;  // in the union

    class cell_value;
    class column_data;
    class row_data;

    template<typename U> U              at(size_t row, int column)                const;
    std::vector<cell_value> const      &at(int column)                            const;
    type_mask_t             const       column_type(int column)                   const;
    column_data                         column(int column);
    column_data                         column(char const *name);
    size_t                  const       columns()                                    const;
    void                                clear_column(int column);
    template<typename T> std::vector<T> detach_column(int column);
    void                                erase_column(int column);
    template<typename T> std::vector<T> extract_column(int column, bool include_nulls=true);
    size_t                  const       lookup_column(char const *name)              const;
    row_data                            row(size_t row)                              const;
    size_t                  const       rows()                                       const;
    void                                swap_columns(int column1, int column2);
    type_mask_t             const       type_at(size_t row, int column)           const;
    row_data                            operator[](size_t n)                         const;

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
        cell_value(cell_value &&other)
        {
            memcpy(this, &other, sizeof(*this));
            memset(&other, 0, sizeof(*this));
        }

        cell_value(cell_value const &other)
        {
            *this = other;
        }

        cell_value &operator=(cell_value const &other)
        {
            memcpy(this, &other, sizeof(*this));
            if (is_string())
                string_ = new std::string(*other.string_);
            return *this;
        }

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

        ~cell_value()
        {
            clear();
        }

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
        column_data(dataset &ds, int column) : ds_(ds), column_(column)
        { }

        column_data(dataset &ds, char const *name) : ds_(ds), column_(ds_.lookup_column(name))
        { }

        column_data(column_data const &other) : ds_(other.ds_), column_(other.column_)
        { }

        column_data(column_data &&other)                 = delete;
        column_data &operator=(column_data const &other) = delete;
        column_data &operator=(column_data &&other)      = delete;

                             void           clear()          { ds_.clear_column(column_);                }
        template<typename T> std::vector<T> detach()         { return ds_.detach_column<T>(column_);     }
                             void           erase()          { return ds_.erase_column(column_);         }
        template<typename T> std::vector<T> extract()        { return ds_.extract_column<T>(column_);    }
                             void           swap(int column) { return ds_.swap_columns(column_, column); }

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

        size_t const count_unique() const
        {
            assert(ds_.column_type(column_) == integer_type  ||  ds_.column_type(column_) == double_type);

            if (ds_.column_type(column_) == double_type)
                return count_unique<double>();
            return count_unique<std::uint32_t>();
        }

        template<typename T>
        size_t const count_unique() const
        {
            std::unordered_map<T, unsigned> counts;
            for (auto const &value : ds_.at(column_))
                if (!value.is_null())
                    counts[value.get<T>()]++;
            return counts.size();
        }

        double const mean() const
        {
            assert(ds_.column_type(column_) == integer_type  ||  ds_.column_type(column_) == double_type);

            if (ds_.column_type(column_) == double_type)
                return sum<double>() / count();
            return (double)sum<std::uint32_t>() / count();
        }

        double const median() const
        {
            assert(ds_.column_type(column_) == integer_type  ||  ds_.column_type(column_) == double_type);

            if (ds_.column_type(column_) == double_type)
                return maths::median(ds_.extract_column<double>(column_, false));
            return (double)maths::median(ds_.extract_column<std::uint32_t>(column_, false));
        }

        double const mode() const
        {
            assert(ds_.column_type(column_) == integer_type  ||  ds_.column_type(column_) == double_type);

            if (ds_.column_type(column_) == double_type)
                return maths::mode(ds_.extract_column<double>(column_, false));
            return (double)maths::mode(ds_.extract_column<std::uint32_t>(column_, false));
        }

        double const standard_deviation() const
        {
            assert(ds_.column_type(column_) == integer_type  ||  ds_.column_type(column_) == double_type);

            if (ds_.column_type(column_) == double_type)
                return maths::standard_deviation(ds_.extract_column<double>(column_, false));
            return maths::standard_deviation(ds_.extract_column<std::uint32_t>(column_, false));
        }

        template<typename T> T max() const
        {
            assert(ds_.column_type(column_) == integer_type  ||  ds_.column_type(column_) == double_type);

            T max = std::numeric_limits<T>::min();
            for (auto const &value : ds_.at(column_))
            {
                if (!value.is_null())
                {
                    T val = value.get<T>();
                    if (val > max)
                        max = val;
                }
            }
            return max;
        }

        template<typename T> T min() const
        {
            assert(ds_.column_type(column_) == integer_type  ||  ds_.column_type(column_) == double_type);

            T min = std::numeric_limits<T>::max();
            for (auto const &value : ds_.at(column_))
            {
                if (!value.is_null())
                {
                    T val = value.get<T>();
                    if (val < min)
                        min = val;
                }
            }
            return min;
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
        cell    operator[](int column)       const { return cell(ds_, row_, column); }
        cell    operator[](char const *name) const { return cell(ds_, row_, ds_.lookup_column(name)); }
        size_t  size()                       const { return ds_.columns();           }

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
            cell(dataset const &ds,size_t row,int column): ds_(ds), row_(row),column_(column)
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

template<typename T>
inline
T dataset::at(size_t row, int column) const
{
    return columns_[column].values[row].get<T>();
}

inline std::vector<dataset::cell_value> const &dataset::at(int column) const
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

inline dataset::column_data dataset::column(int column)
{
    return column_data(*this, column);
}

inline dataset::column_data dataset::column(char const *name)
{
    return column_data(*this, name);
}

inline size_t const dataset::columns() const
{
    return columns_.size();
}

inline type_mask_t const dataset::column_type(int column) const
{
    return columns_[column].type;
}

inline void dataset::clear_column(int column)
{
    for (auto &value : columns_[column].values)
        value.clear();
}

template<typename T>
inline std::vector<T> dataset::extract_column(int column, bool include_nulls)
{
    std::vector<T> result;
    result.reserve(columns_[column].values.size());
    for (auto &value : columns_[column].values)
        if (include_nulls  ||  !value.is_null())
            result.push_back(value.get<T>());
    return result;
}

template<typename T>
inline std::vector<T> dataset::detach_column(int column)
{
    std::vector<T> result = extract_column<T>(column);
    erase_column(column);
    return result;
}

inline void dataset::erase_column(int column)
{
    columns_.erase(columns_.begin() + column);
}

inline void dataset::swap_columns(int column1, int column2)
{
    std::swap(columns_[column1], columns_[column2]);
}

inline size_t const dataset::lookup_column(char const *name) const
{
    size_t index = 0;
    for (auto const &column : columns_)
    {
        if (column.name == name)
            return index;
        ++index;
    }
    throw invalid_column_name();
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

inline type_mask_t const dataset::type_at(size_t row, int column) const
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
    columns_[index].values.emplace_back(value.second, std::string(value.first.begin(), value.first.end()));
}

inline void dataset::add_column_double_data(size_t index, std::pair<string_view, type_mask_t> value)
{
    columns_[index].values.emplace_back(
        value.second,
        strtod(value.first.begin(), nullptr));
}

inline void dataset::add_column_integer_data(size_t index, std::pair<string_view, type_mask_t> value)
{
    columns_[index].values.emplace_back(
        value.second,
        size_t(atol(value.first.begin())));
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
