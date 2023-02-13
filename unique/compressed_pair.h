#pragma once

#include <type_traits>
#include <utility>

// Me think, why waste time write lot code, when few code do trick.

// std::remove_cvref<T> ( C++20 ) removes const, volatile and reference from the class, so it
// is convenient to inherit from it to use copy and move constructors. That's why I used it
// std::remove_cvref<T>::type returns the "pure" T type

template <typename V>
constexpr bool is_empty_v = std::is_empty_v<V> && !std::is_final_v<V>;  // NOLINT

template <typename V, typename U>
constexpr bool is_derived_v = std::is_base_of_v<U, V> || std::is_base_of_v<V, U>;  // NOLINT

template <typename F, typename S, bool F_Empty = is_empty_v<F>, bool S_Empty = is_empty_v<S>,
        bool Same = is_derived_v<F, S> >
class CompressedPair;

template <typename F, typename S>
class CompressedPair<F, S, true, true, false> : F, S {
public:
    CompressedPair() {
    }
    CompressedPair(const F& first, const S& second) : F(first), S(second) {
    }
    CompressedPair(F&& first, const S& second) : F(std::move(first)), S(second) {
    }
    CompressedPair(const F& first, S&& second) : F(first), S(std::move(second)) {
    }
    CompressedPair(F&& first, S&& second) : F(std::move(first)), S(std::move(second)) {
    }

    F& GetFirst() {
        return *this;
    }

    const F& GetFirst() const {
        return *this;
    }

    S& GetSecond() {
        return *this;
    }

    const S& GetSecond() const {
        return *this;
    }
};

template <typename F, typename S>
class CompressedPair<F, S, true, false, false> : F {
public:
    CompressedPair() : second_() {
    }
    CompressedPair(const F& first, const S& second) : F(first), second_(second) {
    }
    CompressedPair(F&& first, const S& second) : F(std::move(first)), second_(second) {
    }
    CompressedPair(const F& first, S&& second) : F(first), second_(std::move(second)) {
    }
    CompressedPair(F&& first, S&& second) : F(std::move(first)), second_(std::move(second)) {
    }

    F& GetFirst() {
        return *this;
    }

    const F& GetFirst() const {
        return *this;
    }

    S& GetSecond() {
        return second_;
    }

    const S& GetSecond() const {
        return second_;
    }

private:
    S second_;
};

template <typename F, typename S>
class CompressedPair<F, S, false, true, false> : S {
public:
    CompressedPair() : first_() {
    }
    CompressedPair(const F& first, const S& second) : first_(first), S(second) {
    }
    CompressedPair(F&& first, const S& second) : first_(std::move(first)), S(second) {
    }
    CompressedPair(const F& first, S&& second) : first_(first), S(std::move(second)) {
    }
    CompressedPair(F&& first, S&& second) : first_(std::move(first)), S(std::move(second)) {
    }

    F& GetFirst() {
        return first_;
    }

    const F& GetFirst() const {
        return first_;
    }

    S& GetSecond() {
        return *this;
    }

    const S& GetSecond() const {
        return *this;
    }

private:
    F first_;
};

template <typename F, typename S>
class CompressedPair<F, S, false, false, false> {
public:
    CompressedPair() : first_(), second_() {
    }
    CompressedPair(const F& first, const S& second) : first_(first), second_(second) {
    }
    CompressedPair(F&& first, const S& second) : first_(std::move(first)), second_(second) {
    }
    CompressedPair(const F& first, S&& second) : first_(first), second_(std::move(second)) {
    }
    CompressedPair(F&& first, S&& second) : first_(std::move(first)), second_(std::move(second)) {
    }

    F& GetFirst() {
        return first_;
    }

    const F& GetFirst() const {
        return first_;
    }

    S& GetSecond() {
        return second_;
    }

    const S& GetSecond() const {
        return second_;
    }

private:
    F first_;
    S second_;
};

template <typename F, typename S>
class CompressedPair<F, S, true, true, true> : F {
public:
    CompressedPair() : first_() {
    }
    CompressedPair(const F& first, const S& second) : first_(first), S(second) {
    }
    CompressedPair(F&& first, const S& second) : first_(std::move(first)), S(second) {
    }
    CompressedPair(const F& first, S&& second) : first_(first), S(std::move(second)) {
    }
    CompressedPair(F&& first, S&& second) : first_(std::move(first)), S(std::move(second)) {
    }

    F& GetFirst() {
        return first_;
    }

    const F& GetFirst() const {
        return first_;
    }

    S& GetSecond() {
        return *this;
    }

    const S& GetSecond() const {
        return *this;
    }

private:
    F first_;  // extra case
};

template <typename F, typename S>
class CompressedPair<F, S, true, false, true> : F {
public:
    CompressedPair() : second_() {
    }
    CompressedPair(const F& first, const S& second) : F(first), second_(second) {
    }
    CompressedPair(F&& first, const S& second) : F(std::move(first)), second_(second) {
    }
    CompressedPair(const F& first, S&& second) : F(first), second_(std::move(second)) {
    }
    CompressedPair(F&& first, S&& second) : F(std::move(first)), second_(std::move(second)) {
    }

    F& GetFirst() {
        return *this;
    }

    const F& GetFirst() const {
        return *this;
    }

    S& GetSecond() {
        return second_;
    }

    const S& GetSecond() const {
        return second_;
    }

private:
    S second_;
};

template <typename F, typename S>
class CompressedPair<F, S, false, true, true> : S {
public:
    CompressedPair() : first_() {
    }
    CompressedPair(const F& first, const S& second) : first_(first), S(second) {
    }
    CompressedPair(F&& first, const S& second) : first_(std::move(first)), S(second) {
    }
    CompressedPair(const F& first, S&& second) : first_(first), S(std::move(second)) {
    }
    CompressedPair(F&& first, S&& second) : first_(std::move(first)), S(std::move(second)) {
    }

    F& GetFirst() {
        return first_;
    }

    const F& GetFirst() const {
        return first_;
    }

    S& GetSecond() {
        return *this;
    }

    const S& GetSecond() const {
        return *this;
    }

private:
    F first_;
};

template <typename F, typename S>
class CompressedPair<F, S, false, false, true> {
public:
    CompressedPair() : first_(), second_() {
    }
    CompressedPair(const F& first, const S& second) : first_(first), second_(second) {
    }
    CompressedPair(F&& first, const S& second) : first_(std::move(first)), second_(second) {
    }
    CompressedPair(const F& first, S&& second) : first_(first), second_(std::move(second)) {
    }
    CompressedPair(F&& first, S&& second) : first_(std::move(first)), second_(std::move(second)) {
    }

    F& GetFirst() {
        return first_;
    }

    const F& GetFirst() const {
        return first_;
    }

    S& GetSecond() {
        return second_;
    }

    const S& GetSecond() const {
        return second_;
    }

private:
    F first_;
    S second_;
};
