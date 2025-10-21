#include <iostream> // used for output with cout etc.
#include <string>
#include <vector>

void output_vec(std::string outstr, int start, std::vector<double> vector)
{
    std::cout << outstr << std::endl;
    for (int i = 0; i < vector.size(); i++) {
        std::cout << "Day " << i - start << ": " << vector[i] << std::endl;
    }
}

int main()
{

    size_t days     = 3; // simulation days
    size_t n_cont   = 10; // number of contacts
    double r_transm = 0.05; // risk of transmission upon contact
    size_t t_rec    = 3; // time to recover

    std::vector<double> new_inf(days + t_rec);
    // init infected
    new_inf[0] = 1;
    new_inf[1] = 2;
    new_inf[2] = 3;

    // simulation
    for (size_t i = t_rec; i < days + t_rec; i++) {
        double curr_infected = 0;
        for (size_t j = 0; j < t_rec; j++) {
            curr_infected += new_inf[i + j - t_rec];
        }
        new_inf[i] = curr_infected * n_cont * r_transm;
    }

    // output
    output_vec("New infections", int(t_rec), new_inf);

    return 0;
}
