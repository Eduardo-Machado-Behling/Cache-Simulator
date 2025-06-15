#include <chrono> // For sleep
#include <condition_variable>
#include <functional>
#include <iostream>
#include <mutex>
#include <optional>
#include <queue>
#include <thread>

template <typename T> class ThreadSafeQueue {
public:
  void push(T value) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_queue.push(std::move(value));
    m_cond.notify_one();
  }

  template <typename... Args> void emplace(Args &&...args) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_queue.emplace(std::forward<Args>(args)...);
    m_cond.notify_one();
  }

  std::optional<T> wait_and_pop(const std::function<bool()> &stop_token) {
    std::unique_lock<std::mutex> lock(m_mutex);

    m_cond.wait(
        lock, [this, &stop_token] { return stop_token() || !m_queue.empty(); });

    if (stop_token() && m_queue.empty()) {
      return std::nullopt;
    }

    T value = std::move(m_queue.front());
    m_queue.pop();
    return value;
  }
  std::optional<T> try_pop() {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_queue.empty()) {
      return std::nullopt;
    }

    T value = std::move(m_queue.front());
    m_queue.pop();
    return value;
  }

  bool empty() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_queue.empty();
  }

private:
  mutable std::mutex m_mutex;
  std::condition_variable m_cond;
  std::queue<T> m_queue;
};