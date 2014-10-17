#include "async_component.h"
#include <iostream>
#include <chrono>

AsyncComponent::~AsyncComponent() {
  stop();
}

AsyncComponent::AsyncComponent() {}

void AsyncComponent::start() {
  pre_start();
  _thread = std::thread { [&] {
    while (!_done) {
      auto t = std::chrono::high_resolution_clock::now();
      {
        std::unique_lock<std::mutex> lock(_mutex);

        if (_sync) {
          _nsync = true;
          _sync->wait(lock);
          _nsync = false;
          _sync = nullptr;
        }

        while (!_queue.empty()) {
          on_event(*_queue.front());
          _queue.pop();
        }

        tick();
      }

      auto dt = std::chrono::high_resolution_clock::now() - t;
      //std::this_thread::sleep_for(std::chrono::milliseconds(17) - (dt > std::chrono::milliseconds(17)? dt : std::chrono::milliseconds(17)));

    }
  }};

}

void AsyncComponent::stop() {
  _done = true;
  if (_thread.joinable()) {
    _thread.join();
  }
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


