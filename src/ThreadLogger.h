//
// Created by tssh on 24-2-23.
//

#ifndef THREADLOGGER_H
#define THREADLOGGER_H
#include <toy/type.h>

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <sstream>
#include <thread>
#include <utility>
namespace mhttplib {

class ThreadPrinter {
public:
  enum Color {
    Color_Default,
    Color_Red,
  };
  void Print(const String& output, int color = Color_Default);
  void Println(const String& output, int color = Color_Default);
  template <typename T>
  ThreadPrinter& operator<<(T output) {
    std::stringstream str;
    str << output;
    printing_queue_.emplace_back(str.str(), Color_Default);
    return *this;
  }
  ~ThreadPrinter();
  static ThreadPrinter& Instance();

private:
  static std::unique_ptr<ThreadPrinter> instance_;
  struct Output {
    String content;
    int color;
    Output(String c, int clr) : content(std::move(c)), color(clr) {
    }
  };
  ThreadPrinter();
  std::atomic_bool stop_ = false;
  std::condition_variable cv_;
  std::mutex cv_mutex_;
  std::mutex mutex_;
  std::thread printing_thread_;
  VecDeque<Output> printing_queue_;
};
}  // namespace mhttplib
#define mhlPrinter mhttplib::ThreadPrinter::Instance()

#endif  // THREADLOGGER_H
