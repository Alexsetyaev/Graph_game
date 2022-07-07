#include "graph_printing.hpp"
#include <array>
#include <iostream>
#include <sstream>

namespace uni_course_cpp {
namespace printing {

std::string color_to_string(const uni_course_cpp::Edge::Color& color) {
  switch (color) {
    case uni_course_cpp::Edge::Color::Grey:
      return "grey";

    case uni_course_cpp::Edge::Color::Yellow:
      return "yellow";

    case uni_course_cpp::Edge::Color::Green:
      return "green";

    case uni_course_cpp::Edge::Color::Red:
      return "red";
  }

  throw std::runtime_error("Failed to determine color");
}

std::string print_graph(const uni_course_cpp::Graph& graph) {
  std::stringstream graph_string;
  graph_string << "{\n  depth: " << graph.get_depth()
               << ",\n  vertices: {amount: " << graph.get_vertices().size()
               << ", distribution: [";
  for (int depth = 0; depth < graph.get_depth() - 1; ++depth) {
    graph_string << graph.get_vertex_ids_at_depth(depth).size() << ", ";
  }
  graph_string << graph.get_vertex_ids_at_depth(graph.get_depth() - 1).size()
               << "]},\n  edges: {amount: " << graph.get_vertices().size()
               << ", distribution: {";
  const std::array<uni_course_cpp::Edge::Color, 4> colors = {
      uni_course_cpp::Edge::Color::Grey, uni_course_cpp::Edge::Color::Green,
      uni_course_cpp::Edge::Color::Yellow, uni_course_cpp::Edge::Color::Red};
  for (const auto& color : colors) {
    if (color != colors[0]) {
      graph_string << ", ";
    }
    graph_string << color_to_string(color) << ": "
                 << graph.get_colored_edge_ids(color).size();
  }

  graph_string << "}}\n}";
  return graph_string.str();
}

std::string print_path(const GraphPath& path) {
  std::stringstream path_string;
  path_string << "{vertices: [";
  for (const auto vertex_id : path.vertex_ids()) {
    path_string << vertex_id;
    if (vertex_id != path.vertex_ids()[path.vertex_ids().size() - 1]) {
      path_string << ", ";
    }
  }
  path_string << "], distance: " << path.edge_ids().size()
              << ", duration: " << path.duration() << "}";
  return path_string.str();
}

std::string print_game(const uni_course_cpp::Game& game) {
  std::stringstream result;
  result << "{\n  map:\n"
         << print_graph(game.map())
         << ",\n  knight position: {vertex_id: " << game.knight_position()
         << ", depth: " << game.map().get_vertex_depth(game.knight_position())
         << "},\n  princess position: {vertex_id: " << game.princess_position()
         << ", depth: " << game.map().get_vertex_depth(game.princess_position())
         << "}\n}\n";
  return result.str();
}

}  // namespace printing
}  // namespace uni_course_cpp
