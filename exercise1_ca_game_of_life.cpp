#include <iostream>
#include <vector>
#include <thread>
#include <chrono>

using namespace std;

const char ALIVE = 'X';
const char DEAD  = '.';

// Type alias for readability
using Grid = vector<vector<bool>>;

// Print the current state of the grid
void print_grid(const Grid& grid)
{
    for (size_t y = 0; y < grid.size(); ++y) {
        for (size_t x = 0; x < grid[0].size(); ++x) {
            std::cout << (grid[y][x] ? ALIVE : DEAD);
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}

// Count alive neighbors of a cell at (x, y)
int count_alive_neighbors(const Grid& grid, int x, int y)
{
    int count = 0;
    for (int dy = -1; dy <= 1; ++dy) {
        for (int dx = -1; dx <= 1; ++dx) {
            if (dx == 0 && dy == 0)
                continue; // skip self
            int nx = x + dx;
            int ny = y + dy;
            // using boundary conditions where all boundaries are considered dead
            if (nx >= 0 && nx < grid[0].size() && ny >= 0 && ny < grid.size()) {
                count += grid[ny][nx];
            }
        }
    }
    return count;
}

// Compute the next generation
Grid advance(const Grid& current)
{
    Grid next = current; // create copy of the current grid
    for (int y = 0; y < current.size(); ++y) {
        for (int x = 0; x < current[0].size(); ++x) {
            int neighbors = count_alive_neighbors(current, x, y);
            if (current[y][x]) {
                next[y][x] = (neighbors == 2 || neighbors == 3);
            }
            else {
                next[y][x] = (neighbors == 3);
            }
        }
    }
    return next;
}

// Initialize a grid with random state
Grid random_grid(int dimX, int dimY)
{
    Grid grid(dimY, vector<bool>(dimX, false));
    for (int y = 0; y < dimY; ++y) {
        for (int x = 0; x < dimX; ++x) {
            grid[y][x] = rand() % 2;
        }
    }
    return grid;
}

Grid static_grid(int dimX, int dimY)
{
    Grid grid(dimY, vector<bool>(dimX, false));
    for (int y = 0; y < dimY; ++y) {
        for (int x = 0; x < dimX; ++x) {
            grid[y][x] = 0;
        }
    }
    grid[dimY / 2][dimX / 2]         = 1;
    grid[dimY / 2][dimX / 2 + 1]     = 1;
    grid[dimY / 2 + 1][dimX / 2]     = 1;
    grid[dimY / 2 + 1][dimX / 2 + 1] = 1;
    return grid;
}

Grid alternating_grid(int dimX, int dimY)
{
    Grid grid(dimY, vector<bool>(dimX, false));
    for (int y = 0; y < dimY; ++y) {
        for (int x = 0; x < dimX; ++x) {
            grid[y][x] = 0;
        }
    }
    grid[dimY / 2][dimX / 2]     = 1;
    grid[dimY / 2][dimX / 2 + 1] = 1;
    grid[dimY / 2][dimX / 2 - 1] = 1;
    return grid;
}

void parse_args(int argc, char* argv[], int& dimX, int& dimY, int& generations)
{
    for (int i = 1; i < argc - 1; ++i) {
        string arg = argv[i];
        if (arg == "--dimX") {
            dimX = std::stoi(argv[++i]);
        }
        else if (arg == "--dimY") {
            dimY = std::stoi(argv[++i]);
        }
        else if (arg == "--gen") {
            generations = std::stoi(argv[++i]);
        }
        else {
            std::cerr << "Unknown or malformed argument. Using standard parameters." << std::endl;
            std::cerr << "Usage: " << argv[0] << " [--dimX L] [--dimY M] [--gen N]\n";
        }
    }
}

int main(int argc, char* argv[])
{

    int dimX        = 10;
    int dimY        = 10;
    int generations = 5;

    parse_args(argc, argv, dimX, dimY, generations);
    std::cout << "Grid dimensions: " << dimX << "x" << dimY << ", Generations: " << generations << std::endl;

    srand(0);
    Grid grid = alternating_grid(dimX, dimY);
    print_grid(grid);
    for (int gen = 0; gen < generations; ++gen) {
        grid = advance(grid);
        print_grid(grid);
        this_thread::sleep_for(std::chrono::milliseconds(200));
    }

    for (int scale = 1; scale <= 9; ++scale) {
        dimX *= 2;
        dimY *= 2;
        std::cout << "Scaled grid dimensions: " << dimX << "x" << dimY << std::endl;
        Grid grid  = random_grid(dimX, dimY);
        auto start = std::chrono::steady_clock::now();
        for (int gen = 0; gen < generations; ++gen) {
            grid = advance(grid);
        }
        auto end      = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        std::cout << "Elapsed time: " << duration.count() << " ms\n";
    }
    return 0;
}
