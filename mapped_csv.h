#include "mapped_csv.detail.h"

namespace cdmh {

namespace data_processing {

class mapped_csv
{
  public:
    mapped_csv(char const * const filename)
      : file_(filename, readonly),
        mmf_(file_, readonly)
    {
    }

    dataset             create_dataset(bool destructive = true);
    void                read(std::uint64_t max_records=0);
    std::uint64_t const size() const;


  protected:
    void create_column(unsigned index, std::pair<char const *, char const *> &name, std::uint32_t /*type*/);
    void store_field(unsigned index, std::pair<char const *, char const *> &value, std::uint32_t type);

    template<typename It, typename Fn>
    bool const process_record(It &begin, It end, Fn fn);

  private:
    typedef std::pair<char const *, char const *> string_t;
    typedef std::vector<string_t>                 string_list_t;
    typedef std::pair<string_t, std::uint32_t>    column_info_t;

    file<char>                 file_;
    memory_mapped_file<char>   mmf_;
    std::uint64_t              record_count_;
    std::vector<column_info_t> column_info_;
    std::vector<std::uint32_t> incl_type_mask_;
    std::vector<string_list_t> column_values_;
};

inline void mapped_csv::create_column(unsigned index, std::pair<char const *, char const *> &name, std::uint32_t /*type*/)
{
#ifdef NDEBUG
    index;
#else
    assert(index == column_info_.size());
#endif
    column_info_.push_back(column_info_t(name,0));
    column_values_.push_back(string_list_t());
}

inline dataset mapped_csv::create_dataset(bool destructive)
{
    dataset ds(column_info_.size());
    for (unsigned index=0; index<column_info_.size(); ++index)
    {
        auto inserter = ds.create_column(column_info_[index].second);
        for (auto value : column_values_[index])
            inserter(value);

        if (destructive)
            string_list_t().swap(column_values_[index]);
    }

    if (destructive)
    {
        record_count_ = 0;
        std::vector<column_info_t>().swap(column_info_);
        std::vector<std::uint32_t>().swap(incl_type_mask_);
        std::vector<string_list_t>().swap(column_values_);
        file_.close();
        mmf_.release();
    }

    return ds;
}

template<typename It, typename Fn>
inline
bool const mapped_csv::process_record(It &begin, It end, Fn fn)
{
    for (unsigned index=0; begin!=end; ++index)
    {
        auto field = detail::read_field(begin, end);
        fn(index, field.first, field.second);
    }

    return true;
}

inline void mapped_csv::read(std::uint64_t max_records)
{
    typedef 
    std::function<void (unsigned, std::pair<char const *, char const *> &, std::uint32_t)>
    store_fn_t;

    using std::placeholders::_1;
    using std::placeholders::_2;
    using std::placeholders::_3;
    store_fn_t store         = std::bind(&mapped_csv::create_column, this, _1, _2, _3);
    store_fn_t store_fields  = std::bind(&mapped_csv::store_field, this, _1, _2, _3);

    char const *it = mmf_.get();
    char const *ite = it + file_.size();

    record_count_ = 0;
    while (it != ite  &&  (max_records == 0  ||  size() < max_records))
    {
        auto eol = std::find(it,ite,'\r');
        process_record(it, eol, store);
        assert(it == eol);
        detail::ltrim(it, ite);
        store = store_fields;
    }
}

inline std::uint64_t const mapped_csv::size() const
{
    return record_count_;
}

inline void mapped_csv::store_field(unsigned index, std::pair<char const *, char const *> &value, std::uint32_t type)
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

    column_values_[index].push_back(value);
    assert(column_info_.size() == column_values_.size());
}

}   // namespace data_processing
}   // namespace cdmh
