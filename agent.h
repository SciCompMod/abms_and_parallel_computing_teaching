#pragma once

#include "position.h"

#include <iostream>

struct Agent {
  Position position;

  friend std::ostream &operator<<(std::ostream &os, const Agent &a) {
    return os << "(" << a.position.x << ", " << a.position.y << ")";
  }
};
