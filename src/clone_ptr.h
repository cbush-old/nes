#pragma once

#include <memory>

template<typename T>
class ClonePtr
{
public:
    template<typename ConcreteType>
    explicit ClonePtr(ConcreteType *p) // takes ownership
        : _model(new Model<ConcreteType>(p))
    {}
    
    ClonePtr &operator=(ClonePtr const &) = delete;
    ClonePtr &operator=(ClonePtr &&) = default;
    ClonePtr(ClonePtr const &) = delete;
    ClonePtr(ClonePtr &&) = default;

    std::unique_ptr<T> clone()
    {
        if (!_model)
        {
            return {};
        }
        return {_model->clone()};
    }

    T &operator*() const
    {
        assert(_model);
        return _model->_p.operator*();
    }

    T *operator->() const
    {
        return _model->p.operator->();
    }

private:
    struct Concept
    {
        Concept(T *p): p(p) {}
        virtual ~Concept() = default;
        virtual T *clone() const = 0;
        std::unique_ptr<T> p;
    };

    template<typename ConcreteType>
    struct Model : Concept
    {
        Model(ConcreteType *p)
            : Concept(p)
        {}

        virtual ConcreteType *clone() const override
        {
            return new ConcreteType(static_cast<ConcreteType const &>(*this->p));
        }
    };

    std::unique_ptr<Concept> _model;
};
