#ifndef EVAL_QUEUE_H_
#define EVAL_QUEUE_H_

#include <mutex>
#include <memory>
#include <functional>
#include <condition_variable>
#include <queue>

template <typename T>
class EvalQueue {
 private:
  mutable std::mutex mut; 
  std::queue<T> data_queue;
  std::condition_variable data_cond;
 public:
  EvalQueue() {}
  EvalQueue(EvalQueue const& other) {
    std::lock_guard<std::mutex> lk(other.mut);
    data_queue=other.data_queue;
  }
  void Push(T new_value) {
    std::lock_guard<std::mutex> lk(mut);
    data_queue.push(new_value);
    data_cond.notify_one();
  }
  void WaitAndPop(T& value) {
    std::unique_lock<std::mutex> lk(mut);
    data_cond.wait(lk,[this]{return !data_queue.empty();});
    value=data_queue.front();
    data_queue.pop();
  }
  std::shared_ptr<T> WaitAndPop() {
    std::unique_lock<std::mutex> lk(mut);
    data_cond.wait(lk,[this]{return !data_queue.empty();});
    std::shared_ptr<T> res(std::make_shared<T>(data_queue.front()));
    data_queue.pop();
    return res;
  }
  bool TryPop(T& value) {
    std::lock_guard<std::mutex> lk(mut);
    if(data_queue.empty())
     return false;
    value=data_queue.front();
    data_queue.pop();
    return true;
  }
  std::shared_ptr<T> TryPop() {
    std::lock_guard<std::mutex> lk(mut);
    if(data_queue.empty())
     return std::shared_ptr<T>();
    std::shared_ptr<T> res(std::make_shared<T>(data_queue.front()));
    data_queue.pop();
    return res;
  }
  bool Empty() const {
    std::lock_guard<std::mutex> lk(mut);
    return data_queue.empty();
  }
};

template <typename T>
class EvalCycleQueue {
 private:
  mutable std::mutex queue_mut_;
  std::vector<T> data_list_;
  int queue_size_;
  int head_;
  int tail_;
 public:
  EvalCycleQueue(int size=1) {
    head_ = tail_ = -1;
    SetSize(size);
  }
  EvalCycleQueue(EvalCycleQueue const& other) {
    std::lock_guard<std::mutex> lk(other.queue_mut_);
    data_list_=other.data_list_;
  }
  void SetSize(int size) {
    std::lock_guard<std::mutex> lk(queue_mut_);
    if (head_ < 0) {
      queue_size_ = size;
      data_list_.resize(size);
      head_ = tail_ = -1;
    }
  }
  bool TryPush(T new_value) {
    std::lock_guard<std::mutex> lk(queue_mut_);
    if (head_ < 0) {
      head_ = tail_ = 0;
      data_list_[head_] = new_value;
      // fprintf(stderr, "push1, head :%d, tail : %d\n", head_, tail_);
      return true;
    }
    int next_tail = tail_ + 1;
    if (next_tail >= queue_size_) {
      next_tail -= queue_size_;
    }
    if (next_tail == head_) {
      // fprintf(stderr, "push2, head :%d, tail : %d\n", head_, tail_);
      return false;
    }
    data_list_[next_tail] = new_value;
    tail_ = next_tail;
    // fprintf(stderr, "push3, head :%d, tail : %d\n", head_, tail_);
    return true;
  }
  bool TryPop(T& value) {
    std::lock_guard<std::mutex> lk(queue_mut_);
    if (head_ < 0) {
      // fprintf(stderr, "pop1, head :%d, tail : %d\n", head_, tail_);
      return false;
    }
    if (head_ == tail_) {
      value = data_list_[head_];
      head_ = tail_ = -1;
      // fprintf(stderr, "pop2, head :%d, tail : %d\n", head_, tail_);
      return true;
    }
    int next_head = head_ + 1;
    if (next_head >= queue_size_) {
      next_head -= queue_size_;
    }
    value = data_list_[head_];
    head_ = (head_ == tail_) ? -1 : next_head;
    // fprintf(stderr, "pop3, head :%d, tail : %d\n", head_, tail_);
    return true;
  }
  bool Empty() const {
    std::lock_guard<std::mutex> lk(queue_mut_);
    return head_ < 0;
  }
};
#endif