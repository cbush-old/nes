#pragma once

#include <cassert>
#include <cstdint>
#include <cstdlib>

template<typename BaseType, size_t MaxSize = sizeof(BaseType) * 4>
class InlinePolymorph
{
public:
    InlinePolymorph()
        : _object(nullptr)
    {}

    /*!
     \brief Construct by copy of derived type
    */
    template<typename DerivedType>
    InlinePolymorph(DerivedType &&instance)
        : _object(nullptr)
    {
        static_assert(std::is_base_of<BaseType, DerivedType>::value, "DerivedType must derive from Base!");
        emplace<DerivedType>(std::forward<DerivedType>(instance));
    }
    
    ~InlinePolymorph()
    {
        clear();
    }

    InlinePolymorph(InlinePolymorph const &other)
        : _object(nullptr)
    {
        other.copy_into(*this);
    }

    InlinePolymorph &operator=(InlinePolymorph const &other)
    {
        clear();
        other.copy_into(*this);
        return *this;
    }

    InlinePolymorph(InlinePolymorph &&other)
        : _object(nullptr)
    {
        other.move_into(*this);
    }

    InlinePolymorph &operator=(InlinePolymorph &&other)
    {
        clear();
        other.move_into(*this);
        return *this;
    }

    void clear()
    {
        if (_object)
        {
            _object->~BaseType();
            _object = nullptr;
            //concept()->~Concept();
            _concept_space = 0;
        }
    }

    /*!
     \return The pointer to the object.
    */
    template<typename DerivedType, typename... Tp>
    DerivedType *emplace(Tp &&... args)
    {
        static_assert(sizeof(DerivedType) <= MaxSize, "DerivedType size exceeds max capacity");
        static_assert(std::is_base_of<BaseType, DerivedType>::value, "DerivedType must derive from BaseType");

        clear();

        static_assert(sizeof(Model<DerivedType>) <= sizeof(uint64_t), "All assumptions are false!");
        new(&_concept_space) Model<DerivedType>();

        DerivedType *p = new(_data) DerivedType(std::forward<Tp>(args)...);
        _object = static_cast<BaseType *>(p);
        return p;
    }

    BaseType const &operator*() const
    {
        assert(_object);
        return *_object;
    }

    BaseType &operator*()
    {
        assert(_object);
        return *_object;
    }

    BaseType const *operator->() const
    {
        assert(_object);
        return _object;
    }

    BaseType *operator->()
    {
        assert(_object);
        return _object;
    }
    
    // return non-const ptr from const member function: following STL
    // const-correctness as in shared_ptr::get(), unique_ptr::get()
    BaseType *get() const
    {
        return _object;
    }
    
    operator bool() const
    {
        return _object != nullptr;
    }

    bool operator!() const
    {
        return _object == nullptr;
    }

private:
    struct Concept
    {
        virtual ~Concept() = default;
        virtual void copy_into(InlinePolymorph const &from, InlinePolymorph &to) const = 0;
        virtual void move_into(InlinePolymorph &&from, InlinePolymorph &to) const = 0;
    };
    
    template<typename DerivedType>
    struct Model : Concept
    {
        virtual void copy_into(InlinePolymorph const &from, InlinePolymorph &to) const override
        {
            to.emplace<DerivedType>(*static_cast<DerivedType const *>(from._object));
        }
        
        virtual void move_into(InlinePolymorph &&from, InlinePolymorph &to) const override
        {
            to.emplace<DerivedType>(std::move(*static_cast<DerivedType *>(from._object)));
        }
    };

    Concept const *concept() const
    {
        return _object ? reinterpret_cast<Concept const *>(&_concept_space) : nullptr;
    }

    void copy_into(InlinePolymorph &other) const
    {
        if (concept())
        {
            concept()->copy_into(*this, other);
        }
    }
    
    void move_into(InlinePolymorph &other)
    {
        if (concept())
        {
            concept()->move_into(std::move(*this), other);
        }
    }

    uint64_t _concept_space;
    BaseType *_object;
    uint8_t _data[MaxSize];
};
