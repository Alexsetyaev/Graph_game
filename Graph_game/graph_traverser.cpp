#include "graph_traverser.hpp"
#include <atomic>
#include <cassert>
#include <climits>
#include <functional>
#include <list>
#include <mutex>
#include <queue>
#include <thread>
#include <unordered_map>
#include <vector>
#include "graph.hpp"

namespace {
constexpr uni_course_cpp::GraphPath::Distance MAX_DISTANCE = INT_MAX;
constexpr uni_course_cpp::Edge::Duration MAX_DURATION = INT_MAX;
constexpr uni_course_cpp::VertexId START_VERTEX_ID = 0;
const int MAX_THREADS_COUNT = std::thread::hardware_concurrency();
}  // namespace

namespace uni_course_cpp {

GraphPath GraphTraverser::find_shortest_path(
    const VertexId& source_vertex_id,
    const VertexId& destination_vertex_id) const {
  std::unordered_map<VertexId, GraphPath::Distance> distances;
  std::unordered_map<VertexId, std::vector<VertexId>> vertex_ids;
  std::unordered_map<VertexId, std::vector<EdgeId>> edge_ids;
  std::unordered_map<VertexId, Edge::Duration> durations;
  for (const auto& vertex : graph_.get_vertices()) {
    distances[vertex.get_id()] = MAX_DISTANCE;
  }

  vertex_ids[source_vertex_id].push_back(source_vertex_id);
  distances[source_vertex_id] = 0;
  std::queue<VertexId> pass_waiting;
  pass_waiting.push(source_vertex_id);

  while (!pass_waiting.empty()) {
    const auto current_vertex_id = pass_waiting.front();
    pass_waiting.pop();
    for (const auto& edge_id :
         graph_.get_connected_edges_ids(current_vertex_id)) {
      const auto& edge = graph_.get_edge(edge_id);
      const auto next_vertex_id = edge.get_first_vertex_id() +
                                  edge.get_second_vertex_id() -
                                  current_vertex_id;
      if (distances[current_vertex_id] + 1 < distances[next_vertex_id]) {
        pass_waiting.push(next_vertex_id);
        distances[next_vertex_id] = distances[current_vertex_id] + 1;
        vertex_ids[next_vertex_id] = vertex_ids[current_vertex_id];
        edge_ids[next_vertex_id] = edge_ids[current_vertex_id];
        durations[next_vertex_id] = durations[current_vertex_id];

        vertex_ids[next_vertex_id].push_back(next_vertex_id);
        edge_ids[next_vertex_id].push_back(edge_id);
        durations[next_vertex_id] += edge.get_duration();
      }
    }
  }
  return GraphPath(durations[destination_vertex_id],
                   std::move(vertex_ids[destination_vertex_id]),
                   std::move(edge_ids[destination_vertex_id]));
}

GraphPath GraphTraverser::find_fastest_path(
    const VertexId& source_vertex_id,
    const VertexId& destination_vertex_id) const {
  std::unordered_map<VertexId, std::vector<VertexId>> vertex_ids;
  std::unordered_map<VertexId, std::vector<EdgeId>> edge_ids;
  std::unordered_map<VertexId, Edge::Duration> path_durations;
  std::unordered_map<VertexId, Edge::Duration> durations;
  for (const auto& vertex : graph_.get_vertices()) {
    durations[vertex.get_id()] = MAX_DURATION;
  }
  durations[source_vertex_id] = 0;

  vertex_ids[source_vertex_id].push_back(source_vertex_id);
  std::queue<VertexId> pass_waiting;
  pass_waiting.push(source_vertex_id);

  while (!pass_waiting.empty()) {
    const auto current_vertex_id = pass_waiting.front();
    pass_waiting.pop();
    for (const auto& edge_id :
         graph_.get_connected_edges_ids(current_vertex_id)) {
      const auto& edge = graph_.get_edge(edge_id);
      const auto next_vertex_id = edge.get_first_vertex_id() +
                                  edge.get_second_vertex_id() -
                                  current_vertex_id;
      if (durations[current_vertex_id] + edge.get_duration() <
          durations[next_vertex_id]) {
        pass_waiting.push(next_vertex_id);
        durations[next_vertex_id] =
            durations[current_vertex_id] + edge.get_duration();
        vertex_ids[next_vertex_id] = vertex_ids[current_vertex_id];
        edge_ids[next_vertex_id] = edge_ids[current_vertex_id];
        path_durations[next_vertex_id] = path_durations[current_vertex_id];

        vertex_ids[next_vertex_id].push_back(next_vertex_id);
        edge_ids[next_vertex_id].push_back(edge_id);
        path_durations[next_vertex_id] += edge.get_duration();
      }
    }
  }
  return GraphPath(durations[destination_vertex_id],
                   std::move(vertex_ids[destination_vertex_id]),
                   std::move(edge_ids[destination_vertex_id]));
}

std::vector<GraphPath> GraphTraverser::find_all_paths() const {
  std::vector<GraphPath> paths;
  using JobCallback = std::function<void()>;
  auto jobs = std::list<JobCallback>();
  std::mutex mutex;
  std::atomic<int> finished_paths = 0;
  std::atomic<bool> should_terminate = false;
  std::vector<VertexId> finish_vertex_ids =
      graph_.get_vertex_ids_at_depth(graph_.get_depth() - 1);
  for (const auto& end_vertex_id : finish_vertex_ids) {
    jobs.push_back([end_vertex_id, &paths, &mutex, &finished_paths, this]() {
      const GraphPath new_path =
          find_shortest_path(START_VERTEX_ID, end_vertex_id);
      {
        const std::lock_guard lock(mutex);
        paths.push_back(std::move(new_path));
      }
      finished_paths++;
    });
  }
  const auto worker = [&should_terminate, &mutex, &jobs]() {
    while (true) {
      if (should_terminate) {
        return;
      }
      const auto job_optional = [&jobs,
                                 &mutex]() -> std::optional<JobCallback> {
        const std::lock_guard lock(mutex);
        if (!jobs.empty()) {
          const auto job = jobs.front();
          jobs.pop_front();
          return job;
        }
        return std::nullopt;
      }();
      if (job_optional.has_value()) {
        const auto& job = job_optional.value();
        job();
      }
    }
  };

  const auto threads_num = std::min(
      MAX_THREADS_COUNT,
      static_cast<int>(
          graph_.get_vertex_ids_at_depth(graph_.get_depth() - 1).size()));
  auto threads = std::vector<std::thread>();
  threads.reserve(threads_num);

  for (int i = 0; i < threads_num; i++) {
    threads.push_back(std::thread(worker));
  }

  while (finished_paths < finish_vertex_ids.size()) {
  }

  should_terminate = true;
  for (auto& thread : threads) {
    thread.join();
  }
  return paths;
}
}  // namespace uni_course_cpp
