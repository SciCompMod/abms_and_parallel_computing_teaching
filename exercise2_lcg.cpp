#include <cstddef>
#include <iostream>
#include <string>
#include <vector>

typedef struct {
    unsigned int a, c, m, state;
} LCG;

void get_sample_print(LCG& lcg, int n)
{
    for (int i = 0; i < n; ++i) {
        lcg.state = (lcg.a * lcg.state + lcg.c) % lcg.m;
        std::cout << (lcg.state) << " "; // potentially scale to [0,1] by dividing by m
    }
}

void get_sample_bin_print(LCG& lcg, int n, int shift = 0)
{
    for (int i = 0; i < shift; ++i) {
        lcg.state = (lcg.a * lcg.state + lcg.c) % lcg.m;
    }
    double min = 1.0, max = 0.0;
    std::vector<int> bins(10, 0);
    for (int i = 0; i < n; ++i) {
        lcg.state = (lcg.a * lcg.state + lcg.c) % lcg.m;
        bins[(size_t)((lcg.state) / (double)lcg.m * 10)]++;
        // std::cout << ((lcg.state) / (double)lcg.m * 100) << " ";
    }
    for (size_t i = 0; i < bins.size(); ++i) {
        double dummy = bins[i] / 1e4;
        if (dummy > max)
            max = dummy;
        if (dummy < min)
            min = dummy;
        std::cout << i << " " << bins[i] / 1e4 << std::endl;
    }
    std::cout << "min: " << min << ", max: " << max << std::endl;
}

void parse_args(int argc, char* argv[], unsigned int& a, unsigned int& c, unsigned int& m, unsigned int& seed)
{
    for (int i = 1; i < argc - 1; ++i) {
        std::string arg(argv[i]);
        if (arg == "--a") {
            a = static_cast<unsigned int>(std::stoi(argv[++i]));
        }
        else if (arg == "--c") {
            c = static_cast<unsigned int>(std::stoi(argv[++i]));
        }
        else if (arg == "--m") {
            m = static_cast<unsigned int>(std::stoi(argv[++i]));
        }
        else if (arg == "--s") {
            seed = static_cast<unsigned int>(std::stoi(argv[++i]));
        }
        else {
            std::cerr << "Unknown or malformed argument. Using standard parameters." << std::endl;
            std::cerr << "Usage: " << std::string(argv[0]) << " [--a A] [--c C] [--m M] [--s S]" << std::endl;
        }
    }
}

int main(int argc, char* argv[])
{
    //     unsigned int seed = 0;
    //     unsigned int a    = 1;
    //     unsigned int c    = 2;
    //     unsigned int m    = 3;

    // unsigned int seed = 1;
    // unsigned int a    = 2;
    // unsigned int c    = 0;
    // unsigned int m    = 9;

    unsigned int seed = 0;
    unsigned int a    = 1103515245;
    unsigned int c    = 12345;
    unsigned int m    = 1U << 31; // 2^31 (glibc (used by GCC), see Wikipedia)

    parse_args(argc, argv, a, c, m, seed);
    std::cout << "Parameters: a=" << a << ", c=" << c << ", m=" << m << ", seed: " << seed << std::endl;

    LCG lcg = {a, c, m, static_cast<unsigned int>(seed)};
    // get_sample_print(lcg, 10);

    get_sample_bin_print(lcg, 1e6, 3e5);
}
