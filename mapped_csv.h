#include "mapped_csv.detail.h"

namespace cdmh {

namespace data_processing {

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
