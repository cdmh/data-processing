// Copyright (c) 2002-2013 Craig Henderson
// Part of the Data Processing Library
// https://github.com/cdmh/dataproc
//
// Thanks to:
//  Malte Starostik for POSIX 64 bit support changes
//  Scott Kirkwood for portability fixes
//
//
// Synopsis:
// --------
//
//  filesize_t get_file_size(file_handle_t &handle);
//  template<typename T> class mapped_memory
//  template<typename T> class file

#pragma once

#ifdef _MSC_VER
#   define MMAP_WINDOWS

#   if _MSC_VER <= 1800
#       define noexcept
#   endif

#   ifndef _WINDOWS_
#       error Please include windows.h first
#   endif

#   ifdef _WIN64
#       define MMAP_USE_INT64
#   endif
#endif

#if !defined(MMAP_POSIX)  &&  !defined(MMAP_WINDOWS)
#   error No memory map configuration defined: MMAP_POSIX or MMAP_WINDOWS
#endif

#include <sys/stat.h>
#include <exception>
#include <string>
#include <errno.h>
#include "boost/config.hpp"
#include "boost/utility.hpp"

namespace cdmh {

#if defined(MMAP_WINDOWS)
    typedef HANDLE file_handle_t;
    typedef DWORD  protection_t;
    typedef struct flags_or_security_
    {
        DWORD                 access;
        LPSECURITY_ATTRIBUTES security;
    }   flags_or_security_t;
#ifdef MMAP_USE_INT64
    typedef std::uint64_t filesize_t;
    typedef struct
    {
        std::uint32_t hi, lo;
    } offset_t;
#else
    typedef DWORD offset_t;
    typedef DWORD filesize_t;
#endif // MMAP_USE_INT64

    typedef struct length_
    {
        filesize_t  map;
        SIZE_T      view;
    } length_t;

    typedef struct detail_struct_
    {
        file_handle_t file_mapping_handle_;
    } detail_struct;
    typedef DWORD err_t;

    #define MEMMAP_INVALID_HANDLE INVALID_HANDLE_VALUE
#elif defined(MMAP_POSIX)
    typedef int   file_handle_t;
    typedef int   protection_t;
    typedef int   flags_or_security_t;
    typedef off_t length_t;
    typedef off_t filesize_t;
    typedef off_t offset_t;

    #define MEMMAP_INVALID_HANDLE -1

    typedef struct detail_struct_
    {
        int len;
    } detail_struct;
    typedef int err_t;
#endif



// returns the size of an open file
inline filesize_t get_file_size(file_handle_t handle)
{
#if defined(MMAP_POSIX)
    struct stat info;
    if (::fstat(handle, &info) == -1)
        return 0;
    return info.st_size;
#elif defined(MMAP_WINDOWS)
    union
    {
        filesize_t len;
        struct
        {
            DWORD lo;
            DWORD hi;
        } i64;
    } u;
    memset(&u, 0, sizeof(u));
    u.i64.lo = ::GetFileSize(handle, &u.i64.hi);
    assert(sizeof(u.i64) == sizeof(u.len)  ||  u.i64.hi == 0);
    return u.len;
#endif
}


typedef enum file_access_ { readonly, readwrite } file_access;


class file_already_attached : public std::exception
{
    public:
    virtual char const * what() const noexcept
    {
        return "boost::file_already_attached";
    }
};


template<typename T=char>
class file
{
    private:
    err_t                err_;
    file_handle_t        handle_;
    std::basic_string<T> filepath_;

    public:
    file() noexcept;
    file(std::basic_string<T> const &filepath, file_access access) noexcept;
    ~file() noexcept;

    bool close() noexcept;
    bool create(const std::basic_string<T> &filepath);

    err_t                error()    const noexcept { return err_;                             }
    std::basic_string<T> filepath() const noexcept { return filepath_;                        }
    file_handle_t        handle()   const noexcept { return handle_;                          }
    filesize_t           size()     const noexcept { return get_file_size(handle_);           }
    bool                 is_open()  const noexcept { return handle_ != MEMMAP_INVALID_HANDLE; }

    bool open_readonly(const std::basic_string<T> &filepath);
    bool open_readwrite(const std::basic_string<T> &filepath);
};

template<typename T>
inline file<T>::file() noexcept
    : err_(0),
    handle_(MEMMAP_INVALID_HANDLE)
{
}

template<typename T>
inline file<T>::file(const std::basic_string<T> &filepath, file_access access) noexcept
    : err_(0),
    handle_(MEMMAP_INVALID_HANDLE)
{
    if (access == readonly)
        this->open_readonly(filepath);
    else if (access == readwrite)
        this->open_readwrite(filepath);
}

template<typename T>
inline file<T>::~file() noexcept
{
    this->close();
}

    


// memory mapped file management class
template <typename T, typename F=file<char> >
class mapped_memory
{
    public:
    mapped_memory();
    mapped_memory(F &file, file_access access);
    mapped_memory(file_handle_t &handle, file_access access);
    ~mapped_memory();

    bool map_readonly(file_handle_t handle);
    bool map_readwrite(file_handle_t handle);

    // this is exposed publicly for completeness, but is
    // unlikely to be used by the library user
    bool map(file_handle_t       &handle,
             protection_t        &prot,
             flags_or_security_t &fos,
             length_t            &len,
             offset_t            &off);

    // release the mapping
    bool     release();

    // is the object mapped to a file?
    bool     is_mapped() const { return (ptr_ != 0); }
    err_t    error()     const { return err_;        }

    // data accessibility
    T       *get()             { return ptr_;        }
    const T *get()       const { return ptr_;        }

    private:
    T             *ptr_;
    err_t          err_;
    detail_struct  detail_;
};


// cross platform default ctor
template <typename T, typename F>
inline mapped_memory<T, F>::mapped_memory()
    : ptr_(0),
    err_(0)
{
    memset(&detail_, 0, sizeof(detail_));
}

template <typename T, typename F>
inline mapped_memory<T, F>::mapped_memory(file_handle_t &handle,
                                                file_access    access)
    : ptr_(0),
    err_(0)
{
    memset(&detail_, 0, sizeof(detail_));
    if (access == readonly)
        this->map_readonly(handle);
    else if (access == readwrite)
        this->map_readwrite(handle);
}

template <typename T, typename F>
inline mapped_memory<T, F>::mapped_memory(F &file, file_access access)
    : ptr_(0),
    err_(0)
{
    memset(&detail_, 0, sizeof(detail_));
    if (access == readonly)
        this->map_readonly(file.handle());
    else if (access == readwrite)
        this->map_readwrite(file.handle());
}

template <typename T, typename F>
inline mapped_memory<T, F>::~mapped_memory()
{
    this->release();
}

template<typename T>
class memory_mapped_file
{
  public:
    explicit memory_mapped_file(char const * const filename);
    explicit memory_mapped_file(std::string const &filename);
    ~memory_mapped_file();

    void             close();
    bool const       is_open() const          { return mm_.is_mapped(); }
    T               *get()                    { return mm_.get();       }
    T const         *get()     const          { return mm_.get();       }
    filesize_t const size()    const noexcept { return file_.size();    }

  private:
    file<char>       file_;
    mapped_memory<T> mm_;
};

template<typename T>
inline memory_mapped_file<T>::memory_mapped_file(char const * const filename)
  : file_(filename, readonly),
    mm_(file_, readonly)
{
}

template<typename T>
inline memory_mapped_file<T>::memory_mapped_file(std::string const &filename)
  : memory_mapped_file(filename.c_str())
{
}

template<typename T>
inline memory_mapped_file<T>::~memory_mapped_file()
{
    close();
}

template<typename T>
inline void memory_mapped_file<T>::close()
{
    mm_.release();
    file_.close();
}

}   // namespace cdmh


/////////////////////////////////////////////////////////////////////
//
// platform specific implementations are in separate headers
//
#if defined(MMAP_WINDOWS)
#   include "memmap/mmf_win32.h"   // platform specific code for Win32
#   include "memmap/file_win32.h"  // platform specific code for Win32
#elif defined(MMAP_POSIX)
#   include "memmap/mmf_posix.h"   // platform specific code for POSIX
#   include "memmap/file_posix.h"  // platform specific code for POSIX
#endif
