// mpicxx -std=c++11 exercise3_nagel_schreckenberg_mpi.cpp -o ex3_nasch_mpi
// mpirun -N 4 ./ex3_nasch_mpi
#include <chrono>
#include <cstddef>
#include <fstream>
#include <iostream>
#include <mpi.h>
#include <numeric>
#include <random>
#include <thread>
#include <vector>

const char FREE = '.';

// Type alias for readability
using Grid = std::vector<int>;

void update_velocities(Grid &grid, size_t vmax, double prob,
                       std::uniform_real_distribution<double> &dist,
                       std::mt19937 &gen) {
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
      if (grid[i] > 0 && random_value < prob) { // 'prob' chance to slow down
        grid[i]--;
      }
    }
  }
}

// Compute the next step
Grid advance(const Grid &grid) {
  Grid new_grid(grid.size(),
                -1); // Initialize new grid with -1 (free positions)
#pragma omp parallel for schedule(static, 1)
  for (int i = 0; i < grid.size(); ++i) {
    if (grid[i] >= 0) { // If there's a car at position i
      int new_pos = (i + grid[i]) % grid.size();
      new_grid[new_pos] = grid[i]; // Move the car to its new position
    }
  }
  return new_grid; // Update the grid to the new state
}

// Initialize a grid with random state
Grid random_grid(size_t length, size_t ncars, size_t vmax) {
  Grid grid = Grid(length, -1);
  size_t cars_placed = 0;
  while (cars_placed < ncars) {
    int pos = rand() % length; // Random position between 0 and length-1
    // std::cout << "Cars placed " << cars_placed << ", Trying to place car at
    // position: " << pos << std::endl;
    if (grid[pos] == -1) {           // If the position is free
      grid[pos] = rand() % vmax + 1; // Random velocity between 1 and vmax
      cars_placed++;
    }
  }
  return grid;
}

void parse_args(int argc, char *argv[], int &length, int &ncars, int &vmax,
                int &steps, double &prob, int &seed) {
  for (int i = 1; i < argc - 1; ++i) {
    std::string arg = argv[i];
    if (arg == "--length") {
      length = std::stoi(argv[++i]);
    } else if (arg == "--ncars") {
      ncars = std::stoi(argv[++i]);
    } else if (arg == "--vmax") {
      vmax = std::stoi(argv[++i]);
    } else if (arg == "--steps") {
      steps = std::stoi(argv[++i]);
    } else if (arg == "--prob") {
      prob = std::stod(argv[++i]);
    } else if (arg == "--seed") {
      seed = std::stoi(argv[++i]);
    } else {
      std::cerr << "Unknown or malformed argument. Using standard parameters."
                << std::endl;
      std::cerr << "Usage: " << argv[0]
                << " [--length L] [--ncars N] [--vmax V] [--steps T] [--prob "
                   "P] [--seed S]\n";
    }
  }
}

int main(int argc, char *argv[]) {

  MPI_Init(&argc, &argv);

  int rank;
  int size;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank); // current process ID
  MPI_Comm_size(MPI_COMM_WORLD, &size); // total number of processes

  int length = 1e7;
  int ncars = 1e6; // Number of cars
  int vmax = 5;
  int steps = 5;
  double prob = 0.3; // Probability of slowing down
  int seed = 0;      // Random seed

  parse_args(argc, argv, length, ncars, vmax, steps, prob, seed);
  std::cout << "Parameters: L=" << length << ", ncars=" << ncars
            << ", vmax=" << vmax << ", time steps: " << steps
            << ", probability: " << prob << ", seed: " << seed << std::endl;

  // sequential random grid creation (not thread-safe)
  srand(seed);
  Grid grid = random_grid(length, ncars, vmax);

  std::random_device rd{};
  std::vector<unsigned int> seed_data = {114381446,  2427727386, 806223567,
                                         832414962,  4121923627, 1581162203,
                                         2468991901, 3237492387};
  std::seed_seq seed_s{seed_data.begin(), seed_data.end()};
  std::mt19937 generator{
      seed_s}; // initialize our Mersenne Twister with the std::seed_seq

  std::uniform_real_distribution<double> distribution(0, 1);
  for (int i = 0; i < steps; ++i) {
    update_velocities(grid, vmax, prob, distribution, generator);
    grid = advance(grid);
  }

  double average = 0;
  for (size_t i = 0; i < grid.size(); i++) {
    if (grid[i] > 0) {
      average += grid[i];
    }
  }
  average /= ncars;
  std::cout << "Average on rank " << rank << ": " << average << "\n";

  if (size == 2) {
    if (rank == 0) {
      // Process 0: send an integer to process 1
      MPI_Send(&average, 1, MPI_DOUBLE, 1, 0, MPI_COMM_WORLD);
    } else if (rank == 1) {
      // Process 1: receive that integer
      double received_avg;
      MPI_Recv(&received_avg, 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD,
               MPI_STATUS_IGNORE);
      std::cout << "Process 0 received average: " << received_avg
                << " from process 1\n";
    }
  } else if (size > 2) {
    std::vector<double> recvbuf;
    if (rank == 0) {
      recvbuf.resize(size);
    }
    MPI_Gather(&average, 1, MPI_DOUBLE, rank == 0 ? recvbuf.data() : nullptr, 1,
               MPI_DOUBLE, 0, MPI_COMM_WORLD);

    if (rank == 0) {
      std::cout << "Gathered averages on rank 0:";
      for (size_t i = 0; i < size; ++i) {
        std::cout << " " << recvbuf.at(i);
      }
      std::cout << "\n";
    }
  }

  MPI_Finalize();
  return 0;
}
