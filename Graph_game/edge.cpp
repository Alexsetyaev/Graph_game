#include "edge.hpp"
#include <iostream>
#include <random>

namespace {
int get_random(int start, int end) {
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<> distrib(start, end);
  return distrib(gen);
}

uni_course_cpp::Edge::Duration calculate_duration(
    const uni_course_cpp::Edge::Color& color) {
  switch (color) {
    case uni_course_cpp::Edge::Color::Grey:
      return get_random(1, 2);
    case uni_course_cpp::Edge::Color::Red:
      return get_random(2, 4);
    case uni_course_cpp::Edge::Color::Yellow:
      return get_random(1, 3);
    case uni_course_cpp::Edge::Color::Green:
      return get_random(1, 2);
  }
  throw std::runtime_error("Color not found!\n");
}

}  // namespace

namespace uni_course_cpp {

Edge::Edge(EdgeId id,
           VertexId first_vertex_id,
           VertexId second_vertex_id,
           const Edge::Color& color)
    : id_(id),
      first_vertex_id_(first_vertex_id),
      second_vertex_id_(second_vertex_id),
      color_(color),
      duration_(calculate_duration(color)) {}

Edge::Color Edge::get_color() const {
  return color_;
}

EdgeId Edge::get_id() const {
  return id_;
}

VertexId Edge::get_first_vertex_id() const {
  return first_vertex_id_;
}

VertexId Edge::get_second_vertex_id() const {
  return second_vertex_id_;
}
}  // namespace uni_course_cpp
