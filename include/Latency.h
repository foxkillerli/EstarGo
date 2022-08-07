#ifndef _LATENCY_H
#define _LATENCY_H

#include <mutex>
#include <atomic>

class Latency {
 public:
  Latency() {
    ema_latency_ = 0;
  };
  ~Latency() {};
  long get_record_count() {
    return latency_count_.load(std::memory_order_acquire);
  }
  long get_latency_sum() {
    return latency_sum_.load(std::memory_order_acquire);
  }
  long get_max_latency() {
    return latency_max_.load(std::memory_order_acquire);
  }
  long get_average() {
    long count = get_record_count();
    long sum = get_latency_sum();
    return sum / (count+1);
  }
  float get_ema_latency() {
    std::lock_guard<std::mutex> lock(lantency_mutex_);
    return ema_latency_;
  }
  void Record(long latency) {
    latency_count_.fetch_add(1, std::memory_order_relaxed);
    latency_sum_.fetch_add(latency, std::memory_order_relaxed);
    if (get_max_latency() < latency) {
      latency_max_.store(latency);
    }
    std::lock_guard<std::mutex> lock(lantency_mutex_);
    ema_latency_ = 0.9 * ema_latency_ + 0.1 * latency;
  }
  void Clear() {
    std::lock_guard<std::mutex> lock(lantency_mutex_);
    latency_count_ = 0;
    latency_sum_ = 0;
    latency_max_ = 0;
    ema_latency_ = 0;
  }

 private:
  std::mutex lantency_mutex_;
  float ema_latency_;
  std::atomic<long> latency_count_{0};
  std::atomic<long> latency_sum_{0};
  std::atomic<long> latency_max_{0};
};

#endif