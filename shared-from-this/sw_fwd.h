#pragma once

#include <exception>
#include <cstddef>

class BadWeakPtr : public std::exception {};

template <typename T>
class SharedPtr;

template <typename T>
class WeakPtr;

class EnableSharedFromThisBase {  // empty class for ESFT, do not inherit from it -> UB
};

template <typename T>
class EnableSharedFromThis;

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

    void IncStrongRef() override {
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

    ~ControlBlockHolder() override = default;

private:
    alignas(Y) char storage_[sizeof(Y)];  // inner storage of Y
};