//
// Created by tssh on 24-2-23.
//

#include "ThreadLogger.h"

#include "fmt/color.h"
#include "fmt/compile.h"
void mhttplib::ThreadPrinter::Print(const String& output, int color) {
  std::lock_guard _lk(mutex_);
  printing_queue_.emplace_back(output, color);
  cv_.notify_all();
}
void mhttplib::ThreadPrinter::Println(const String& output, int color) {
  std::lock_guard _lk(mutex_);
  printing_queue_.emplace_back(output + "\n", color);
  cv_.notify_all();
}
mhttplib::ThreadPrinter::~ThreadPrinter() {
  stop_ = true;
  printing_thread_.join();
}
std::once_flag of;
std::unique_ptr<mhttplib::ThreadPrinter> mhttplib::ThreadPrinter::instance_;
mhttplib::ThreadPrinter& mhttplib::ThreadPrinter::Instance() {
  std::call_once(of, []() {
    if (!instance_) {
      instance_.reset(new ThreadPrinter);
    }
  });
  return *instance_;
}
mhttplib::ThreadPrinter::ThreadPrinter() {
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
        std::lock_guard _lk(mutex_);
        auto pc = fmt::fg(fmt::color::black);
        if (printing_queue_.front().color == Color_Red) {
          fmt::print(fmt::fg(fmt::color::red), "{}", printing_queue_.front().content);
        } else {
          fmt::print("{}", printing_queue_.front().content);
        }
        printing_queue_.pop_front();
      }
    }
  });
}