// Copyright (c) 2002-2013 Craig Henderson
// Part of the Data Processing Library
// https://github.com/cdmh/dataproc

#if !defined(MMAP_WINDOWS)
#error Win32 header file has been included without MMAP_WINDOWS defined
#endif

namespace cdmh {

template<typename T>
bool file<T>::close(void) throw()
{
    if (handle_ == MEMMAP_INVALID_HANDLE)
        return true;

    bool ret = (::CloseHandle(handle_) != 0);
    if (ret)
    {
        handle_ = MEMMAP_INVALID_HANDLE;
        filepath_.clear();

        // CloseHandle() doesn't reset the last error,
        // so set our errorcode to zero
        err_ = 0;
    }
    else
        err_ = ::GetLastError();

    return ret;
}



template<typename T>
bool file<T>::create(const std::basic_string<T> &filepath) THROWS_ALREADY_ATTACHED
{
    if (handle_ != MEMMAP_INVALID_HANDLE)
        throw file_already_attached();

    handle_ = ::CreateFile(filepath.c_str(),
                           GENERIC_ALL, FILE_SHARE_READ, NULL, CREATE_ALWAYS,
                           0, NULL);
    err_ = ::GetLastError();
    if (this->is_open())
        filepath_ = filepath;
    return this->is_open();
}


template<typename T>
bool file<T>::open_readonly(const std::basic_string<T> &filepath) THROWS_ALREADY_ATTACHED
{
    if (handle_ != MEMMAP_INVALID_HANDLE)
        throw file_already_attached();

    handle_ = ::CreateFileA(filepath.c_str(),
                            GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,
                            0, NULL);
    err_ = ::GetLastError();
    if (this->is_open())
        filepath_ = filepath;
    return this->is_open();
}



template<typename T>
bool file<T>::open_readwrite(const std::basic_string<T> &filepath) THROWS_ALREADY_ATTACHED
{
    if (handle_ != MEMMAP_INVALID_HANDLE)
        throw file_already_attached();

    handle_ = ::CreateFileA(filepath.c_str(),
                            GENERIC_ALL, FILE_SHARE_READ, NULL, OPEN_EXISTING,
                            0, NULL);
    err_ = ::GetLastError();
    if (this->is_open())
        filepath_ = filepath;
    return this->is_open();
}


}   // namespace cdmh
