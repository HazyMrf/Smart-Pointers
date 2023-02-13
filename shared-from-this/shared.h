#pragma once

#include "sw_fwd.h"  // Forward declaration
#include "weak.h"

#include <cstddef>  // std::nullptr_t

// https://en.cppreference.com/w/cpp/memory/shared_ptr
template <typename T>
class SharedPtr {
public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Constructors

    SharedPtr() {
    }
    SharedPtr(std::nullptr_t) {
    }
    // default pointers are already nullptr

    explicit SharedPtr(T* ptr) : ptr_(ptr), block_(new ControlBlockPointer<T>(ptr)) {
        block_->IncStrongRef();
        if constexpr (std::is_convertible_v<T*, EnableSharedFromThisBase*>) {
            InitWeakThis(ptr);
        }
    }

    template <typename Up>  // require Up* to be convertible to T*
    explicit SharedPtr(Up* ptr) : ptr_(ptr), block_(new ControlBlockPointer<Up>(ptr)) {
        block_->IncStrongRef();
        if constexpr (std::is_convertible_v<Up*, EnableSharedFromThisBase*>) {
            InitWeakThis(ptr);
        }
    }

    SharedPtr(const SharedPtr& other) : ptr_(other.ptr_), block_(other.block_) {
        if (block_) {
            block_->IncStrongRef();
        }
    }

    template <typename Up>
    SharedPtr(const SharedPtr<Up>& other) : ptr_(other.ptr_), block_(other.block_) {
        if (block_) {
            block_->IncStrongRef();
        }
    }

    SharedPtr(SharedPtr&& other) {
        Dispose();
        ptr_ = other.ptr_;
        block_ = other.block_;

        other.ptr_ = nullptr;
        other.block_ = nullptr;
    }

    template <typename Up>
    SharedPtr(SharedPtr<Up>&& other) {
        Dispose();
        ptr_ = other.ptr_;
        block_ = other.block_;

        other.ptr_ = nullptr;
        other.block_ = nullptr;
    }

    // Aliasing constructor
    // #8 from https://en.cppreference.com/w/cpp/memory/shared_ptr/shared_ptr
    template <typename Y>
    SharedPtr(const SharedPtr<Y>& other, T* ptr) : ptr_(ptr), block_(other.block_) {
        block_->IncStrongRef();
    }

    // Promote `WeakPtr`
    // #11 from https://en.cppreference.com/w/cpp/memory/shared_ptr/shared_ptr
    explicit SharedPtr(const WeakPtr<T>& other) : block_(other.block_) {
        if (other.UseCount() == 0) {
            throw BadWeakPtr();
        }
        if (other.block_) {
            ptr_ = other.ptr_;
            block_ = other.block_;
            block_->IncStrongRef();
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // `operator=`-s

    SharedPtr& operator=(const SharedPtr& other) {
        Dispose();
        if (other.block_) {
            other.block_->IncStrongRef();
        }
        ptr_ = other.ptr_;
        block_ = other.block_;
        return *this;
    }
    template <typename Up>
    SharedPtr& operator=(const SharedPtr<Up>& other) {
        Dispose();
        if (other.block_) {
            other.block_->IncStrongRef();
        }
        ptr_ = other.ptr_;
        block_ = other.block_;
        return *this;
    }

    SharedPtr& operator=(SharedPtr&& other) {
        Dispose();
        ptr_ = other.ptr_;
        block_ = other.block_;

        other.ptr_ = nullptr;
        other.block_ = nullptr;
        return *this;
    }
    template <typename Up>
    SharedPtr& operator=(SharedPtr<Up>&& other) {
        Dispose();
        ptr_ = other.ptr_;
        block_ = other.block_;

        other.ptr_ = nullptr;
        other.block_ = nullptr;
        return *this;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Destructor

    ~SharedPtr() {
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

    template <typename Up>
    void Reset(Up* ptr) {  // to be precise we need to be sure that Up* is convertible to T*a
        if (ptr == nullptr) {
            Reset();
        } else {
            Dispose();
            ptr_ = ptr;
            block_ = new ControlBlockPointer<Up>(ptr);
            block_->IncStrongRef();
        }
    }
    void Swap(SharedPtr& other) {
        std::swap(ptr_, other.ptr_);
        std::swap(block_, other.block_);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
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
        if (block_) {
            return block_->GetStrongRefCount();
        }
        return 0;
    }
    explicit operator bool() const {
        if (ptr_ == nullptr) {
            return false;
        }
        return true;
    }

private:
    T* ptr_ = nullptr;
    ControlBlockBase* block_ = nullptr;

    void Dispose() {
        if (block_) {
            block_->IncWeakRef();
            block_->DecStrongRef();
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
    void InitWeakThis(EnableSharedFromThis<Y>* e) {
        e->weak_this_ = *this;
        //        e->weak_this_.ptr_ = ptr_;
        //        e->weak_this_.block_ = block_;
        //        block_->IncWeakRef();
    }

    template <typename Y>
    friend class SharedPtr;  // Now SharedPtr<Y> is a friend of SharedPtr

    template <typename Y>
    friend class WeakPtr;  // for ctor from weak ptr, which is not realized here

    template <typename Y>
    friend class EnableSharedFromThis;

    template <typename P, typename... Args>
    friend SharedPtr<P> MakeShared(Args&&... args);
};

template <typename T, typename U>
inline bool operator==(const SharedPtr<T>& left, const SharedPtr<U>& right) {
    return left.Get() == right.Get();
}

// Allocate memory only once
template <typename T, typename... Args>
SharedPtr<T> MakeShared(Args&&... args) {
    SharedPtr<T> s;
    ControlBlockHolder<T>* block = new ControlBlockHolder<T>(std::forward<Args>(args)...);
    // to have ->GetPointer() func we need to do =
    s.ptr_ = block->GetPointer();
    s.block_ = block;
    s.block_->IncStrongRef();
    if constexpr (std::is_convertible_v<T*, EnableSharedFromThisBase*>) {
        s.InitWeakThis(s.ptr_);
    }
    return s;
};

// Look for usage examples in tests
template <typename T>
class EnableSharedFromThis : public EnableSharedFromThisBase {
public:
    SharedPtr<T> SharedFromThis() {
        return SharedPtr<T>(this->weak_this_);
    }
    SharedPtr<const T> SharedFromThis() const {
        return SharedPtr<const T>(this->weak_this_);
    }

    WeakPtr<T> WeakFromThis() noexcept {
        return this->weak_this_;
    }
    WeakPtr<const T> WeakFromThis() const noexcept {
        return this->weak_this_;
    }

public:  // PRIVATE
    WeakPtr<T> weak_this_;

    template <typename Y>
    friend class WeakPtr;

    template <typename Y>
    friend class SharedPtr;
};