// Copyright (c) 2002-2013 Craig Henderson
// Part of the Data Processing Library
// https://github.com/cdmh/dataproc

#if !defined(MMAP_WINDOWS)
#error Win32 header file has been included without MMAP_WINDOWS defined
#endif

namespace cdmh {


template <typename T, typename F>
bool memory_mapped_file<T, F>::release(void)
{
    if (detail_.file_mapping_handle_ != INVALID_HANDLE_VALUE
    &&  detail_.file_mapping_handle_ != NULL)
    {
        if (::UnmapViewOfFile(ptr_))
        {
            ::CloseHandle(detail_.file_mapping_handle_);
            detail_.file_mapping_handle_ = NULL;
            ptr_ = 0;
        }
    }

    return (ptr_ == 0);
}


template <typename T, typename F>
bool memory_mapped_file<T, F>::map_readonly(file_handle_t handle)
{
    flags_or_security_t fos  = { FILE_MAP_READ, NULL };
    protection_t        prot = PAGE_READONLY;
    length_t            len  = { 0 };
    offset_t            off  = { 0 };
    return this->map(handle, prot, fos, len, off);
}


template <typename T, typename F>
bool memory_mapped_file<T, F>::map_readwrite(file_handle_t handle)
{
    flags_or_security_t fos  = { FILE_MAP_WRITE, NULL };
    protection_t        prot = PAGE_READWRITE;
    length_t            len  = { 0 };
    offset_t            off  = { 0 };
    return this->map(handle, prot, fos, len, off);
}



template <typename T, typename F>
bool memory_mapped_file<T, F>::map(file_handle_t    &handle,
                                protection_t        &prot,
                                flags_or_security_t &fos,
                                length_t            &len,
                                offset_t            &off)
{
    if (ptr_ != 0)
        return false;

    detail_.file_mapping_handle_ = ::CreateFileMapping(handle,
                                                       fos.security,
                                                       prot,
#ifdef _USE_INT64
                                                       len.map.hi, len.map.lo,
#else
                                                       0, len.map,
#endif
                                                       NULL);
    if (detail_.file_mapping_handle_ == NULL)
    {
        err_ = ::GetLastError();
        return false;
    }

    void *ptr;
    ptr = ::MapViewOfFileEx(detail_.file_mapping_handle_,   // handle to file-mapping object
                            fos.access,                     // access mode
#ifdef _USE_INT64
                            off.hi,                         // high-order DWORD of offset_t
                            off.lo,                         // low-order DWORD of offset_t
#else
                            0, off,
#endif
                            len.view,                       // number of bytes to map
                            NULL);                          // starting address
    if (ptr == 0)
    {
        err_ = ::GetLastError();
        return false;
    }

    ptr_ = reinterpret_cast<T *>(ptr);
    return (ptr_ != 0);
}

}   // namespace cdmh
