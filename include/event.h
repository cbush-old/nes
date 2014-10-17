#ifndef EVENT_H
#define EVENT_H

#include <memory>
#include <semaphore.h>

class Event {
  public:
    virtual ~Event() =0;

  public:
    std::shared_ptr<void> _payload;

};

Event::~Event(){}

#endif
