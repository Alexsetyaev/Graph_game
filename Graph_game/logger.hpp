#pragma once

#include <fstream>
#include <mutex>
#include <string>

namespace uni_course_cpp {

class Logger {
 public:
  static Logger& get_logger() {
    static Logger logger;
    return logger;
  }
  ~Logger();

  void log(const std::string& string);

 private:
  std::mutex mutex_;
  Logger();
  Logger(const Logger&) = delete;
  Logger& operator=(const Logger&) = delete;
  std::ofstream file_;
  Logger(Logger&&) = delete;
  Logger& operator=(Logger&&) = delete;
};
}  // namespace uni_course_cpp
