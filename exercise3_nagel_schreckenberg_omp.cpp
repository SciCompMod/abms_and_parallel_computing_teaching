// g++ -std=c++11 -fopenmp exercise3_nagel_schreckenberg_omp.cpp -o ex3_nasch_omp
// OMP_NUM_THREADS=4 ./ex3_nasch_omp
#include <cstddef>
#include <iostream>
#include <vector>
#include <chrono>
#include <fstream>
#include <numeric>
#include <thread>
#include <random>
#include <omp.h>
#include <mutex>

const char FREE = '.';

// Type alias for readability
using Grid = std::vector<int>;

void update_velocities_deterministic(Grid& grid, size_t vmax, double prob)
{
#pragma omp parallel for schedule(static, 1)
    for (int i = 0; i < grid.size(); ++i) {
        if (grid[i] >= 0) {
            // Rule 1: Acceleration
            if (grid[i] < static_cast<int>(vmax)) {
                grid[i]++;
            }
            // Rule 2: Slowing down due to other cars
            size_t j = 1;
            size_t k = (i + 1) % grid.size();
            while (j <= vmax) {
                if (grid[k] >= 0) {
                    grid[i] = std::min(grid[i], static_cast<int>(j - 1));
                    break;
                }
                j++;
                k = (k + 1) % grid.size();
            }
            // Rule 3: Randomization
            // if (grid[i] > 0 && (rand() % 100) < 100 * prob) { // 'prob' chance to slow down
            //     grid[i]--;
            // }
        }
    }
}

void update_velocities_random_lock(Grid& grid, size_t vmax, double prob, std::uniform_real_distribution<double>& dist,
                                   std::mt19937& gen)
{
    std::mutex rng_lock;
#pragma omp parallel for schedule(static, 1)
    for (int i = 0; i < grid.size(); ++i) {
        if (grid[i] >= 0) {
            // Rule 1: Acceleration
            if (grid[i] < static_cast<int>(vmax)) {
                grid[i]++;
            }
            // Rule 2: Slowing down due to other cars
            size_t j = 1;
            size_t k = (i + 1) % grid.size();
            while (j <= vmax) {
                if (grid[k] >= 0) {
                    grid[i] = std::min(grid[i], static_cast<int>(j - 1));
                    break;
                }
                j++;
                k = (k + 1) % grid.size();
            }
            // Rule 3 : Randomization
            rng_lock.lock();
            double random_value = dist(gen);
            rng_lock.unlock();
            if (grid[i] > 0 && random_value < prob) { // 'prob' chance to slow down
                grid[i]--;
            }
        }
    }
}

void update_velocities_random(Grid& grid, size_t vmax, double prob, std::uniform_real_distribution<double>& dist,
                              std::mt19937& gen)
{
#pragma omp parallel for schedule(static, 1)
    for (int i = 0; i < grid.size(); ++i) {
        if (grid[i] >= 0) {
            // Rule 1: Acceleration
            if (grid[i] < static_cast<int>(vmax)) {
                grid[i]++;
            }
            // Rule 2: Slowing down due to other cars
            size_t j = 1;
            size_t k = (i + 1) % grid.size();
            while (j <= vmax) {
                if (grid[k] >= 0) {
                    grid[i] = std::min(grid[i], static_cast<int>(j - 1));
                    break;
                }
                j++;
                k = (k + 1) % grid.size();
            }
            // Rule 3 : Randomization
            double random_value = dist(gen);
            //             int tid             = omp_get_thread_num();
            //             int nthreads        = omp_get_num_threads();
            // #pragma omp critical
            //             {
            //                 std::cout << "OMP thread " << tid << ". Random value: " << random_value << "\n";
            //             }
            if (grid[i] > 0 && random_value < prob) { // 'prob' chance to slow down
                grid[i]--;
            }
        }
    }
}

// Compute the next step
Grid advance(const Grid& grid)
{
    Grid new_grid(grid.size(), -1); // Initialize new grid with -1 (free positions)
#pragma omp parallel for schedule(static, 1)
    for (int i = 0; i < grid.size(); ++i) {
        if (grid[i] >= 0) { // If there's a car at position i
            int new_pos       = (i + grid[i]) % grid.size();
            new_grid[new_pos] = grid[i]; // Move the car to its new position
        }
    }
    return new_grid; // Update the grid to the new state
}

// Initialize a grid with random state
Grid random_grid(size_t length, size_t ncars, size_t vmax)
{
    Grid grid          = Grid(length, -1);
    size_t cars_placed = 0;
    while (cars_placed < ncars) {
        int pos = rand() % length; // Random position between 0 and length-1
        // std::cout << "Cars placed " << cars_placed << ", Trying to place car at position: " << pos << std::endl;
        if (grid[pos] == -1) { // If the position is free
            grid[pos] = rand() % vmax + 1; // Random velocity between 1 and vmax
            cars_placed++;
        }
    }
    return grid;
}

void parse_args(int argc, char* argv[], int& length, int& ncars, int& vmax, int& steps, double& prob, int& seed)
{
    for (int i = 1; i < argc - 1; ++i) {
        std::string arg = argv[i];
        if (arg == "--length") {
            length = std::stoi(argv[++i]);
        }
        else if (arg == "--ncars") {
            ncars = std::stoi(argv[++i]);
        }
        else if (arg == "--vmax") {
            vmax = std::stoi(argv[++i]);
        }
        else if (arg == "--steps") {
            steps = std::stoi(argv[++i]);
        }
        else if (arg == "--prob") {
            prob = std::stod(argv[++i]);
        }
        else if (arg == "--seed") {
            seed = std::stoi(argv[++i]);
        }
        else {
            std::cerr << "Unknown or malformed argument. Using standard parameters." << std::endl;
            std::cerr << "Usage: " << argv[0]
                      << " [--length L] [--ncars N] [--vmax V] [--steps T] [--prob P] [--seed S]\n";
        }
    }
}

int main(int argc, char* argv[])
{

    int length  = 1e8;
    int ncars   = 1e7; // Number of cars
    int vmax    = 5;
    int steps   = 5;
    double prob = 0.3; // Probability of slowing down
    int seed    = 0; // Random seed

    parse_args(argc, argv, length, ncars, vmax, steps, prob, seed);
    std::cout << "Parameters: L=" << length << ", ncars=" << ncars << ", vmax=" << vmax << ", time steps: " << steps
              << ", probability: " << prob << ", seed: " << seed << std::endl;

    // sequential random grid creation (not thread-safe)
    srand(seed);
    Grid grid = random_grid(length, ncars, vmax);

    // deterministic update
    // auto start = std::chrono::steady_clock::now();
    // for (int i = 0; i < steps; ++i) {
    //     update_velocities_deterministic(grid, vmax, prob);
    //     grid = advance(grid);
    // }
    // auto end      = std::chrono::steady_clock::now();
    // auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    // std::cout << "Elapsed time in iterations: " << duration.count() << " ms\n";

    // https://www.learncpp.com/cpp-tutorial/generating-random-numbers-using-mersenne-twister/
    std::random_device rd{};
    // std::seed_seq seed_s{rd(), rd(), rd(), rd(), rd(),
    //                      rd(), rd(), rd()}; // get 8 integers of random numbers from std::random_device for our seed

    // set seeds, e.g., for debugging
    std::vector<unsigned int> seed_data = {114381446,  2427727386, 806223567,  832414962,
                                           4121923627, 1581162203, 2468991901, 3237492387};
    std::seed_seq seed_s{seed_data.begin(), seed_data.end()};
    // std::mt19937 generator{seed_s}; // initialize our Mersenne Twister with the std::seed_seq
    thread_local std::mt19937 generator{seed_s}; // initialize our Mersenne Twister with the std::seed_seq
    // std::vector<std::mt19937> generators_vec = {std::mt19937{seed_s}, std::mt19937{seed_s}};

    // Alternative comments on underseeding:
    // https://codereview.stackexchange.com/questions/109260/seed-stdmt19937-from-stdrandom-device
    //    std::mt19937 rng(std::random_device{}());
    // However, this only provides a single unsigned int, i.e. 32 bits on most systems, of seed randomness,
    // which seems quite tiny when compared to the 19937 bit state space we want to seed.
    // Indeed, if I find out the first number generated, my PC (Intel i7-4790K)
    // only needs about 10 minutes to search through all 32 bit numbers and find the used seed.
    // Some approach:
    //     auto RandomlySeededMersenneTwister () {
    //     std::mt19937 rng(std::random_device{}());
    //     rng.discard(700000);
    //     return rng;
    // }
    // --> Burn-in is good, but it does not change the fact that you seed a 624 byte state with just a single integer.

    // MT19937
    std::uniform_real_distribution<double> distribution(0, 1);
    auto start            = std::chrono::steady_clock::now();
    auto end              = std::chrono::steady_clock::now();
    auto duration_update  = 0.0;
    auto duration_advance = 0.0;
    // std::cout << distribution(generator) << std::endl; // check for reproducibility
    for (int i = 0; i < steps; ++i) {
        start = std::chrono::steady_clock::now();
        // update_velocities_random_lock(grid, vmax, prob, distribution, generator); // used without threadlocal MT
        update_velocities_random(grid, vmax, prob, distribution, generator); // used with threadlocal MT
        end = std::chrono::steady_clock::now();
        duration_update += std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        start = std::chrono::steady_clock::now();
        grid  = advance(grid);
        end   = std::chrono::steady_clock::now();
        duration_advance += std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    }
    std::cout << "Elapsed time in update: " << duration_update << " ms, in advance: " << duration_advance << "ms.\n";

    return 0;
}
