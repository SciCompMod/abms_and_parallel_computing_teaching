#include <iostream>
#include <ostream>
#include <vector>

typedef struct {
    unsigned int a, c, m, state;
} LCG;

void draw_samples_and_print_statistics(LCG& lcg, int n)
{
    std::vector<int> headstails(100, 0);
    for (int i = 0; i < n; ++i) {
        lcg.state = (lcg.a * lcg.state + lcg.c) % lcg.m;
        if ((lcg.state) / (double)lcg.m < 0.33333333333333333) // We assume the PRNG is uniform between 0 and 1
        {
            headstails[0]++; // Heads
            // std::cout << "Heads" << std::endl;
        }
        else {
            headstails[1]++; // Tails
            // std::cout << "Tails" << std::endl;
        }
    }
    std::cout << "Heads: " << headstails[0] / (double)n << ", Tails: " << headstails[1] / (double)n << std::endl;
}

int main(int argc, char* argv[])
{

    // LCG for probabilities for the coin to appear fair
    unsigned int seed = 0;
    unsigned int a    = 5;
    unsigned int c    = 1;
    unsigned int m    = 24;

    // LCG for inverted empirical probabilities for the coin to appear biased towards heads
    // unsigned int seed = 1;
    // unsigned int a    = 5;
    // unsigned int c    = 7;
    // unsigned int m    = 21;

    std::cout << "Parameters: a=" << a << ", c=" << c << ", m=" << m << ", seed: " << seed << std::endl;

    LCG lcg = {a, c, m, static_cast<unsigned int>(seed)};
    // get_sample_print(lcg, 10);

    draw_samples_and_print_statistics(lcg, 1e3);
}
