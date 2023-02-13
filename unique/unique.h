#pragma once

#include "compressed_pair.h"

#include <cstddef>  // std::nullptr_t
#include <algorithm>

template <typename T>
struct Slug {
    Slug() = default;

    template <typename Up>
    Slug(const Slug<Up>&) {  // For UpCasts
    }

    void operator()(T* ptr) {
        delete ptr;
    }
};

template <typename T>
struct Slug<T[]> {
    Slug() = default;

    template <typename Up>
    Slug(const Slug<Up[]>&) {  // For UpCasts
    }

    void operator()(T* ptr) {
        delete[] ptr;
    }
};

// Primary template
template <typename T, typename Deleter = Slug<T>>
class UniquePtr {
public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Constructors

    explicit UniquePtr(T* ptr = nullptr) : data_(ptr, Deleter()) {
    }
    template <typename Del>
    UniquePtr(T* ptr, Del&& deleter) : data_(ptr, std::forward<Del>(deleter)) {
    }

    template <typename U, typename Del>
    UniquePtr(UniquePtr<U, Del>&& other) noexcept
            : data_(other.Release(), std::forward<Del>(other.GetDeleter())) {
            // Release other's ptr and forward the deleter
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // `operator=`-s

    UniquePtr& operator=(UniquePtr&& other) noexcept {
        Reset(other.Release());
        GetDeleter() = std::move(other.GetDeleter());
        return *this;
    }
    UniquePtr& operator=(std::nullptr_t) {
        Reset();
        return *this;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Destructor

    ~UniquePtr() {
        if (CpPtr()) {
            GetDeleter()(CpPtr());
        }
        CpPtr() = nullptr;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Modifiers

    T* Release() {  // Releases the ownership of the pointer and returns the raw pointer
        auto ptr = CpPtr();
        CpPtr() = nullptr;
        return ptr;
    }
    void Reset(T* ptr = nullptr) {  // deletes the pointer and resets its value to ptr
        auto old_ptr = CpPtr();
        CpPtr() = ptr;
        if (old_ptr) {
            GetDeleter()(old_ptr);
        }
    }
    void Swap(UniquePtr& other) {
        std::swap(CpPtr(), other.CpPtr());
        std::swap(CpDeleter(), other.CpDeleter());
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Observers

    T* Get() const {
        return CpPtrConst();
    }
    Deleter& GetDeleter() {
        return CpDeleter();
    }
    const Deleter& GetDeleter() const {
        return CpDeleterConst();
    }

    // Returns true if the stored pointer is not nullptr
    explicit operator bool() const {
        if (CpPtrConst()) {
            return true;
        }
        return false;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Single-object dereference operators

    std::add_lvalue_reference_t<T> operator*() const {
        return *CpPtrConst();
    }

    T* operator->() const {
        return CpPtrConst();
    }

private:
    CompressedPair<T*, Deleter> data_;
    // first of data is pointer to object, second of data is Deleter

    const Deleter& CpDeleterConst() const {
        return data_.GetSecond();
    }
    T* CpPtrConst() const {
        return data_.GetFirst();
    }
    T*& CpPtr() {  // this one is specifically for inner assignments
        return data_.GetFirst();
    }
    Deleter& CpDeleter() {
        return data_.GetSecond();
    }
};

// Specialization for arrays
template <typename T, typename Deleter>
class UniquePtr<T[], Deleter> {
public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Constructors

    explicit UniquePtr(T* ptr = nullptr) : data_(ptr, Deleter()) {
    }
    template <typename Del>
    UniquePtr(T* ptr, Del&& deleter) : data_(ptr, std::forward<Del>(deleter)) {
    }

    template <typename U, typename Del>
    UniquePtr(UniquePtr<U, Del>&& other) noexcept
            : data_(other.Release(), std::forward<Del>(other.GetDeleter())) {
            // Release other's ptr and forward the deleter
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // `operator=`-s

    UniquePtr& operator=(UniquePtr&& other) noexcept {
        Reset(other.Release());
        GetDeleter() = std::move(other.GetDeleter());
        return *this;
    }
    UniquePtr& operator=(std::nullptr_t) {
        Reset();
        return *this;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Destructor

    ~UniquePtr() {
        if (CpPtr()) {
            GetDeleter()(CpPtr());
        }
        CpPtr() = nullptr;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Modifiers

    T* Release() {  // Releases the ownership of the pointer and returns the raw pointer
        auto ptr = CpPtr();
        CpPtr() = nullptr;
        return ptr;
    }
    void Reset(T* ptr = nullptr) {  // deletes the pointer and resets its value to ptr
        auto old_ptr = CpPtr();
        CpPtr() = ptr;
        if (old_ptr) {
            GetDeleter()(old_ptr);
        }
    }
    void Swap(UniquePtr& other) {
        std::swap(CpPtr(), other.CpPtr());
        std::swap(CpDeleter(), other.CpDeleter());
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Observers

    T* Get() const {
        return CpPtrConst();
    }
    Deleter& GetDeleter() {
        return CpDeleter();
    }
    const Deleter& GetDeleter() const {
        return CpDeleterConst();
    }

    // Returns true if the stored pointer is not nullptr
    explicit operator bool() const {
        if (CpPtrConst()) {
            return true;
        }
        return false;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Single-object dereference operators

    T& operator[](size_t i) const {
        return CpPtrConst()[i];
    }
    T& operator[](size_t i) {
        return CpPtr()[i];
    }

private:
    CompressedPair<T*, Deleter> data_;
    // first of data is pointer to object, second of data is Deleter

    Deleter& CpDeleter() {
        return data_.GetSecond();
    }
    const Deleter& CpDeleterConst() const {
        return data_.GetSecond();
    }
    T* CpPtrConst() const {
        return data_.GetFirst();
    }
    T*& CpPtr() {  // this one is specifically for inner assignments
        return data_.GetFirst();
    }
};
