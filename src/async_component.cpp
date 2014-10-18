#include "async_component.h"
#include <iostream>
#include <chrono>

AsyncComponent::~AsyncComponent() {
  stop();
}

AsyncComponent::AsyncComponent(IAsyncComponentMonitor& monitor)
  : _monitor(monitor)
  {}

void AsyncComponent::start() {
  pre_start();
  _thread = std::thread { [&] {
    while (!_done) {
      //auto t = std::chrono::high_resolution_clock::now();
      //for (size_t i = 0; i < ; ++i)
      {
        std::unique_lock<std::mutex> lock(_mutex);

        while (!_queue.empty()) {
          on_event(*_queue.front());
          _queue.pop();
        }

        tick();

      }

      if (++_ticks == CLOCK_FREQUENCY_HZ()) {
        _monitor.signal(this);
        _semaphore.wait();
        std::cout << this << " resume " << CLOCK_FREQUENCY_HZ() << std::endl;
        _ticks = 0;
      }

      //auto dt = std::chrono::high_resolution_clock::now() - t;
      //std::this_thread::sleep_for(std::chrono::milliseconds(17) - (dt > std::chrono::milliseconds(17)? dt : std::chrono::milliseconds(17)));

    }
  }};

}

void AsyncComponent::stop() {
  _done = true;
  _semaphore.signal();
  if (_thread.joinable()) {
    _thread.join();
  }
}

void AsyncComponent::signal() {
  _semaphore.signal();
}


void AsyncComponent::post(IEvent const& event) {
  std::lock_guard<std::mutex> lock(_mutex);
  _queue.emplace(&event);
}

void AsyncComponent::sync(std::condition_variable& cond) {
  std::lock_guard<std::mutex> lock(_mutex);
  if (_sync) {
    throw std::runtime_error("double call to AsyncComponent::sync");
  }
  _sync = &cond;
}

bool AsyncComponent::is_syncing() const {
  return _nsync;
}


