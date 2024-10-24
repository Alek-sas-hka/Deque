#include <exception>
#include <iostream>
#include <random>
#include <stdexcept>
#include <type_traits>
#include <vector>

const int kBucketSize = 10;

template <typename T, typename Allocator = std::allocator<T>>
class Deque {
  using allocator_type = Allocator;
  using alloc_traits = std::allocator_traits<Allocator>;

 public:
  Deque(const Allocator& alloc = Allocator())
      : size_(0),
        deque_vector_(3, nullptr),
        head_(&deque_vector_[1]),
        head_place_(0),
        tail_(&deque_vector_[1]),
        tail_place_(0),
        alloc_(alloc) {
    deque_vector_[1] = alloc_traits::allocate(alloc_, kBucketSize);
  }

  ~Deque() {
    for (size_t i = 0; i < size_;) {
      // std::cout << size_ << std::endl;
      pop_back();
    }
    for (size_t i = 0; i < deque_vector_.size(); ++i) {
      if (deque_vector_[i] != nullptr) {
        alloc_traits::deallocate(alloc_, deque_vector_[i], kBucketSize);
      }
    }
  }

  Deque(size_t count, const Allocator& alloc = Allocator())
      : size_(count), alloc_(alloc) {
    size_t full_buckets = count / kBucketSize;
    bool extra_bucket = static_cast<bool>(count % kBucketSize);
    extra_bucket ? deque_vector_.resize(full_buckets + 1, nullptr)
                 : deque_vector_.resize(full_buckets, nullptr);

    for (size_t i = 0; i < deque_vector_.size(); ++i) {
      deque_vector_[i] = alloc_traits::allocate(alloc_, kBucketSize);
    }
    size_t counter = 0;
    size_t idx;
    int jdx;
    try {
      for (idx = 0; (idx < deque_vector_.size()) && (counter < count); ++idx) {
        for (jdx = 0; jdx < kBucketSize && counter < count; ++jdx, ++counter) {
          alloc_traits::construct(alloc_, &deque_vector_[idx][jdx]);
        }
      }
    } catch (...) {
      int destroyed = 0;
      for (idx = 0; (idx < deque_vector_.size()) &&
                    (destroyed < static_cast<int>(counter));
           ++idx) {
        for (jdx = 0;
             jdx < kBucketSize && destroyed < static_cast<int>(counter);
             ++jdx, ++destroyed) {
          alloc_traits::destroy(alloc_, &deque_vector_[idx][jdx]);
        }
      }
      for (size_t i = 0; i < deque_vector_.size(); ++i) {
        if (deque_vector_[i] != nullptr) {
          alloc_traits::deallocate(alloc_, deque_vector_[i], kBucketSize);
        }
      }
      size_ = 0;
      deque_vector_.assign(3, nullptr);
      head_ = &deque_vector_[1];
      tail_ = &deque_vector_[1];
      head_place_ = 0;
      tail_place_ = 0;
      throw;
    }
    tail_ = (deque_vector_).data();
    tail_place_ = 0;
    head_ = &deque_vector_[--idx];
    head_place_ = --jdx;
  }

  Deque(size_t count, const T& value, const Allocator& alloc = Allocator())
      : alloc_(alloc) {
    size_t full_buckets = count / kBucketSize;
    bool extra_bucket = static_cast<bool>(count % kBucketSize);
    extra_bucket ? deque_vector_.resize(full_buckets + 1, nullptr)
                 : deque_vector_.resize(full_buckets, nullptr);

    for (size_t i = 0; i < deque_vector_.size(); ++i) {
      deque_vector_[i] = alloc_traits::allocate(alloc_, kBucketSize);
    }
    size_ = count;
    size_t counter = 0;
    size_t idx;
    int jdx;
    try {
      for (idx = 0; (idx < deque_vector_.size()) && (counter < count); ++idx) {
        for (jdx = 0; jdx < kBucketSize && counter < count; ++jdx, ++counter) {
          alloc_traits::construct(alloc_, &deque_vector_[idx][jdx], value);
        }
      }
    } catch (...) {
      int destroyed = 0;
      for (idx = 0; (idx < deque_vector_.size()) &&
                    (destroyed < static_cast<int>(counter));
           ++idx) {
        for (jdx = 0;
             jdx < kBucketSize && destroyed < static_cast<int>(counter);
             ++jdx, ++destroyed) {
          alloc_traits::destroy(alloc_, &deque_vector_[idx][jdx]);
        }
      }
      for (size_t i = 0; i < deque_vector_.size(); ++i) {
        if (deque_vector_[i] != nullptr) {
          alloc_traits::deallocate(alloc_, deque_vector_[i], kBucketSize);
        }
      }
      size_ = 0;
      deque_vector_.assign(3, nullptr);
      head_ = &deque_vector_[1];
      tail_ = &deque_vector_[1];
      head_place_ = 0;
      tail_place_ = 0;
      throw;
    }
    tail_ = (deque_vector_).data();
    tail_place_ = 0;
    head_ = &deque_vector_[--idx];
    head_place_ = --jdx;
  }

  Deque(std::initializer_list<T> list, const Allocator& alloc = Allocator())
      : alloc_(alloc) {
    size_ = 0;
    deque_vector_.assign(3, nullptr);
    head_ = &deque_vector_[1];
    tail_ = &deque_vector_[1];
    head_place_ = 0;
    tail_place_ = 0;
    deque_vector_[1] = alloc_traits::allocate(alloc_, kBucketSize);
    for (auto& elem : list) {
      push_back(elem);
    }
  }

  Deque(const Deque& src)
      : alloc_(
            alloc_traits::select_on_container_copy_construction(src.alloc_)) {
    size_ = 0;
    deque_vector_.assign(3, nullptr);
    head_ = &deque_vector_[1];
    tail_ = &deque_vector_[1];
    head_place_ = 0;
    tail_place_ = 0;
    deque_vector_[1] = alloc_traits::allocate(alloc_, kBucketSize);
    try {
      for (size_t i = 0; i < src.size(); ++i) {
        push_back(src[i]);
      }
    } catch (...) {
      for (size_t i = 0; i < size_;) {
        pop_back();
      }
    }
  }

  Deque(Deque&& src) {
    size_ = src.size_;
    deque_vector_ = std::move(src.deque_vector_);
    alloc_ = std::move(src.alloc_);
    head_ = src.head_;
    head_place_ = src.head_place_;
    tail_ = src.tail_;
    tail_place_ = src.tail_place_;
    src.size_ = 0;
    src.deque_vector_.assign(3, nullptr);
    src.head_ = &src.deque_vector_[1];
    src.tail_ = &src.deque_vector_[1];
    src.head_place_ = 0;
    src.tail_place_ = 0;
    src.alloc_ = Allocator();
    src.deque_vector_[1] = alloc_traits::allocate(src.alloc_, kBucketSize);
  }

  void swap(Deque& right) {
    std::swap(size_, right.size_);
    std::swap(deque_vector_, right.deque_vector_);
    std::swap(head_, right.head_);
    std::swap(head_place_, right.head_place_);
    std::swap(tail_, right.tail_);
    std::swap(tail_place_, right.tail_place_);
  }

  Deque& operator=(const Deque& right) {
    if (this == &right) {
      return *this;
    }
    Deque tmp(right);
    swap(tmp);
    if (alloc_traits::propagate_on_container_copy_assignment::value &&
        alloc_ != right.alloc_) {
      alloc_ = right.alloc_;
    }
    return *this;
  }

  Deque& operator=(Deque&& right) {
    if (this == &right) {
      return *this;
    }
    Deque tmp(right);
    swap(tmp);
    if (alloc_traits::propagate_on_container_move_assignment::value &&
        alloc_ != right.alloc_) {
      alloc_ = right.alloc_;
    }
    return *this;
  }

  size_t size() const { return size_; }

  bool empty() const { return size_ == 0; }

  const T& operator[](size_t idx) const {
    if (idx < static_cast<size_t>(kBucketSize - tail_place_)) {
      return (*(tail_))[tail_place_ + idx];
    }

    size_t skip_buckets = (tail_place_ + idx) / kBucketSize;
    size_t place = (tail_place_ + idx) % kBucketSize;
    return (*(tail_ + skip_buckets))[place];
  }

  T& operator[](size_t idx) {
    if (idx < static_cast<size_t>(kBucketSize - tail_place_)) {
      return (*(tail_))[tail_place_ + idx];
    }

    size_t skip_buckets = (tail_place_ + idx) / kBucketSize;
    size_t place = (tail_place_ + idx) % kBucketSize;
    return (*(tail_ + skip_buckets))[place];
  }

  T& at(size_t idx) {
    if (size_ <= idx) {
      throw std::out_of_range("I can't, I have paws :( \n");
    }
    if (idx < static_cast<size_t>(kBucketSize - tail_place_)) {
      return (*(tail_))[tail_place_ + idx];
    }

    size_t skip_buckets = (tail_place_ + idx) / kBucketSize;
    size_t place = (tail_place_ + idx) % kBucketSize;
    return (*(tail_ + skip_buckets))[place];
  }

  const T& at(size_t idx) const {
    if (size_ <= idx) {
      throw std::out_of_range("I can't, I have paws :( \n");
    }
    if (idx < static_cast<size_t>(kBucketSize - tail_place_)) {
      return (*(tail_))[tail_place_ + idx];
    }

    size_t skip_buckets = (tail_place_ + idx) / kBucketSize;
    size_t place = (tail_place_ + idx) % kBucketSize;
    return (*(tail_ + skip_buckets))[place];
  }

  void reserve() {
    if (head_ == &deque_vector_[deque_vector_.size() - 1] &&
            head_place_ == kBucketSize - 1 ||
        tail_ == (deque_vector_).data() && tail_place_ == 0) {
      size_t full_buckets = size_ / kBucketSize;
      bool extra_bucket = static_cast<bool>(size_ % kBucketSize);
      size_t new_size;
      extra_bucket ? new_size = 3 * (full_buckets + 1)
                   : new_size = 3 * full_buckets;
      std::vector<T*> new_vector(new_size, nullptr);

      auto start = tail_;
      for (size_t i = new_size / 3; i < new_size * 2 / 3; ++i, ++start) {
        new_vector[i] = *start;
      }
      deque_vector_ = new_vector;
      tail_ = &deque_vector_[new_size / 3];
      head_ = &deque_vector_[new_size * 2 / 3 - 1];
    }
  }

  void push_back(const T& unit) {
    try {
      if (empty()) {
        ++size_;
        alloc_traits::construct(alloc_, *head_ + head_place_, unit);
        return;
      }
      reserve();
      ++size_;
      if (head_place_ < kBucketSize - 1) {
        ++head_place_;
      } else {
        ++head_;
        if (*head_ == nullptr) {
          *head_ = alloc_traits::allocate(alloc_, kBucketSize);
        }
        head_place_ = 0;
      }
      alloc_traits::construct(alloc_, *head_ + head_place_, unit);
    } catch (...) {
      size_--;
      throw;
    }
  }

  void push_back(T&& unit) {
    try {
      if (empty()) {
        ++size_;
        alloc_traits::construct(alloc_, *head_ + head_place_, std::move(unit));
        return;
      }
      reserve();
      ++size_;
      if (head_place_ < kBucketSize - 1) {
        ++head_place_;
      } else {
        ++head_;
        if (*head_ == nullptr) {
          *head_ = alloc_traits::allocate(alloc_, kBucketSize);
        }
        head_place_ = 0;
      }
      alloc_traits::construct(alloc_, *head_ + head_place_, std::move(unit));
    } catch (...) {
      size_--;
      throw;
    }
  }

  template <typename... Args>
  void emplace_back(Args&&... args) {
    try {
      if (empty()) {
        ++size_;
        alloc_traits::construct(alloc_, *head_ + head_place_,
                                std::forward<Args>(args)...);
        return;
      }
      reserve();
      ++size_;
      if (head_place_ < kBucketSize - 1) {
        ++head_place_;
      } else {
        ++head_;
        if (*head_ == nullptr) {
          *head_ = alloc_traits::allocate(alloc_, kBucketSize);
        }
        head_place_ = 0;
      }
      alloc_traits::construct(alloc_, *head_ + head_place_,
                              std::forward<Args>(args)...);
    } catch (...) {
      size_--;
      throw;
    }
  }

  void push_front(const T& unit) {
    try {
      if (empty()) {
        ++size_;
        alloc_traits::construct(alloc_, *head_ + head_place_, unit);
        return;
      }

      reserve();
      try {
        if (tail_place_ > 0) {
          --tail_place_;
        } else {
          --tail_;
          if (*tail_ == nullptr) {
            *tail_ = alloc_traits::allocate(alloc_, kBucketSize);
          }
          tail_place_ = kBucketSize - 1;
        }
        alloc_traits::construct(alloc_, *tail_ + tail_place_, unit);
        ++size_;
      } catch (...) {
        --size_;
      }
    } catch (...) {
      size_--;
      throw;
    }
  }

  void push_front(T&& unit) {
    try {
      if (empty()) {
        ++size_;
        alloc_traits::construct(alloc_, *head_ + head_place_, std::move(unit));
        return;
      }

      reserve();
      try {
        if (tail_place_ > 0) {
          --tail_place_;
        } else {
          --tail_;
          if (*tail_ == nullptr) {
            *tail_ = alloc_traits::allocate(alloc_, kBucketSize);
          }
          tail_place_ = kBucketSize - 1;
        }
        alloc_traits::construct(alloc_, *tail_ + tail_place_, std::move(unit));
        ++size_;
      } catch (...) {
        --size_;
      }
    } catch (...) {
      size_--;
      throw;
    }
  }

  template <typename... Args>
  void push_front(Args&&... args) {
    try {
      if (empty()) {
        ++size_;
        alloc_traits::construct(alloc_, *head_ + head_place_,
                                std::forward<Args>(args)...);
        return;
      }

      reserve();
      try {
        if (tail_place_ > 0) {
          --tail_place_;
        } else {
          --tail_;
          if (*tail_ == nullptr) {
            *tail_ = alloc_traits::allocate(alloc_, kBucketSize);
          }
          tail_place_ = kBucketSize - 1;
        }
        alloc_traits::construct(alloc_, *tail_ + tail_place_,
                                std::forward<Args>(args)...);
        ++size_;
      } catch (...) {
        --size_;
      }
    } catch (...) {
      size_--;
      throw;
    }
  }

  void pop_back() {
    if (empty()) {
      return;
    }
    alloc_traits::destroy(alloc_, *head_ + head_place_);
    if (size_ == 1) {
      size_ = 0;
      return;
    }
    if (head_place_ > 0) {
      --head_place_;
    } else {
      --head_;
      head_place_ = kBucketSize - 1;
    }
    --size_;
  }

  void pop_front() {
    if (empty()) {
      return;
    }
    alloc_traits::destroy(alloc_, *tail_ + tail_place_);
    if (size_ == 1) {
      size_ = 0;
      return;
    }
    if (tail_place_ + 1 == kBucketSize) {
      ++tail_;
      tail_place_ = 0;
    } else {
      ++tail_place_;
    }
    --size_;
  }

  template <bool IsConst>
  class Iterator;

  template <bool IsConst>
  class ReverseIterator;

  using const_iterator = Iterator<true>;
  using iterator = Iterator<false>;
  using reverse_iterator = ReverseIterator<false>;
  using const_reverse_iterator = ReverseIterator<true>;

  iterator begin() { return iterator(tail_, tail_place_); }

  iterator end() {
    if (empty()) {
      return iterator(tail_, tail_place_);
    }
    if (head_place_ == kBucketSize - 1) {
      return iterator(head_ + 1, 0);
    }
    return iterator(head_, head_place_ + 1);
  }

  const_iterator begin() const { return const_iterator(tail_, tail_place_); }

  const_iterator end() const {
    if (empty()) {
      return const_iterator(tail_, tail_place_);
    }
    if (head_place_ == kBucketSize - 1) {
      return Iterator<true>(head_ + 1, 0);
    }
    return const_iterator(head_, head_place_ + 1);
  }

  const_iterator cbegin() const { return const_iterator(tail_, tail_place_); }

  const_iterator cend() const {
    if (empty()) {
      return Iterator<true>(tail_, tail_place_);
    }
    if (head_place_ == kBucketSize - 1) {
      return Iterator<true>(head_ + 1, 0);
    }
    return Iterator<true>(head_, head_place_ + 1);
  }

  reverse_iterator rbegin() { return reverse_iterator(head_, head_place_); }

  reverse_iterator rend() {
    if (empty()) {
      return reverse_iterator(head_, head_place_);
    }
    if (tail_place_ == 0) {
      return reverse_iterator(tail_ - 1, kBucketSize - 1);
    }
    return reverse_iterator(tail_, tail_place_ - 1);
  }

  const_reverse_iterator rbegin() const {
    return const_reverse_iterator(head_, head_place_);
  }

  const_reverse_iterator rend() const {
    if (empty()) {
      return const_reverse_iterator(head_, head_place_);
    }
    if (tail_place_ == 0) {
      return const_reverse_iterator(tail_ - 1, kBucketSize - 1);
    }
    return const_reverse_iterator(tail_, tail_place_ - 1);
  }

  const_reverse_iterator crbegin() const {
    return const_reverse_iterator(head_, head_place_);
  }

  const_reverse_iterator crend() const {
    if (empty()) {
      return const_reverse_iterator(head_, head_place_);
    }
    if (tail_place_ == 0) {
      return const_reverse_iterator(tail_ - 1, kBucketSize - 1);
    }
    return const_reverse_iterator(tail_, tail_place_ - 1);
  }

  void insert(Iterator<false> iterator, const T& elem) {
    auto eend = end();

    T first(elem);
    T second(elem);
    for (; iterator < eend; ++iterator) {
      second = *iterator;
      *iterator = first;
      first = second;
    }
    push_back(first);
  }

  void emplace(Iterator<false> iterator, T&& elem) {
    auto eend = end();

    T first(std::move(elem));
    T second(std::move(elem));
    for (; iterator < eend; ++iterator) {
      second = *iterator;
      *iterator = first;
      first = second;
    }
    push_back(first);
  }

  void erase(Iterator<false> iterator) {
    auto eend = end();
    for (; iterator < eend - 1; ++iterator) {
      *iterator = *(iterator + 1);
    }
    alloc_traits::destroy(alloc_, *head_ + head_place_);
    --size_;
    if (head_place_ == 0) {
      --head_;
      head_place_ = kBucketSize - 1;
    } else {
      --head_place_;
    }
  }

  Allocator get_allocator() const { return alloc_; }

 private:
  std::vector<T*> deque_vector_;
  size_t size_;
  T** head_;
  T** tail_;
  int head_place_;
  int tail_place_;
  Allocator alloc_;
};

template <typename T, typename Allocator>
template <bool IsConst>
class Deque<T, Allocator>::Iterator {
 public:
  using value_type = std::remove_cv_t<T>;
  using iterator_category = std::random_access_iterator_tag;
  using difference_type = std::ptrdiff_t;
  using pointer = std::conditional_t<IsConst, const T, T>*;
  using reference = std::conditional_t<IsConst, const T, T>&;

  T** get_pointer() const { return pointer_; }

  size_t get_place() const { return place_; }

  operator Iterator<true>() const { return {pointer_, place_}; }

  operator Iterator<false>() const = delete;

  Iterator() : pointer_(nullptr), place_(0) {}

  Iterator(T** ptr, size_t pos) : pointer_(ptr), place_(pos) {}

  Iterator operator++(int) {
    Iterator copy(*this);
    ++*this;
    return copy;
  }

  Iterator& operator++() {
    if (place_ == kBucketSize - 1) {
      place_ = 0;
      ++pointer_;
    } else {
      ++place_;
    }
    return *this;
  }

  Iterator& operator+=(int shift) {
    if (shift < 0) {
      *this -= (-shift);
      return *this;
    }
    pointer_ += (place_ + shift) / kBucketSize;
    place_ = (place_ + shift) % kBucketSize;
    return *this;
  }

  Iterator operator+(int shift) const {
    Iterator copy(pointer_, place_);
    copy += shift;
    return copy;
  }

  Iterator operator--(int) {
    Iterator copy(*this);
    --*this;
    return copy;
  }

  Iterator& operator--() {
    if (place_ == 0) {
      place_ = kBucketSize - 1;
      pointer_--;
    } else {
      place_--;
    }
    return *this;
  }

  Iterator& operator-=(int shift) {
    if (shift < 0) {
      *this += (-shift);
      return *this;
    }

    if (static_cast<int>(place_) - shift < 0) {
      size_t num_of_layers_to_skip = (shift - place_ - 1) / kBucketSize;
      int diff = shift % kBucketSize - int(place_);
      if (diff > 0) {
        place_ = kBucketSize - diff;
      } else {
        place_ -= shift % kBucketSize;
      }
      pointer_ -= (num_of_layers_to_skip + 1);
    } else {
      place_ -= shift;
    }
    return *this;
  }

  Iterator operator-(int shift) const {
    Iterator copy(pointer_, place_);
    copy -= shift;
    return copy;
  }

  template <bool ConstR>
  bool operator<(const Iterator<ConstR> kRight) const {
    if (pointer_ < kRight.get_pointer()) {
      return true;
    }
    if (pointer_ > kRight.get_pointer()) {
      return false;
    }
    return place_ < kRight.get_place();
  }

  template <bool ConstR>
  bool operator==(const Iterator<ConstR> kRight) const {
    return (pointer_ == kRight.get_pointer() && place_ == kRight.get_place());
  }

  template <bool ConstR>
  bool operator>=(const Iterator<ConstR> kRight) const {
    return !(*this < kRight);
  }

  template <bool ConstR>
  bool operator>(const Iterator<ConstR> kRight) const {
    return !(*this < kRight) && !(*this == kRight);
  }

  template <bool ConstR>
  bool operator<=(const Iterator<ConstR> kRight) const {
    return !(*this > kRight);
  }

  reference operator*() const {
    if (pointer_ == nullptr) {
      throw std::runtime_error(":(");
    }
    return (*pointer_)[place_];
  }

  pointer operator->() const {
    if (pointer_ == nullptr) {
      throw std::runtime_error(":(");
    }
    return *pointer_ + place_;
  }

  template <bool ConstR>
  difference_type operator-(const Iterator<ConstR> kRight) const {
    return (static_cast<int>((pointer_ - kRight.get_pointer()) * kBucketSize) +
            static_cast<int>(place_) - static_cast<int>(kRight.get_place()));
  }

 private:
  T** pointer_;
  size_t place_;
};

template <typename T, typename Allocator>
template <bool IsConst>
class Deque<T, Allocator>::ReverseIterator {
 public:
  using value_type = std::remove_cv_t<T>;
  using iterator_category = std::random_access_iterator_tag;
  using difference_type = std::ptrdiff_t;
  using pointer = std::conditional_t<IsConst, const T, T>*;
  using reference = std::conditional_t<IsConst, const T, T>&;

  T** get_pointer() const { return pointer_; }

  size_t get_place() const { return place_; }

  operator ReverseIterator<true>() const { return {pointer_, place_}; }

  operator ReverseIterator<false>() const = delete;

  ReverseIterator() : pointer_(nullptr), place_(0) {}

  ReverseIterator(T** ptr, size_t pos) : pointer_(ptr), place_(pos) {}

  ReverseIterator operator++(int) {
    ReverseIterator copy(*this);
    ++*this;
    return copy;
  }

  ReverseIterator& operator++() {
    if (place_ == 0) {
      place_ = kBucketSize - 1;
      pointer_--;
    } else {
      place_--;
    }
    return *this;
  }

  ReverseIterator& operator+=(int shift) {
    Iterator<IsConst> iter(pointer_, place_);
    iter -= shift;
    pointer_ = iter.get_pointer();
    place_ = iter.get_place();
    return *this;
  }

  ReverseIterator operator+(int shift) const {
    ReverseIterator copy(pointer_, place_);
    copy += shift;
    return copy;
  }

  ReverseIterator operator--(int) {
    ReverseIterator copy(*this);
    --*this;
    return copy;
  }

  ReverseIterator& operator--() {
    if (place_ == kBucketSize - 1) {
      place_ = 0;
      pointer_++;
    } else {
      place_++;
    }
    return *this;
  }

  ReverseIterator& operator-=(int shift) {
    Iterator<IsConst> iter(pointer_, place_);
    iter += shift;
    pointer_ = iter.get_pointer();
    place_ = iter.get_place();
    return *this;
  }

  ReverseIterator operator-(int shift) const {
    ReverseIterator copy(pointer_, place_);
    copy -= shift;
    return copy;
  }

  template <bool ConstR>
  bool operator<(const ReverseIterator<ConstR> kRight) const {
    if (pointer_ > kRight.get_pointer()) {
      return true;
    }
    if (pointer_ < kRight.get_pointer()) {
      return false;
    }
    return place_ > kRight.get_place();
  }

  template <bool ConstR>
  bool operator==(const ReverseIterator<ConstR> kRight) const {
    return (pointer_ == kRight.get_pointer() && place_ == kRight.get_place());
  }

  template <bool ConstR>
  bool operator>=(const ReverseIterator<ConstR> kRight) const {
    return !(*this < kRight);
  }

  template <bool ConstR>
  bool operator>(const ReverseIterator<ConstR> kRight) const {
    return !(*this < kRight) && !(*this == kRight);
  }

  template <bool ConstR>
  bool operator<=(const ReverseIterator<ConstR> kRight) const {
    return !(*this > kRight);
  }

  reference operator*() const {
    if (pointer_ == nullptr) {
      throw std::runtime_error(":(");
    }
    return (*pointer_)[place_];
  }

  pointer operator->() const {
    if (pointer_ == nullptr) {
      throw std::runtime_error(":(");
    }
    return *pointer_ + place_;
  }

  template <bool ConstR>
  difference_type operator-(const ReverseIterator<ConstR> kRight) const {
    Iterator<IsConst> it1(get_pointer(), get_place());
    Iterator<ConstR> it2(kRight.get_pointer(), kRight.get_place());
    return it2 - it1;
  }

 private:
  T** pointer_;
  size_t place_;
};
