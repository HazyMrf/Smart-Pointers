#pragma once

#include "sw_fwd.h"  // Forward declaration

#include <cstddef>  // std::nullptr_t

class ControlBlockBase {
public:
    size_t GetStrongRefCount() const {
        return strong_ref_count_;
    }
    size_t GetWeakRefCount() const {
        return weak_ref_count_;
    }

    virtual void IncStrongRef() = 0;

    virtual void DecStrongRef() = 0;

    virtual void IncWeakRef() = 0;

    virtual void DecWeakRef() = 0;

    virtual ~ControlBlockBase() = default;

protected:
    size_t strong_ref_count_ = 0;
    size_t weak_ref_count_ = 0;
};

template <typename Y>
class ControlBlockPointer : public ControlBlockBase {
public:
    ControlBlockPointer(Y* ptr) : ptr_(ptr) {
    }

    void IncStrongRef() override {
        ++strong_ref_count_;
    }

    void DecStrongRef() override {
        --strong_ref_count_;
        if (strong_ref_count_ == 0) {
            delete ptr_;
            ptr_ = nullptr;
        }
    }

    void IncWeakRef() override {
        ++weak_ref_count_;
    }

    void DecWeakRef() override {
        --weak_ref_count_;
    }

    ~ControlBlockPointer() override {
        if (ptr_) {
            delete ptr_;
        }
    }

private:
    Y* ptr_;
};

template <typename Y>
class ControlBlockHolder : public ControlBlockBase {
public:
    template <typename... Args>
    ControlBlockHolder(Args&&... args) {
        new (GetPointer()) Y(std::forward<Args>(args)...);
    }

    Y* GetPointer() {
        return reinterpret_cast<Y*>(&storage_);
    }

    void IncStrongRef() {
        ++strong_ref_count_;
    }

    void DecStrongRef() override {
        --strong_ref_count_;
        if (strong_ref_count_ == 0) {
            GetPointer()->~Y();
        }
    }

    void IncWeakRef() override {
        ++weak_ref_count_;
    }

    void DecWeakRef() override {
        --weak_ref_count_;
    }

    ~ControlBlockHolder() override {
        GetPointer()->~Y();
    }

private:
    alignas(Y) char storage_[sizeof(Y)];  // inner storage of Y
};

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
    }

    template <typename Up>  // require Up* to be convertible to T*
    explicit SharedPtr(Up* ptr) : ptr_(ptr), block_(new ControlBlockPointer<Up>(ptr)) {
        block_->IncStrongRef();
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
        DecBlockRef();
        ptr_ = other.ptr_;
        block_ = other.block_;

        other.ptr_ = nullptr;
        other.block_ = nullptr;
    }

    template <typename Up>
    SharedPtr(SharedPtr<Up>&& other) {
        DecBlockRef();
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
    //    explicit SharedPtr(const WeakPtr<T>& other);

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // `operator=`-s

    SharedPtr& operator=(const SharedPtr& other) {
        DecBlockRef();
        if (other.block_) {
            other.block_->IncStrongRef();
        }
        ptr_ = other.ptr_;
        block_ = other.block_;
        return *this;
    }
    template <typename Up>
    SharedPtr& operator=(const SharedPtr<Up>& other) {
        DecBlockRef();
        if (other.block_) {
            other.block_->IncStrongRef();
        }
        ptr_ = other.ptr_;
        block_ = other.block_;
        return *this;
    }

    SharedPtr& operator=(SharedPtr&& other) {
        DecBlockRef();
        ptr_ = other.ptr_;
        block_ = other.block_;

        other.ptr_ = nullptr;
        other.block_ = nullptr;
        return *this;
    }
    template <typename Up>
    SharedPtr& operator=(SharedPtr<Up>&& other) {
        DecBlockRef();
        ptr_ = other.ptr_;
        block_ = other.block_;

        other.ptr_ = nullptr;
        other.block_ = nullptr;
        return *this;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Destructor

    ~SharedPtr() {
        DecBlockRef();
        ptr_ = nullptr;
        block_ = nullptr;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Modifiers

    void Reset() {
        DecBlockRef();
        ptr_ = nullptr;
        block_ = nullptr;
    }

    template <typename Up>
    void Reset(Up* ptr) {  // to be precise we need to be sure that Up* is convertible to T*a
        if (ptr == nullptr) {
            Reset();
        } else {
            DecBlockRef();
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

    void DecBlockRef() {
        if (block_) {
            block_->DecStrongRef();
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
    friend class SharedPtr;  // Now SharedPtr<Y> is a friend of SharedPtr

    template <typename Y>
    friend class WeakPtr;  // for ctor from weak ptr, which is not realized here

    template <typename P, typename... Args>
    friend SharedPtr<P> MakeShared(Args&&... args);
};

// template <typename T, typename U>
// inline bool operator==(const SharedPtr<T>& left, const SharedPtr<U>& right) {
//
// }

// Allocate memory only once
template <typename T, typename... Args>
SharedPtr<T> MakeShared(Args&&... args) {
    SharedPtr<T> s;
    ControlBlockHolder<T>* block = new ControlBlockHolder<T>(std::forward<Args>(args)...);
    // to have ->GetPointer() func we need to do =
    s.ptr_ = block->GetPointer();
    s.block_ = block;
    s.block_->IncStrongRef();
    return s;
};

// Look for usage examples in tests
// template <typename T>
// class EnableSharedFromThis {
// public:
//    SharedPtr<T> SharedFromThis();
//    SharedPtr<const T> SharedFromThis() const;
//
//    WeakPtr<T> WeakFromThis() noexcept;
//    WeakPtr<const T> WeakFromThis() const noexcept;
//};
