

#pragma once

#include "vertex.hpp"

namespace uni_course_cpp {
using EdgeId = int;
struct Edge {
 public:
  using Duration = int;
  enum class Color { Red, Grey, Green, Yellow };
  Edge(EdgeId id,
       VertexId first_vertex_id,
       VertexId second_vertex_id,
       const Edge::Color& color);
  EdgeId get_id() const;
  VertexId get_first_vertex_id() const;
  VertexId get_second_vertex_id() const;
  Edge::Color get_color() const;
  Duration get_duration() const { return duration_; }

 private:
  Duration duration_ = 0;
  EdgeId id_ = 0;
  VertexId first_vertex_id_ = 0;
  VertexId second_vertex_id_ = 0;
  Edge::Color color_ = Edge::Color::Grey;
};
}  // namespace uni_course_cpp
