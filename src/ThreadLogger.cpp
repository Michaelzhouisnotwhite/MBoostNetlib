//
// Created by tssh on 24-2-23.
//

#include "ThreadLogger.h"

#include "fmt/color.h"
void mnet::ThreadPrinter::Print(const String& output, int color) {
  std::lock_guard _lk(mutex_);
  printing_queue_.emplace_back(output, color);
  cv_.notify_all();
}
void mnet::ThreadPrinter::Println(const String& output, int color) {
  std::lock_guard _lk(mutex_);
  printing_queue_.emplace_back(output + "\n", color);
  cv_.notify_all();
}
mnet::ThreadPrinter::~ThreadPrinter() {
  stop_ = true;
  printing_thread_.join();
}
std::once_flag of;
std::unique_ptr<mnet::ThreadPrinter> mnet::ThreadPrinter::instance_;
mnet::ThreadPrinter& mnet::ThreadPrinter::Instance() {
  std::call_once(of, []() {
    if (!instance_) {
      instance_.reset(new ThreadPrinter);
    }
  });
  return *instance_;
}
mnet::ThreadPrinter::ThreadPrinter() {
  printing_thread_ = std::thread([this]() {
    while (true) {
      std::unique_lock cv_lk(cv_mutex_);
      cv_.wait(cv_lk, [this]() {
        return !printing_queue_.empty() || stop_;
      });
      if (stop_) {
        return;
      }
      while (!printing_queue_.empty()) {
        mutex_.lock();
        auto outputs = printing_queue_;
        printing_queue_.clear();
        mutex_.unlock();
        for (const auto& output : outputs) {
          if (output.color == Color_Red) {
            fmt::print(fmt::fg(fmt::color::red), "{}", output.content);
          } else {
            fmt::print("{}", output.content);
          }
        }
      }
    }
  });
}