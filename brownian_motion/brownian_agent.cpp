#include "agent.h"
#include "position.h"

#include <iostream>
#include <random>

Position potential_gradient(Position p) { return p; }

template <class RNG>
void move(RNG &rng, const double noise, const double /*t*/, const double dt,
          Agent &agent) {
  static std::normal_distribution<double> standart_normal_dist(0.0, 1.0);

  const Position random_direction = {standart_normal_dist(rng),
                                     standart_normal_dist(rng)};

  agent.position = agent.position - dt * potential_gradient(agent.position) +
                   (noise * std::sqrt(dt)) * random_direction;
}

int main() {
  const int n = 1;

  std::random_device rd{};
  std::mt19937 rng{rd()};
  std::normal_distribution<double> standart_normal_dist(0.0, 1.0);

  std::vector<Agent> population(n);

  for (Agent &agent : population) {
    agent.position = {standart_normal_dist(rng), standart_normal_dist(rng)};
  }

  const double tmax = 1;
  const double dt = 0.1;

  for (double time = 0; time < tmax; time += dt) {
    for (Agent &agent : population) {
      move(rng, 0, time, dt, agent);
      std::cout << agent << "\n";
    }
  }

  return 0;
}
