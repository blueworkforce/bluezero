#include <b0/utils/compress.hpp>
#include <b0/config.h>
#include <boost/format.hpp>
#include <stdexcept>
#include <cstring>

#ifdef ZLIB_FOUND

#include <zlib.h>

static std::string zlib_wrapper(const std::string &str, bool compress, int level = 0)
{
    if(level == -1) level = Z_BEST_COMPRESSION;
    const char *method = compress ? "deflate" : "inflate";
    int ret;
    z_stream zs;
    memset(&zs, 0, sizeof(zs));
    if(compress) ret = deflateInit(&zs, level); else ret = inflateInit(&zs);
    if(ret != Z_OK)
        throw std::runtime_error((boost::format("%sInit failed") % method).str());;
    zs.next_in = (Bytef*)(str.data());
    zs.avail_in = str.size();
    char *outbuf = new char[2 << 24];
    std::string outstr;
    do
    {
        zs.next_out = reinterpret_cast<Bytef*>(outbuf);
        zs.avail_out = sizeof(outbuf);
        ret = compress ? deflate(&zs, Z_FINISH) : inflate(&zs, 0);
        if(outstr.size() < zs.total_out)
            outstr.append(outbuf, zs.total_out - outstr.size());
    }
    while(ret == Z_OK);
    if(compress) deflateEnd(&zs); else inflateEnd(&zs);
    delete[] outbuf;
    if(ret != Z_STREAM_END)
        throw std::runtime_error((boost::format("zlib %s error %d: %s") % method % ret % zs.msg).str());
    return outstr;
}

#else

static std::string zlib_wrapper(const std::string &str, bool compress, int level = 0)
{
    return std::string(str);
}

#endif

std::string compress(const std::string &str, int level)
{
    return zlib_wrapper(str, true, level);
}

std::string decompress(const std::string &str)
{
    return zlib_wrapper(str, false);
}

