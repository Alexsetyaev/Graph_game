#include "graph_generator.hpp"
#include <atomic>
#include <functional>
#include <iostream>
#include <list>
#include <mutex>
#include <random>
#include <thread>

namespace {
constexpr double kGreenProbability = 0.1;
constexpr double kRedProbability = 0.33;
const int kMaxThreadCount = std::thread::hardware_concurrency();

bool check_probability(double probability) {
  std::random_device rd;
  std::mt19937 gen(rd());
  std::bernoulli_distribution d(probability);
  return d(gen);
}

int get_random_index(int size) {
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<> distrib(0, size);
  return distrib(gen);
}
}  // namespace
namespace uni_course_cpp {

void GraphGenerator::generate_grey_edges(Graph& graph) const {
  using JobCallback = std::function<void()>;
  auto jobs = std::list<JobCallback>();
  std::mutex mutex;
  std::atomic<bool> check_termination = false;
  std::atomic<int> jobs_done = 0;
  for (int job_number = 0; job_number < params_.new_vertices_count();
       job_number++) {
    jobs.push_back([&jobs_done, &mutex, &graph, this]() {
      generate_grey_branch(graph, 0, 0, mutex);
      jobs_done++;
    });
  }
  const auto worker = [&check_termination, &mutex, &jobs]() {
    while (true) {
      if (check_termination) {
        return;
      }

      const auto optional_job = [&jobs,
                                 &mutex]() -> std::optional<JobCallback> {
        const std::lock_guard<std::mutex> lock(mutex);
        if (jobs.size() > 0) {
          const auto job = jobs.back();
          jobs.pop_back();
          return job;
        }
        return std::nullopt;
      }();

      if (optional_job.has_value()) {
        const auto& job = optional_job.value();
        job();
      }
    }
  };

  const auto thread_count =
      std::min(kMaxThreadCount, params_.new_vertices_count());
  auto threads = std::vector<std::thread>();
  threads.reserve(thread_count);
  for (int thread_num = 0; thread_num < thread_count; ++thread_num) {
    threads.push_back(std::thread(worker));
  }
  while (jobs_done < params_.new_vertices_count()) {
  }
  check_termination = true;
  for (auto& thread : threads) {
    thread.join();
  }
}

void GraphGenerator::generate_grey_branch(Graph& graph,
                                          Graph::Depth depth,
                                          const VertexId& vertex_id,
                                          std::mutex& mutex) const {
  if (depth >= params_.depth() - 1) {
    return;
  }
  if (!check_probability(1.0 - (double)depth / (double)params_.depth())) {
    return;
  }

  const auto& new_vertex = [&graph, &mutex, &vertex_id]() {
    const std::lock_guard<std::mutex> lock(mutex);
    const auto& added_vertex = graph.add_vertex();
    graph.add_edge(vertex_id, added_vertex.get_id());
    return added_vertex;
  }();
  for (int job_number = 0; job_number < params_.new_vertices_count();
       ++job_number) {
    generate_grey_branch(graph, depth + 1, new_vertex.get_id(), mutex);
  }
}

void GraphGenerator::generate_green_edges(Graph& graph,
                                          std::mutex& mutex) const {
  for (const auto& vertex : graph.get_vertices()) {
    if (check_probability(kGreenProbability)) {
      const std::lock_guard<std::mutex> lock(mutex);
      graph.add_edge(vertex.get_id(), vertex.get_id());
    }
  }
}
std::vector<VertexId> get_unconected_vertex_ids(
    const Graph& graph,
    const Vertex& vertex,
    const std::vector<VertexId>& vertices_at_depth,
    std::mutex& mutex) {
  std::vector<VertexId> vertices_ids;
  const auto vertex_id = vertex.get_id();
  for (const auto& another_vertex : vertices_at_depth) {
    const auto is_connected_bool = [&graph, &vertex_id, &another_vertex,
                                    &mutex]() {
      const std::lock_guard<std::mutex> lock(mutex);
      return graph.is_connected(vertex_id, another_vertex);
    }();

    if (!is_connected_bool) {
      vertices_ids.push_back(another_vertex);
    }
  }
  return vertices_ids;
}

void GraphGenerator::generate_yellow_edges(Graph& graph,
                                           std::mutex& mutex) const {
  for (const auto& first_vertex : graph.get_vertices()) {
    if (graph.get_vertex_depth(first_vertex.get_id()) >=
        graph.get_depth() - 1) {
      continue;
    }

    if (check_probability(
            (double)graph.get_vertex_depth(first_vertex.get_id()) /
            ((double)params_.depth() - 1.0))) {
      const auto unconnected_vertex_ids = get_unconected_vertex_ids(
          graph, first_vertex,
          graph.get_vertex_ids_at_depth(
              graph.get_vertex_depth(first_vertex.get_id()) + 1),
          mutex);

      if (unconnected_vertex_ids.size() > 0) {
        const VertexId second_vertex_id =
            unconnected_vertex_ids[get_random_index(
                unconnected_vertex_ids.size() - 1)];
        const std::lock_guard<std::mutex> lock(mutex);
        graph.add_edge(first_vertex.get_id(), second_vertex_id);
      }
    }
  }
}
void GraphGenerator::generate_red_edges(Graph& graph, std::mutex& mutex) const {
  for (const auto& first_vertex : graph.get_vertices()) {
    if (graph.get_vertex_depth(first_vertex.get_id()) >=
        graph.get_depth() - 2) {
      continue;
    }

    if (check_probability(kRedProbability)) {
      const std::vector<VertexId>& second_vertices_ids =
          graph.get_vertex_ids_at_depth(
              graph.get_vertex_depth(first_vertex.get_id()) + 2);
      if (second_vertices_ids.size() > 0) {
        const VertexId second_vertex_id = second_vertices_ids[get_random_index(
            second_vertices_ids.size() - 1)];
        const std::lock_guard<std::mutex> lock(mutex);
        graph.add_edge(first_vertex.get_id(), second_vertex_id);
      }
    }
  }
}

Graph GraphGenerator::generate() const {
  auto graph = Graph();
  graph.add_vertex();
  std::mutex mutex;
  generate_grey_edges(graph);
  std::thread green_thread(
      [&graph, &mutex, this]() { generate_green_edges(graph, mutex); });
  std::thread yellow_thread(
      [&graph, &mutex, this]() { generate_yellow_edges(graph, mutex); });
  std::thread red_thread(
      [&graph, &mutex, this]() { generate_red_edges(graph, mutex); });
  green_thread.join();
  yellow_thread.join();
  red_thread.join();
  return graph;
}

}  // namespace uni_course_cpp
