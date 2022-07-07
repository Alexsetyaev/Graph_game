#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include "config.hpp"
#include "game_generator.hpp"
#include "graph.hpp"
#include "graph_generation_controller.hpp"
#include "graph_generator.hpp"
#include "graph_json_printing.hpp"
#include "graph_printing.hpp"
#include "graph_traverser_controller.hpp"
#include "logger.hpp"

void json_to_file(const std::string& str, const std::string& filepath) {
  std::ofstream json;
  json.open(filepath);
  if (!json.is_open()) {
    throw std::runtime_error("Can't open graph.json file");
  }
  json << str;
  json.close();
}

int handle_depth_input() {
  int depth = 0;
  std::cout << "Please, enter graph's depth:\n";
  std::cin >> depth;
  if (depth < 1) {
    throw std::runtime_error("Incorrect depth! Depth should be above zero!\n");
  }
  return depth;
}

int handle_new_vertices_count_input() {
  int count = 0;
  std::cout << "Please, enter graph's new vertices count:\n";
  std::cin >> count;
  if (count < 1) {
    throw std::runtime_error("Incorrect count! Count should be above zero!\n");
  }
  return count;
}

int handle_graphs_count_input() {
  int count = 0;
  std::cout << "Please, enter graph's count:\n";
  std::cin >> count;
  if (count < 1) {
    throw std::runtime_error("Incorrect count! Count should be above zero!\n");
  }
  return count;
}

int handle_threads_count_input() {
  int count = 0;
  std::cout << "Please, enter threads count:\n";
  std::cin >> count;
  if (count < 1) {
    throw std::runtime_error("Incorrect count! Count should be above zero!\n");
  }
  return count;
}

const uni_course_cpp::GraphGenerator::Params GraphGenerationParams(
    const int depth,
    const int new_vertices_count) {
  return uni_course_cpp::GraphGenerator::Params(depth, new_vertices_count);
}

void prepare_temp_directory() {
  std::filesystem::create_directory(
      std::string(uni_course_cpp::config::kTempDirectoryPath));
}

std::string generation_started_string(int graph_number) {
  std::stringstream output;
  output << "Graph " << graph_number << ", GenerationStarted";
  return output.str();
}
std::string generation_finished_string(int graph_number,
                                       const std::string& graph_description) {
  std::stringstream output;
  output << "Graph " << graph_number << ", GenerationFinished "
         << graph_description;
  return output.str();
}

std::string traversal_started_string(int graph_number) {
  std::stringstream output;
  output << "Graph " << graph_number << ", TraversalStarted";
  return output.str();
}

std::string traversal_finished_string(
    int graph_number,
    const std::vector<uni_course_cpp::GraphPath>& pathes) {
  std::stringstream output;
  output << "Graph " << graph_number << ", TraversalFinished, Paths: [\n";
  for (int index = 0; index < pathes.size(); ++index) {
    output << "  " << uni_course_cpp::printing::print_path(pathes[index]);
    if (index != pathes.size() - 1) {
      output << ",";
    }
    output << "\n";
  }
  output << "]\n";
  return output.str();
}

std::vector<uni_course_cpp::Graph> generate_graphs(
    const uni_course_cpp::GraphGenerator::Params& params,
    int graphs_count,
    int threads_count) {
  auto generation_controller = uni_course_cpp::GraphGenerationController(
      threads_count, graphs_count, params);

  auto& logger = uni_course_cpp::Logger::get_logger();

  auto graphs = std::vector<uni_course_cpp::Graph>();
  graphs.reserve(graphs_count);

  generation_controller.generate(
      [&logger](int index) { logger.log(generation_started_string(index)); },
      [&logger, &graphs](int index, uni_course_cpp::Graph graph) {
        const auto graph_description =
            uni_course_cpp::printing::print_graph(graph);
        logger.log(generation_finished_string(index, graph_description));
        const auto graph_json =
            uni_course_cpp::printing::json::graph_to_string(graph);
        graphs.push_back(std::move(graph));
        json_to_file(graph_json,
                     std::string(uni_course_cpp::config::kTempDirectoryPath) +
                         "graph_" + std::to_string(index) + ".json");
      });

  return graphs;
}

void traverse_graphs(const std::vector<uni_course_cpp::Graph>& graphs) {
  auto traversal_controller = uni_course_cpp::GraphTraversalController(graphs);
  auto& logger = uni_course_cpp::Logger::get_logger();

  traversal_controller.traverse(
      [&logger](int index) { logger.log(traversal_started_string(index)); },
      [&logger](int index, std::vector<uni_course_cpp::GraphPath> paths) {
        logger.log(traversal_finished_string(index, paths));
      });
}

std::string game_preparing_string() {
  return "Game is Preparing...";
}
std::string game_ready_string(const uni_course_cpp::Game& game) {
  std::stringstream output;
  output << "Game is Ready " << uni_course_cpp::printing::print_game(game);
  return output.str();
}

std::string shortest_path_searching_string() {
  return "Searching for Shortest Path...";
}
std::string shortest_path_ready_string(const uni_course_cpp::GraphPath& path) {
  return "Shortest Path: " + uni_course_cpp::printing::print_path(path);
}

std::string fastest_path_searching_string() {
  return "Searching for Fastest Path...";
}
std::string fastest_path_ready_string(const uni_course_cpp::GraphPath& path) {
  return "Fastest Path: " + uni_course_cpp::printing::print_path(path);
}

int main() {
  const int depth = handle_depth_input();
  const int new_vertices_count = handle_new_vertices_count_input();
  prepare_temp_directory();

  auto& logger = uni_course_cpp::Logger::get_logger();
  logger.log(game_preparing_string());

  auto params = GraphGenerationParams(depth, new_vertices_count);
  const auto game_generator = uni_course_cpp::GameGenerator(std::move(params));
  const auto game = game_generator.generate();

  logger.log(game_ready_string(game));
  logger.log(shortest_path_searching_string());

  const auto shortest_path = game.find_shortest_path();

  logger.log(shortest_path_ready_string(shortest_path));
  logger.log(fastest_path_searching_string());

  const auto fastest_path = game.find_fastest_path();

  logger.log(fastest_path_ready_string(fastest_path));

  const auto map_json =
      uni_course_cpp::printing::json::graph_to_string(game.map());
  json_to_file(map_json, "map.json");

  return 0;
}
