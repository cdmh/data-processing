// Copyright (c) 2002-2013 Craig Henderson
// Part of the Data Processing Library
// https://github.com/cdmh/dataproc

#if !defined(MMAP_POSIX)
#error POSIX header file has been included without MMAP_POSIX defined
#endif

#include <sys/fcntl.h>

namespace cdmh {

template<typename T>
bool file<T>::close() throw()
{
    if (handle_ == MEMMAP_INVALID_HANDLE)
        return true;

    bool ret = (::close(handle_) != 0);
    if (ret)
    {
        err_    = 0;
        handle_ = MEMMAP_INVALID_HANDLE;
        filepath_.clear();
    }
    else
        err_ = errno;

    return ret;
}



template<typename T>
bool file<T>::create(const std::basic_string<T> &filepath) throw(file_already_attached)
{
    if (handle_ != MEMMAP_INVALID_HANDLE)
        throw file_already_attached();

    handle_ = ::open(filepath.c_str(), O_CREAT | O_RDWR | O_BINARY);
    err_ = errno;
    if (this->is_open())
        filepath_ = filepath;
    return this->is_open();
}


template<typename T>
bool file<T>::open_readonly(const std::basic_string<T> &filepath) throw(file_already_attached)
{
    if (handle_ != MEMMAP_INVALID_HANDLE)
        throw file_already_attached();

    handle_ = ::open(filepath.c_str(), O_RDONLY);
    err_ = errno;
    if (this->is_open())
        filepath_ = filepath;
    return this->is_open();
}



template<typename T>
bool file<T>::open_readwrite(const std::basic_string<T> &filepath) throw(file_already_attached)
{
    if (handle_ != MEMMAP_INVALID_HANDLE)
        throw file_already_attached();

    handle_ = ::open(filepath.c_str(), O_RDWR);
    err_ = errno;
    if (this->is_open())
        filepath_ = filepath;
    return this->is_open();
}


}   // namespace cdmh
