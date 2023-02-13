#pragma once

#include "sw_fwd.h"  // Forward declaration

// https://en.cppreference.com/w/cpp/memory/weak_ptr
template <typename T>
class WeakPtr {
public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Constructors

    WeakPtr() {
    }

    WeakPtr(const WeakPtr& other) : ptr_(other.ptr_), block_(other.block_) {
        if (block_) {
            block_->IncWeakRef();
        }
    }

    template <typename Up>
    WeakPtr(const WeakPtr<Up>& other) : ptr_(other.ptr_), block_(other.block_) {
        if (block_) {
            block_->IncWeakRef();
        }
    }

    WeakPtr(WeakPtr&& other) {
        Dispose();
        ptr_ = other.ptr_;
        block_ = other.block_;

        other.ptr_ = nullptr;
        other.block_ = nullptr;
    }

    template <typename Up>
    WeakPtr(WeakPtr<Up>&& other) {
        Dispose();
        ptr_ = other.ptr_;
        block_ = other.block_;

        other.ptr_ = nullptr;
        other.block_ = nullptr;
    }

    // Demote `SharedPtr`
    // #2 from https://en.cppreference.com/w/cpp/memory/weak_ptr/weak_ptr
    WeakPtr(const SharedPtr<T>& other) : block_(other.block_) {
        if (block_) {
            ptr_ = other.ptr_;
            block_->IncWeakRef();
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // `operator=`-s

    WeakPtr& operator=(const WeakPtr& other) {
        Dispose();
        if (other.block_) {
            other.block_->IncWeakRef();
        }
        ptr_ = other.ptr_;
        block_ = other.block_;
        return *this;
    }

    template <typename Up>
    WeakPtr& operator=(const WeakPtr<Up>& other) {
        Dispose();
        if (other.block_) {
            other.block_->IncWeakRef();
        }
        ptr_ = other.ptr_;
        block_ = other.block_;
        return *this;
    }

    WeakPtr& operator=(WeakPtr&& other) {
        Dispose();
        ptr_ = other.ptr_;
        block_ = other.block_;

        other.ptr_ = nullptr;
        other.block_ = nullptr;
        return *this;
    }

    template <typename Up>
    WeakPtr& operator=(WeakPtr<Up>&& other) {
        Dispose();
        ptr_ = other.ptr_;
        block_ = other.block_;

        other.ptr_ = nullptr;
        other.block_ = nullptr;
        return *this;
    }

    template <typename Y>
    WeakPtr& operator=(const SharedPtr<Y>& other) {
        Dispose();
        if (other.block_) {
            other.block_->IncWeakRef();
        }
        ptr_ = other.ptr_;
        block_ = other.block_;
        return *this;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Destructor

    ~WeakPtr() {
        Dispose();
        ptr_ = nullptr;
        block_ = nullptr;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Modifiers

    void Reset() {
        Dispose();
        ptr_ = nullptr;
        block_ = nullptr;
    }
    void Swap(WeakPtr& other) {
        std::swap(ptr_, other.ptr_);
        std::swap(block_, other.block_);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Observers

    size_t UseCount() const {
        if (block_) {
            return block_->GetStrongRefCount();
        }
        return 0;  // *this is empty
    }
    bool Expired() const {
        return (UseCount() == 0);
    }
    SharedPtr<T> Lock() const {
        if (Expired()) {
            return SharedPtr<T>();
        }
        return SharedPtr<T>(*this);
    }

private:
    T* ptr_ = nullptr;
    ControlBlockBase* block_ = nullptr;

    void Dispose() {  // this method is actually deleting the block
        if (block_) {
            block_->DecWeakRef();
        }
        if (block_) {
            if (block_->GetStrongRefCount() == 0 &&
                block_->GetWeakRefCount() == 0) {  // may be "delete nullptr"
                delete block_;
                block_ = nullptr;
            }
        }
    }

    template <typename Y>
    friend class SharedPtr;

    template <typename Y>
    friend class WeakPtr;

    template <typename Y>
    friend class EnableSharedFromThis;
};
