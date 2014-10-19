#ifndef OBSERVABLE_H
#define OBSERVABLE_H

#include <set>

template<typename T>
class observable;

template<typename T>
class IObserver {
  public:
    virtual ~IObserver(){}

  public:
    virtual void on_change(observable<T> const*, T was, T is) =0;
};


template<typename T>
class observable {
  public:
    observable(T v): _value(v){}

  public:
    virtual ~observable(){}
    observable(observable<T> const&) = delete;
    observable<T>& operator= (observable<T> const&) = delete;

  public:
    void add_observer(IObserver<T>* observer) {
      _observers.insert(observer);
    }

    void remove_observer(IObserver<T>* observer) {
      _observers.erase(observer);
    }

  public:
    #define BINARY_OPERATOR(op, arg) observable<T>& operator op (arg x) { T was = _value; _value op x; notify(was, _value); return *this; }
    BINARY_OPERATOR( =, T const&);
    BINARY_OPERATOR(|=, const int);
    BINARY_OPERATOR(&=, const int);
    BINARY_OPERATOR(^=, const int);
    BINARY_OPERATOR(-=, const int);
    #undef BINARY_OPERATOR

    observable<T>& operator++() {
      T was = _value;
      ++_value;
      notify(was, _value);
      return *this;
    }

    T operator++(int) {
      T was = _value;
      ++_value;
      notify(was, _value);
      return was;
    }

    observable<T>& operator--() {
      T was = _value;
      --_value;
      notify(was, _value);
      return *this;
    }

    T operator--(int) {
      T was = _value;
      --_value;
      notify(was, _value);
      return was;
    }

    operator T() const {
      return _value;
    }

  private:
    void notify(T was, T is) {
      for (auto const& p : _observers) {
        p->on_change(this, was, is);
      }
    }

  private:
    T _value;
    std::set<IObserver<T>*> _observers;

};


#endif