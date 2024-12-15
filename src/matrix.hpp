#include "fmt/core.h" // fmt::print
#include "libassert/assert.hpp"
#include <algorithm> // std::copy_n
#include <cstdint> // std::size_t

template <typename T>
class Matrix
{
private:
  std::size_t m_rows;
  std::size_t m_cols;
  T *m_data;

  void
  reset() {
    m_rows = 0;
    m_cols = 0;
    m_data = nullptr;
  }

public:
  Matrix(std::size_t rows, std::size_t cols)
      : m_rows(rows),
        m_cols(cols),
        m_data(new T[rows * cols]) {}
  Matrix(std::size_t rows, std::size_t cols, T init)
      : m_rows(rows),
        m_cols(cols),
        m_data(new T[rows * cols]) {
    std::fill_n(m_data, rows * cols, init);
  }
  Matrix(Matrix const &other)
      : m_rows(other.m_rows),
        m_cols(other.m_cols),
        m_data(new T[m_rows * m_cols]) {
    std::copy_n(other.m_data, other.m_rows * other.m_cols, m_data);
  }
  Matrix(Matrix &&other) noexcept
      : m_rows(other.m_rows),
        m_cols(other.m_cols),
        m_data(other.m_data) {
    other.reset();
  }
  Matrix &
  operator=(Matrix const &other) {
    if (this == &other) {
      return *this;
    }
    m_rows = other.m_rows;
    m_cols = other.m_cols;
    delete[] m_data;
    m_data = new T[m_rows * m_cols];
    std::copy_n(other.m_data, other.m_rows * other.m_cols, m_data);
    return *this;
  }
  Matrix &
  operator=(Matrix &&other) noexcept {
    m_rows = other.m_rows;
    m_cols = other.m_cols;
    delete[] m_data;
    m_data = new T[m_rows * m_cols];
    std::copy_n(other.m_data, other.m_rows * other.m_cols, m_data);
    other.reset();
    return *this;
  }
  ~Matrix() {
    delete[] m_data;
    reset();
  }

  [[nodiscard]] std::size_t
  rows() const {
    return m_rows;
  }
  [[nodiscard]] std::size_t
  cols() const {
    return m_cols;
  }
  T const *
  get_row(std::size_t row) const {
    DEBUG_ASSERT(row < m_rows);
    return &m_data[row * m_cols];
  }
  T *
  get_row(std::size_t row) {
    DEBUG_ASSERT(row < m_rows);
    return &m_data[row * m_cols];
  }
  T const &
  operator()(std::size_t row, std::size_t col) const {
    DEBUG_ASSERT(row < m_rows);
    DEBUG_ASSERT(col < m_cols);
    return m_data[(row * m_cols) + col];
  }
  T &
  operator()(std::size_t row, std::size_t col) {
    DEBUG_ASSERT(row < m_rows);
    DEBUG_ASSERT(col < m_cols);
    return m_data[(row * m_cols) + col];
  }
  void
  print() const {
    for (std::size_t row = 0; row < m_rows; ++row) {
      for (std::size_t col = 0; col < m_cols; ++col) {
        fmt::print("{}", (*this)(row, col));
      }
      fmt::println("");
    }
  }
};
