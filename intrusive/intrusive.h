#pragma once

#include <cstddef>  // for std::nullptr_t
#include <utility>  // for std::exchange / std::swap

class SimpleCounter {
public:
    SimpleCounter() {
    }

    SimpleCounter(const SimpleCounter& other) : SimpleCounter() {
    }

    SimpleCounter& operator=(const SimpleCounter& other) {
        return *this;
    }

    size_t IncRef() {
        return ++count_;
    }
    size_t DecRef() {
        return --count_;
    }
    size_t RefCount() const {
        return count_;
    }

private:
    size_t count_ = 0;
};

struct DefaultDelete {
    template <typename T>
    static void Destroy(T* object) {
        delete object;
    }
};

template <typename Derived, typename Counter, typename Deleter>
class RefCounted {
public:
    // Increase reference counter.
    void IncRef() {
        counter_.IncRef();
    }
    // Decrease reference counter.
    // Destroy object using Deleter when the last instance dies.
    void DecRef() {
        if (!counter_.DecRef()) {
            Deleter().Destroy(static_cast<Derived*>(this));
        }
    }

    // Get current counter value (the number of strong references).
    size_t RefCount() const {
        return counter_.RefCount();
    }

private:
    Counter counter_;
};

template <typename Derived, typename D = DefaultDelete>
using SimpleRefCounted = RefCounted<Derived, SimpleCounter, D>;

template <typename T>
class IntrusivePtr {
    template <typename Y>
    friend class IntrusivePtr;

public:
    // Constructors
    IntrusivePtr() {
    }
    IntrusivePtr(std::nullptr_t) {
    }
    IntrusivePtr(T* ptr) : ptr_(ptr) {
        Inc();
    }

    template <typename Y>
    IntrusivePtr(const IntrusivePtr<Y>& other) : ptr_(other.ptr_) {
        Inc();
    }

    template <typename Y>
    IntrusivePtr(IntrusivePtr<Y>&& other) : ptr_(other.ptr_) {
        other.ptr_ = nullptr;  // if other == nullptr, then no Inc and Dec, if not that Inc+Dec=0
    }

    IntrusivePtr(const IntrusivePtr& other) : ptr_(other.ptr_) {
        Inc();
    }
    IntrusivePtr(IntrusivePtr&& other) : ptr_(other.ptr_) {
        other.ptr_ = nullptr;
    }

    // `operator=`-s
    IntrusivePtr& operator=(const IntrusivePtr& other) {
        other.Inc();
        Dec();
        ptr_ = other.ptr_;
        return *this;
    }
    IntrusivePtr& operator=(IntrusivePtr&& other) {
        if (ptr_ != other.ptr_) {
            Dec();
            ptr_ = other.ptr_;
            other.ptr_ = nullptr;
        }
        return *this;
    }

    // Destructor
    ~IntrusivePtr() {
        Dec();
    }

    // Modifiers
    void Reset() {
        Dec();
        ptr_ = nullptr;
    }
    void Reset(T* ptr) {
        if (ptr_ != ptr) {
            Dec();
            ptr_ = ptr;
            Inc();
        }
    }
    void Swap(IntrusivePtr& other) {
        std::swap(ptr_, other.ptr_);
    }

    // Observers
    T* Get() const {
        return ptr_;
    }
    T& operator*() const {
        return *ptr_;
    }
    T* operator->() const {
        return ptr_;
    }
    size_t UseCount() const {
        if (ptr_) {
            return ptr_->RefCount();
        }
        return 0;
    }
    explicit operator bool() const {
        if (ptr_) {
            return true;
        }
        return false;
    }

private:
    T* ptr_ = nullptr;

    void Inc() const {
        if (ptr_) {
            ptr_->IncRef();
        }
    }

    void Dec() const {
        if (ptr_) {
            ptr_->DecRef();
        }
    }
};

template <typename T, typename... Args>
IntrusivePtr<T> MakeIntrusive(Args&&... args) {
    return IntrusivePtr<T>(new T(std::forward<Args>(args)...));  // first create then pass
}
