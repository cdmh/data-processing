#include "mapped_csv.detail.h"

namespace cdmh {

namespace data_processing {

class delimited_data
{
  public:
    delimited_data(): record_count_(0) { }

    template<typename It>
    bool const attach(It begin, It end, std::uint64_t max_records=0);

    bool const attach(char const *data, std::uint64_t max_records=0);

    dataset             create_dataset(bool destructive = true);
    std::uint64_t const size() const;

  protected:
    void create_column(unsigned index, string_view &name, type_mask_t /*type*/);
    void store_field(unsigned index, string_view &value, type_mask_t type);

    template<typename It, typename Fn>
    bool const process_record(It &begin, It end, Fn fn);

  private:
    typedef std::pair<string_view, type_mask_t> column_info_t;

    std::uint64_t              record_count_;
    std::vector<column_info_t> column_info_;
    std::vector<type_mask_t>   incl_type_mask_;

    typedef std::vector<std::pair<string_view, type_mask_t>> string_list_t;
    std::vector<string_list_t> column_values_;
};

template<typename It>
inline
bool const delimited_data::attach(It begin, It end, std::uint64_t max_records)
{
    typedef 
    std::function<void (unsigned, string_view &, type_mask_t)>
    store_fn_t;

    using std::placeholders::_1;
    using std::placeholders::_2;
    using std::placeholders::_3;
    store_fn_t store         = std::bind(&delimited_data::create_column, this, _1, _2, _3);
    store_fn_t store_fields  = std::bind(&delimited_data::store_field, this, _1, _2, _3);

    while (begin != end  &&  (max_records == 0  ||  size() < max_records))
    {
        auto eol = std::find_if(begin, end, [](char ch) { return ch == '\r'  ||  ch == '\n'; });
        process_record(begin, eol, store);
        assert(begin == eol);
        detail::ltrim(begin, end);
        store = store_fields;
    }

    return true;
}

inline bool const delimited_data::attach(char const *data, std::uint64_t max_records)
{
    return attach(data, data+strlen(data), max_records);
}


inline void delimited_data::create_column(unsigned index, string_view &name, type_mask_t /*type*/)
{
#ifdef NDEBUG
    index;
#else
    assert(index == column_info_.size());
#endif
    column_info_.push_back(column_info_t(name,0));
    column_values_.push_back(string_list_t());
}

inline dataset delimited_data::create_dataset(bool destructive)
{
    dataset ds(column_info_.size());
    for (unsigned index=0; index<column_info_.size(); ++index)
    {
        auto inserter = ds.create_column(
            column_info_[index].second,
            std::string(
                column_info_[index].first.begin(),
                column_info_[index].first.end()));

        for (auto value : column_values_[index])
            inserter(value);

        if (destructive)
            string_list_t().swap(column_values_[index]);
    }

    if (destructive)
    {
        record_count_ = 0;
        std::vector<column_info_t>().swap(column_info_);
        std::vector<type_mask_t>().swap(incl_type_mask_);
        std::vector<string_list_t>().swap(column_values_);
    }

    return ds;
}

template<typename It, typename Fn>
inline
bool const delimited_data::process_record(It &begin, It end, Fn fn)
{
    for (unsigned index=0; begin!=end; ++index)
    {
        auto field = detail::read_field(begin, end);
        fn(index, field.first, field.second);
    }

    return true;
}

inline std::uint64_t const delimited_data::size() const
{
    return record_count_;
}

inline void delimited_data::store_field(unsigned index, string_view &value, type_mask_t type)
{
    assert(index < column_info_.size());

    if (index == 0)
        ++record_count_;

    if (type != null_type)
    {
        // if the column type doesn't match the new type, then
        // the column type will be a string
        if (column_info_[index].second == 0)
            column_info_[index].second = type;
        else if (column_info_[index].second != type)
            column_info_[index].second = string_type;
    }

    column_values_[index].push_back(std::make_pair(value, type));
    assert(column_info_.size() == column_values_.size());
}






class mapped_csv : public delimited_data
{
  public:
    explicit mapped_csv(char const * const filename)
      : file_(filename, readonly),
        mmf_(file_, readonly)
    {
    }
    ~mapped_csv();
    void close();

    file<char>                 file_;
    memory_mapped_file<char>   mmf_;

    bool          const read(std::uint64_t max_records=0);
};

inline mapped_csv::~mapped_csv()
{
    close();
}

inline void mapped_csv::close()
{
    file_.close();
    mmf_.release();
}

inline bool const mapped_csv::read(std::uint64_t max_records)
{
    if (!mmf_.is_mapped())
        return false;

    char const *it = mmf_.get();
    char const *ite = it + file_.size();
    attach(it, ite, max_records);

    return true;
}

}   // namespace data_processing
}   // namespace cdmh
