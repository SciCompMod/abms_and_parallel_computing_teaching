// build: g++ -O2 -fopenmp exercise3_ompintro_race.cpp -o ex3_omp_race
// run:   OMP_NUM_THREADS=4 ./ex3_omp

#include <iostream>
#include <vector>
#include <omp.h>

// Helper to print a vector
void print_vec(const char* name, const std::vector<int>& v)
{
    std::cout << name << ": [";
    for (size_t i = 0; i < v.size(); ++i) {
        std::cout << v[i] << (i + 1 == v.size() ? "" : ", ");
    }
    std::cout << "]\n";
}

int main()
{
    const int N = 16;

    // Input: all ones; the correct prefix-sum is y[i] = i+1
    std::vector<int> x(N, 1);
    std::vector<int> y_seq(N, 0), y_bad(N, 0);

    // --- Correct sequential baseline (no OpenMP) ---
    y_seq[0] = x[0];
    for (int i = 1; i < N; ++i)
        y_seq[i] = y_seq[i - 1] + x[i];

    // --- WRONG: naive parallelization of a loop-carried dependence ---
    // Loop-carried dependence on y[i-1] -> data race and wrong results.
    y_bad[0] = x[0];
#pragma omp parallel for
    for (int i = 1; i < N; ++i) {
        // UNSAFE: y[i] depends on y[i-1] which may be concurrently updated.
        y_bad[i] = y_bad[i - 1] + x[i];
    }

    // Show results
    print_vec("seq", y_seq);
    print_vec("bad", y_bad);

    // Quick correctness checks
    bool bad_matches = (y_bad == y_seq);

    return 0;
}
