#include <b0/utils/compress.hpp>
#include <iostream>
#include <iomanip>

int main(int argc, char **argv)
{
    for(int level = -1; level <= 9; level++)
    {
        std::string in = "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
            "aaaabababababbbbababbbbbbbacccccccccccccccccccccccccccccc";
        std::cout << "in: " << in << std::endl;
        std::cout << "in size: " << in.size() << std::endl;
        std::string out = compress(in);
        std::cout << "out:";
        for(int i = 0; i < out.size(); i++)
            std::cout << " " << std::setfill('0') << std::setw(2) << (int)out[i];
        std::cout << std::endl;
        std::cout << "out size: " << out.size() << std::endl;
        if(!(out.size() < in.size())) exit(1);
        std::string in2 = decompress(out);
        std::cout << "in2: " << in2 << std::endl;
        std::cout << "in2 size: " << in2.size() << std::endl;
        if(in != in2) exit(2);
    }
    exit(0);
}

