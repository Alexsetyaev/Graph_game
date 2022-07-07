#pragma once
#include <string>
#include "game.hpp"
#include "graph.hpp"
#include "graph_path.hpp"

namespace uni_course_cpp {
namespace printing {
std::string color_to_string(const uni_course_cpp::Edge::Color& color);
std::string print_graph(const uni_course_cpp::Graph& graph);
std::string print_path(const GraphPath& path);
std::string print_game(const uni_course_cpp::Game& game);

}  // namespace printing
}  // namespace uni_course_cpp
