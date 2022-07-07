#pragma once

#include <vector>
#include "graph.hpp"

namespace uni_course_cpp {
struct GraphPath {
 public:
  using Distance = int;

  Distance distance() const;
  Edge::Duration duration() const { return duration_; }
  std::vector<VertexId> vertex_ids() const { return vertex_ids_; }
  std::vector<EdgeId> edge_ids() const { return edge_ids_; }
  GraphPath(Edge::Duration new_duration,
            std::vector<VertexId>&& new_vertex_ids,
            std::vector<EdgeId>&& new_edge_ids)
      : duration_(new_duration),
        vertex_ids_(new_vertex_ids),
        edge_ids_(new_edge_ids) {}

 private:
  std::vector<VertexId> vertex_ids_;
  std::vector<EdgeId> edge_ids_;
  Edge::Duration duration_ = 0;
};
}  // namespace uni_course_cpp
