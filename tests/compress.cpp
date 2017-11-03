#include <b0/utils/compress.hpp>
#include <sstream>
#include <iostream>
#include <iomanip>

std::string generatePayload(size_t size)
{
    std::stringstream ss;
    std::string in = "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
            "aaaabababababbbbababbbbbbbacccccccccccccccccccccccccccccc";
    for(size_t i = 0; i < size; i++)
        ss << in[i % in.size()];
    return ss.str();
}

int main(int argc, char **argv)
{
    size_t size[] = {100, 500, 2000, 8000, 25000, 100000, 1000000, 5000000};
    for(int j = 0; j < sizeof(size)/sizeof(size[0]); j++)
    {
        std::cout << "Testing with a payload size of " << size[j] << std::endl;
        for(int level = -1; level <= 9; level++)
        {
            std::cout << "Testing with compression level of " << level << std::endl;
            std::string in = generatePayload(size[j]);
            std::cout << "in size: " << in.size() << std::endl;
            std::string out = compress(in);
            std::cout << std::endl;
            std::cout << "out size: " << out.size() << std::endl;
            if(!(out.size() < in.size())) exit(1);
            std::string in2 = decompress(out);
            std::cout << "in2 size: " << in2.size() << std::endl;
            if(in != in2) exit(2);
        }
    }
    exit(0);
}

