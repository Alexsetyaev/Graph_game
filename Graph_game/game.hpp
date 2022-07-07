#pragma once

#include <vector>
#include "graph.hpp"
#include "graph_path.hpp"

namespace uni_course_cpp {
class Game {
 public:
  Game(Graph&& map, VertexId knight_position, VertexId princess_position)
      : map_(map),
        knight_position_(knight_position),
        princess_position_(princess_position) {}
  // Traverse by `Distance`
  GraphPath find_shortest_path() const;
  // Traverse by `Duration`
  GraphPath find_fastest_path() const;
  Graph map() const { return map_; }
  VertexId knight_position() const { return knight_position_; }
  VertexId princess_position() const { return princess_position_; }

 private:
  Graph map_;
  VertexId knight_position_;
  VertexId princess_position_;
};
}  // namespace uni_course_cpp
