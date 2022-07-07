#pragma once

#include <vector>
#include "game.hpp"
#include "graph_generator.hpp"
#include "graph_traverser.hpp"

namespace uni_course_cpp {
class GameGenerator {
 public:
  GameGenerator(GraphGenerator::Params&& params) : params_(params) {}

  Game generate() const;

 private:
  GraphGenerator::Params params_;
};
}  // namespace uni_course_cpp
