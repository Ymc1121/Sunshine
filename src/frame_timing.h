// Bounded, opt-in diagnostics. No video pixels, input events or credentials.
#pragma once

#include <atomic>
#include <chrono>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <mutex>
#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace frame_timing {
  inline int64_t now_ns() {
    return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
  }
  inline int64_t source_ns(const std::optional<std::chrono::steady_clock::time_point> &value) {
    return value ? std::chrono::duration_cast<std::chrono::nanoseconds>(value->time_since_epoch()).count() : 0;
  }
  struct context_t { uint64_t channel {}; int64_t frame {}; int64_t source {}; };
  inline thread_local context_t context;
  struct context_guard {
    context_t previous;
    context_guard(uint64_t channel, int64_t frame, int64_t source): previous(context) { context = {channel, frame, source}; }
    ~context_guard() { context = previous; }
  };
  struct row_t {
    int64_t begin, end;
    const char *stage;
    context_t context;
    int64_t value;
  };
  class recorder {
  public:
    explicit recorder(std::filesystem::path root): root_(std::move(root)) {}
    ~recorder() { finish(); }
    recorder(const recorder &) = delete;
    bool active() noexcept {
      auto now = now_ns();
      auto next = next_poll_.load(std::memory_order_relaxed);
      if (now >= next && next_poll_.compare_exchange_strong(next, now + 1000000000LL)) {
        try { poll(now); } catch (...) { enabled_ = false; }
      }
      return enabled_.load(std::memory_order_relaxed);
    }
    void add(row_t row) noexcept {
      if (!enabled_.load(std::memory_order_relaxed)) return;
      try {
        std::lock_guard lock(mutex_);
        if (!enabled_ || row.end > deadline_) return;
        if (rows_.size() >= maximum_rows) { overflow_ = true; return; }
        rows_.push_back(row);
      } catch (...) { enabled_ = false; }
    }
    static bool valid_request(const std::string &tag, int seconds) {
      if (tag.empty() || tag.size() > 32 || seconds < 5 || seconds > 120) return false;
      for (auto ch : tag) if (!((ch >= 'a' && ch <= 'z') || (ch >= '0' && ch <= '9') || ch == '-')) return false;
      return true;
    }
    void finish() noexcept {
      try {
        std::lock_guard lock(mutex_);
        finish_locked();
      } catch (...) { enabled_ = false; }
    }
  private:
    void poll(int64_t now) {
      std::lock_guard lock(mutex_);
      if (enabled_ && now >= deadline_) finish_locked();
      if (enabled_) return;
      std::ifstream request(root_ / "frame-timing-v1.request");
      std::string tag, extra;
      int seconds = 0;
      if (!(request >> tag >> seconds) || (request >> extra) || !valid_request(tag, seconds) || tag == consumed_tag_) return;
      consumed_tag_ = tag;
      output_ = root_ / ("frame-timing-" + tag + ".csv");
      if (std::filesystem::exists(output_)) return;
      rows_.clear();
      rows_.reserve(maximum_rows);
      overflow_ = false;
      started_ = now;
      utc_started_ = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
      deadline_ = now + seconds * 1000000000LL;
      enabled_ = true;
    }
    void finish_locked() {
      if (!enabled_.exchange(false)) return;
      // One write after capture ends; no file I/O for individual frames.
      std::ofstream out(output_, std::ios::out | std::ios::trunc);
      if (!out) return;
      out << "# schema=gokuai.frame-timing/v1,steady_start_ns=" << started_ << ",utc_start_ns=" << utc_started_
          << ",requested_end_ns=" << deadline_ << ",window_complete=" << (now_ns() >= deadline_ ? 1 : 0)
          << ",overflow=" << (overflow_ ? 1 : 0) << ",rows=" << rows_.size() << '\n';
      out << "begin_ns,end_ns,stage,channel,frame,source_ns,value\n";
      for (const auto &r : rows_) out << r.begin << ',' << r.end << ',' << r.stage << ',' << r.context.channel << ',' << r.context.frame << ',' << r.context.source << ',' << r.value << '\n';
      rows_.clear();
    }
    static constexpr size_t maximum_rows = 200000;
    std::filesystem::path root_, output_;
    std::string consumed_tag_;
    std::atomic<bool> enabled_ {false};
    std::atomic<int64_t> next_poll_ {0};
    std::mutex mutex_;
    int64_t started_ {}, utc_started_ {}, deadline_ {};
    bool overflow_ {};
    std::vector<row_t> rows_;
  };
  inline recorder &instance() {
    static recorder value("C:/ProgramData/GouKuai/Diagnostics/frame-timing");
    return value;
  }
  class span {
  public:
    explicit span(const char *stage, int64_t value = 0): stage_(stage), value_(value), context_(context) {
      if (instance().active()) begin_ = now_ns();
    }
    ~span() { stop(); }
    void source(int64_t timestamp) noexcept { context_.source = timestamp; }
    void stop() noexcept {
      if (begin_) { instance().add({begin_, now_ns(), stage_, context_, value_}); begin_ = 0; }
    }
  private:
    const char *stage_;
    int64_t value_, begin_ {};
    context_t context_;
  };
}
