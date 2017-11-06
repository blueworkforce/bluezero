#include <stdexcept>
#include <cstring>

#include <boost/format.hpp>

#include <b0/config.h>
#include <b0/compress/lz4.h>

#ifdef LZ4_FOUND

#include <lz4.h>

namespace b0
{

namespace compress
{

std::string lz4_compress(const std::string &str, int level)
{
    int sourceSize = str.size();
    int maxDestSize = LZ4_compressBound(sourceSize);
    const char *source = str.data();
    char *dest = new char[maxDestSize];
    int bytesWritten = LZ4_compress_default(source, dest, sourceSize, maxDestSize);
    if(!bytesWritten)
    {
        delete[] dest;
        throw std::runtime_error("lz4 compress failed");
    }
    std::string ret(dest, bytesWritten);
    delete[] dest;
    return ret;
}

std::string lz4_decompress(const std::string &str, size_t size)
{
    int compressedSize = str.size();
    int maxDecompressedSize = size ? size : compressedSize * 10;
    const char *source = str.data();
    char *dest = new char[size];
    int bytesWritten = LZ4_decompress_safe(source, dest, compressedSize, maxDecompressedSize);
    if(bytesWritten < 0)
    {
        delete[] dest;
        throw std::runtime_error("lz4 decompress failed");
    }
    std::string ret(dest, bytesWritten);
    delete[] dest;
    return ret;
}

} // namespace compress

} // namespace b0

#endif // LZ4_FOUND

