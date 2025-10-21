// g++ -std=c++11 exercise1_nagel_schreckenberg.cpp -o ex1_nasch
#include <cstddef>
#include <iostream>
#include <vector>
#include <chrono>
#include <fstream>
#include <numeric>
#include <thread>

const char FREE = '.';

// Type alias for readability
using Grid = std::vector<int>;

// Print the current state of the grid
void print_grid(const Grid& grid)
{
    for (size_t i = 0; i < grid.size(); ++i) {
        std::cout << (grid[i] > -1 ? static_cast<char>('0' + grid[i]) : FREE);
    }
    std::cout << std::endl;
}

void update_velocities(Grid& grid, size_t vmax, double prob)
{
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
            if (grid[i] > 0 && (rand() % 100) < 100 * prob) { // 'prob' chance to slow down
                grid[i]--;
            }
        }
    }
}

// Compute the next step
Grid advance(const Grid& grid)
{
    Grid new_grid(grid.size(), -1); // Initialize new grid with -1 (free positions)
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

    int length  = 50;
    int ncars   = 13; // Number of cars
    int vmax    = 5;
    int steps   = 5;
    double prob = 0.3; // Probability of slowing down
    int seed    = 0; // Random seed

    parse_args(argc, argv, length, ncars, vmax, steps, prob, seed);
    std::cout << "Parameters: L=" << length << ", ncars=" << ncars << ", vmax=" << vmax << ", time steps: " << steps
              << ", probability: " << prob << ", seed: " << seed << std::endl;

    srand(seed);
    Grid grid = random_grid(length, ncars, vmax);
    print_grid(grid);
    for (int i = 0; i < steps; ++i) {
        update_velocities(grid, vmax, prob);
        grid = advance(grid);
        print_grid(grid);
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }

    length           = 10000;
    steps            = 1e3;
    int scale_length = 1;
    length *= scale_length; // Scale the length of the road
    auto start = std::chrono::steady_clock::now();
    std::vector<std::tuple<double, double, double>> results;

    for (double p : {0.0, 0.2, 0.4, 0.6, 0.8, 1.0}) {
        for (int ncars : {1, 3, 5, 7, 9, 13, 17, 21, 25, 30, 35, 40, 45, 50, 65, 80, 95}) {
            ncars *= scale_length; // Scale the number of cars
            double density = (double)ncars / (double)length;
            auto val_neg   = steps * (length - ncars); // negative values in the grid over all steps
            for (int seed = 1; seed <= 1; ++seed) {
                srand(seed);
                auto velocities = val_neg;
                Grid grid       = random_grid(length, ncars, vmax);

                for (int i = 0; i < steps; ++i) {
                    update_velocities(grid, vmax, p);
                    grid = advance(grid);
                    velocities += std::accumulate(grid.begin(), grid.end(), 0);
                }

                double flow = velocities / (double)(steps * length);
                results.push_back(std::tuple<double, double, double>(p, density, flow));
            }
        }
    }

    auto end      = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "Elapsed time: " << duration.count() << " ms\n";

    std::ofstream csv_file("results.csv");
    csv_file << "probability,density,flow\n";
    for (const auto& tuple : results) {
        csv_file << std::get<0>(tuple) << "," << std::get<1>(tuple) << "," << std::get<2>(tuple) << "\n";
    }
    csv_file.close();

    return 0;
}