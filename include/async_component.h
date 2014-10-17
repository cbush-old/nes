#ifndef ASYNC_COMPONENT_H
#define ASYNC_COMPONENT_H

#include "ievent.h"

#include <mutex>
#include <thread>
#include <queue>
#include <condition_variable>

class AsyncComponent {
  public:
    AsyncComponent();
    virtual ~AsyncComponent();

  public:
    void start();
    void stop();
    void post(IEvent const&);

  public:
    /**
     * @brief have this component's execution thread wait on the given std::condition_variable for synchronization purposes
     * @param cond the condition variable that will be waited on in the component's execution thread
     **/
    void sync(std::condition_variable& cond);

    /**
     * @brief check whether a call to sync() has taken effect
     * @return true if the thread waiting on a condition variable as a result of a call to sync()
     **/
    bool is_syncing() const;

  protected:
    virtual uint32_t CLOCK_FREQUENCY_HZ() const =0;

  protected:
    virtual void pre_start(){}
    virtual void on_event(IEvent const&) =0;
    virtual void tick() =0;

  protected:
    std::mutex _mutex;

  private:
    bool _done { false };
    bool _nsync { false };
    std::condition_variable *_sync { nullptr };
    std::thread _thread;
    std::queue<std::shared_ptr<const IEvent>> _queue;

};

#endif