#pragma once

#include <cstddef>
#include <utility>

namespace cs106l {

/**
 * @brief A smart pointer that owns an object and deletes it when it goes out of scope.
 * @tparam T The type of the object to manage.
 * @note This class is a simpler version of `std::unique_ptr`.
 */
template <typename T> class unique_ptr {
private:
  T* ptr;
public:
  /**
   * @brief Constructs a new `unique_ptr` from the given pointer.
   * @param ptr The pointer to manage.
   * @note You should avoid using this constructor directly and instead use `make_unique()`.
   */
  unique_ptr(T* ptr) {
    this->ptr = ptr;
  }

  /**
   * @brief Constructs a new `unique_ptr` from `nullptr`.
   */
  unique_ptr(std::nullptr_t) {
    this->ptr = nullptr;
  }

  /**
   * @brief Constructs an empty `unique_ptr`.
   * @note By default, a `unique_ptr` points to `nullptr`.
   */
  unique_ptr() : unique_ptr(nullptr) {}

  /**
   * @brief Dereferences a `unique_ptr` and returns a reference to the object.
   * @return A reference to the object.
   */
  T& operator*() {
    return *this->ptr; // assuming not nullptr, otherwise undefined behavior
  }

  /**
   * @brief Dereferences a `unique_ptr` and returns a const reference to the object.
   * @return A const reference to the object.
   */
  const T& operator*() const {
    return *this->ptr; // assuming not nullptr, otherwise undefined behavior
  }

  /**
   * @brief Returns a pointer to the object managed by the `unique_ptr`.
   * @note This allows for accessing the members of the managed object through the `->` operator.
   * @return A pointer to the object.
   */
  T* operator->() {
    return this->ptr;
  }

  /**
   * @brief Returns a const pointer to the object managed by the `unique_ptr`.
   * @note This allows for accessing the members of the managed object through the `->` operator.
   * @return A const pointer to the object.
   */
  const T* operator->() const {
    return this->ptr;
  }

  /**
   * @brief Returns whether or not the `unique_ptr` is non-null.
   * @note Marked `explicit` so this conversion only fires in
   *       boolean contexts (e.g. inside an `if`) and does not
   *       silently participate as an implicit conversion to `int`.
   * @return `true` if the `unique_ptr` is non-null, `false` otherwise.
   */
  explicit operator bool() const {
    return this->ptr != nullptr;
  }


  ~unique_ptr() {
    delete this->ptr;
  }

  unique_ptr(const unique_ptr&) = delete;
  unique_ptr& operator=(const unique_ptr&) = delete;

  unique_ptr(unique_ptr&& other) noexcept {
    this->ptr = other.ptr;
    other.ptr = nullptr;
  }

  unique_ptr& operator=(unique_ptr&& other) noexcept {
    if (this != &other) {
      delete this->ptr;
      this->ptr = other.ptr;
      other.ptr = nullptr;
    }
    return *this;
  }

  friend bool operator==(const unique_ptr& lhs, const unique_ptr& rhs) {
    return lhs.ptr == rhs.ptr;
  }
  //even just one works since friend and implicit conversion of nullptr_t to unique_ptr
};

/**
 * @brief Creates a new unique_ptr for a type with the given arguments.
 * @example auto ptr = make_unique<int>(5);
 * @tparam T The type to create a unique_ptr for.
 * @tparam Args The types of the arguments to pass to the constructor of T.
 * @param args The arguments to pass to the constructor of T.
 */
template <typename T, typename... Args> 
unique_ptr<T> make_unique(Args&&... args) {
  return unique_ptr<T>(new T(std::forward<Args>(args)...));
}

}