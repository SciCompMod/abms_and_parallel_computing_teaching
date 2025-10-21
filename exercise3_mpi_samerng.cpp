// mpicxx -std=c++11 exercise3_mpi_samerng.cpp -o ex3_mpi_samerng
#include <iostream>
#include <stdlib.h>
#include <mpi.h>
#include <random>

int main(int argc, char** argv)
{
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    std::random_device rd{};
    std::vector<unsigned int> seed_data = {114381446,  2427727386, 806223567,  832414962,
                                           4121923627, 1581162203, 2468991901, 3237492387};
    std::seed_seq seed_s{seed_data.begin(), seed_data.end()};
    std::mt19937 generator{seed_s}; // initialize our Mersenne Twister with the std::seed_seq
    std::uniform_real_distribution<double> distribution(0, 1);

    // draw a few numbers (identical sequence on every rank)
    const int N = 5;
    std::vector<double> numbers(N);
    for (int i = 0; i < N; ++i) {
        numbers[i] = distribution(generator);
    }

    // print in rank order so output isn't interleaved
    for (int r = 0; r < size; ++r) {
        if (rank == r) {
            std::cout << "Rank " << rank << ": ";
            for (int i = 0; i < N; ++i) {
                std::cout << numbers[i] << " ";
            }
            std::cout << std::endl << std::flush;
        }
        MPI_Barrier(MPI_COMM_WORLD);
    }

    MPI_Finalize();
    return 0;
}