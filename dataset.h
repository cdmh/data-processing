// Copyright (c) 2013 Craig Henderson
// Part of the Data Processing Library
// https://github.com/cdmh/dataproc

namespace cdmh {

namespace data_processing {

class dataset
{
  public:
    class cell_value;
    class column_data;
    class row_data;

    class invalid_column_name : public std::runtime_error
    {
      public:
        invalid_column_name() : std::runtime_error("Invalid column name")
        { }
    };

    template<typename It>
    bool const attach(It begin, It end, std::uint64_t max_records=0);
    bool const attach(char const *data, std::uint64_t max_records=0);

    row_data                            operator[](size_t n)               const;
    template<typename U> U              at(size_t row, size_t column)      const;
    cell_value              const      &cell(size_t row, size_t column)    const;
    std::vector<cell_value> const      &cells(size_t column)               const;
    column_data                         column(int n)                      const;
    column_data                         column(size_t column)              const;
    column_data                         column(char const *name)           const;
    size_t                  const       columns()                          const;
    type_mask_t             const       column_type(size_t column)         const;
    std::string                         column_title(size_t column)        const;
    void                                erase_column(size_t column);
    template<typename T> std::vector<T> extract_column(size_t column, bool include_nulls=false) const;
    bool                    const       is_attached()                      const;
    size_t                  const       lookup_column(char const *name)    const;
    row_data                            row(size_t row)                    const;
    size_t                  const       rows()                             const;
    void                                write_column_info(std::ostream &o) const;

  private:
    void create_column(unsigned index, string_view const &name, type_mask_t /*type*/);
    void store_field(unsigned index, string_view const &value, type_mask_t type);

    template<typename It, typename Fn>
    bool const process_record(It &begin, It end, Fn fn);

  private:
    typedef std::pair<string_view, type_mask_t> column_info_t;
    typedef std::vector<cell_value>             string_list_t;
    std::vector<column_info_t> column_info_;
    std::vector<string_list_t> column_values_;

    template<typename E, typename T>
    friend
    std::basic_ostream<E,T> &operator<<(std::basic_ostream<E,T> &o, dataset const &dd);

};

class dataset::row_data
{
  public:
    row_data(dataset const &ds, size_t row);
    row_data(row_data const &)            = delete;
    row_data &operator=(row_data const &) = delete;

    class cell_reference;
    cell_reference operator[](int column)       const;
    cell_reference operator[](size_t column)    const;
    cell_reference operator[](char const *name) const;
    size_t         size()                       const;


  private:
    dataset const &dd_;
    size_t  const  row_;
};

class dataset::row_data::cell_reference
{
  public:
    cell_reference(cell_reference const &) = default;
    cell_reference &operator=(cell_reference const &) = delete;

    cell_reference(cell_reference &&other)            noexcept;
    cell_reference &operator=(cell_reference &&other) = delete;

    template<typename U>    operator U()                const;
    template<typename U>    U                 get()     const;
                            bool const        is_null() const;
                            type_mask_t const type()    const;

  protected:
    cell_reference(dataset const &ds,size_t row,size_t column);

    friend row_data;

    private:
    dataset const &dd_;
    size_t  const  row_;
    size_t  const  column_;
};

class dataset::cell_value
{
  public:
    explicit cell_value(string_view string);

    type_mask_t   const  type()       const;
    bool          const  is_double()  const { return type() == double_type;  }
    bool          const  is_integer() const { return type() == integer_type; }
    bool          const  is_string()  const { return type() == string_type;  }
    bool          const  is_null()    const { return string_.length() == 0;  }
    template<typename T> T get()      const;

  private:
    string_view string_;
};

class dataset::column_data
{
  public:
    column_data(dataset const &ds, size_t column);
    column_data(dataset const &ds, char const *name);
    column_data(column_data const &other);

    column_data(column_data &&other)                 = delete;
    column_data &operator=(column_data const &other) = delete;
    column_data &operator=(column_data &&other)      = delete;

                            size_t const   count()              const;
                            size_t const   count_null()         const;
                            size_t const   count_unique()       const;
    template<typename T>    size_t const   count_unique()       const;
    template<typename T>    std::vector<T> extract()            const;
                            bool   const   is_double()          const { return dd_.column_type(column_) == double_type;  }
                            bool   const   is_integer()         const { return dd_.column_type(column_) == integer_type; }
                            bool   const   is_string()          const { return dd_.column_type(column_) == string_type;  }
    template<typename T>    T              max()                const;
    template<typename T>    T              min()                const;
                            double const   mean()               const;
                            double const   median()             const;
                            double const   mode()               const;
                            size_t const   size()               const;
    template<typename T>    T const        sum()                const;
                            double const   standard_deviation() const;

  private:
    dataset const &dd_;
    size_t  const  column_;
};

}   // namespace data_processing
}   // namespace cdmh
