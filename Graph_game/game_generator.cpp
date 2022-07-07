#include "game_generator.hpp"
#include <random>

namespace {
uni_course_cpp::VertexId get_random_vertex(
    const std::vector<uni_course_cpp::VertexId>& vertices) {
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<> distrib(0, vertices.size() - 1);
  return vertices[distrib(gen)];
}

}  // namespace

namespace uni_course_cpp {

Game GameGenerator::generate() const {
  const auto graph_generator = GraphGenerator(params_);
  auto map = graph_generator.generate();
  const auto& final_vertices = map.get_vertex_ids_at_depth(map.get_depth() - 1);

  return Game(std::move(map), 0, get_random_vertex(final_vertices));
}

}  // namespace uni_course_cpp
