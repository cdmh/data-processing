// Copyright (c) 2013 Craig Henderson
// Part of the Data Processing Library
// https://github.com/cdmh/dataproc

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
    dataset &operator=(dataset const &other)     = delete;  // in the union

    dataset &operator=(dataset &&other) noexcept
    {
        std::swap(columns_, other.columns_);
        return *this;
    }

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
    bool                    const       import_csv(char const *filename);
    size_t                  const       lookup_column(char const *name)              const;
    row_data                            row(size_t row)                              const;
    size_t                  const       rows()                                       const;
    void                                swap_columns(int column1, int column2);
    type_mask_t             const       type_at(size_t row, int column)           const;
    row_data                            operator[](size_t n)                         const;

    std::function<void (std::pair<string_view, type_mask_t>)>
    create_column(type_mask_t type, std::string const &name);

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


class dataset::cell_value
{
  public:
    cell_value(cell_value &&other);
    cell_value(cell_value const &other);
    cell_value &operator=(cell_value const &other);

    cell_value(type_mask_t type, double dbl);
    cell_value(type_mask_t type, size_t integer);
    cell_value(type_mask_t type, std::string string);

    type_mask_t   const  type()       const;
    bool          const  is_double()  const;
    bool          const  is_integer() const;
    bool          const  is_string()  const;
    bool          const  is_null()    const;
    template<typename T> T get()      const;

    ~cell_value();
    void clear();

  private:
    type_mask_t type_;
    union {
        double       double_;
        size_t       integer_;
        std::string *string_;
    };
};


class dataset::column_data
{
  public:
    column_data(dataset &ds, int column);
    column_data(dataset &ds, char const *name);
    column_data(column_data const &other);

    column_data(column_data &&other)                 = delete;
    column_data &operator=(column_data const &other) = delete;
    column_data &operator=(column_data &&other)      = delete;

                            void           clear();
                            size_t const   count()              const;
                            size_t const   count_null()         const;
                            size_t const   count_unique()       const;
    template<typename T>    size_t const   count_unique()       const;
    template<typename T>    std::vector<T> detach();
                            void           erase();
    template<typename T>    std::vector<T> extract();
    template<typename T>    T              max()                const;
    template<typename T>    T              min()                const;
                            double const   mean()               const;
                            double const   median()             const;
                            double const   mode()               const;
                            size_t const   size()               const;
    template<typename T>    T const        sum()                const;
                            double const   standard_deviation() const;
                            void           swap(int column);

  private:
    dataset       &ds_;
    size_t  const  column_;
};


class dataset::row_data
{
  public:
    row_data(dataset const &ds, size_t row);
    row_data(row_data const &)            = delete;
    row_data &operator=(row_data const &) = delete;

    class cell;
    cell    operator[](int column)       const;
    cell    operator[](char const *name) const;
    size_t  size()                       const;


  private:
    dataset const &ds_;
    size_t  const  row_;
};


class dataset::row_data::cell
{
  public:
    cell(cell const &) = default;
    cell &operator=(cell const &) = delete;

    cell(cell &&other)            noexcept;
    cell &operator=(cell &&other) noexcept;

    template<typename U>    operator U()             const;
    template<typename U>    U                 get()  const;
                            type_mask_t const type() const;

  protected:
    cell(dataset const &ds,size_t row,int column);

    friend row_data;

    private:
    dataset const &ds_;
    size_t         row_;
    size_t         column_;
};

}   // namespace data_processing
}   // namespace cdmh
