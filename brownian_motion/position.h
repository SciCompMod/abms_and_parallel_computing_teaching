#pragma once

struct Position {
  double x, y;

  Position &operator*=(double scalar) {
    x *= scalar;
    y *= scalar;
    return *this;
  }

  Position operator*(double scalar) {
    Position scaled = *this;
    scaled *= scalar;
    return scaled;
  }

  Position &operator+=(Position other) {
    x += other.x;
    y += other.y;
    return *this;
  }

  Position operator+(Position other) {
    Position translated = *this;
    translated += other;
    return translated;
  }

  Position &operator-=(Position other) {
    x -= other.x;
    y -= other.y;
    return *this;
  }

  Position operator-(Position other) {
    Position translated = *this;
    translated -= other;
    return translated;
  }
};

Position operator*(double scalar, Position position) {
  return position * scalar;
}
