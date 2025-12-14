#include "romanov_m_horizontal_matrix_vector/seq/include/ops_seq.hpp"

#include <cstddef>
#include <vector>

namespace romanov_m_horizontal_matrix_vector {

RomanovMHorizontalMatrixVectorSEQ::RomanovMHorizontalMatrixVectorSEQ(const InType &in) {
  SetTypeOfTask(GetStaticTypeOfTask());
  GetInput() = in;
  GetOutput() = {};
}

bool RomanovMHorizontalMatrixVectorSEQ::ValidationImpl() {
  const auto &mat = std::get<0>(GetInput());
  const int r = std::get<1>(GetInput());
  const int c = std::get<2>(GetInput());
  const auto &v = std::get<3>(GetInput());

  if (r <= 0 || c <= 0) {
    return false;
  }
  if (mat.size() != static_cast<size_t>(r * c)) {
    return false;
  }
  if (v.size() != static_cast<size_t>(c)) {
    return false;
  }

  return true;
}

bool RomanovMHorizontalMatrixVectorSEQ::PreProcessingImpl() {
  GetOutput().resize(std::get<1>(GetInput()));
  return true;
}

bool RomanovMHorizontalMatrixVectorSEQ::RunImpl() {
  const auto &matrix = std::get<0>(GetInput());
  const int rows = std::get<1>(GetInput());
  const int cols = std::get<2>(GetInput());
  const auto &vec = std::get<3>(GetInput());

  auto &res = GetOutput();

  for (int i = 0; i < rows; ++i) {
    double temp = 0.0;
    for (int j = 0; j < cols; ++j) {
      temp += matrix[i * cols + j] * vec[j];
    }
    res[i] = temp;
  }

  return true;
}

bool RomanovMHorizontalMatrixVectorSEQ::PostProcessingImpl() {
  return true;
}

}  // namespace romanov_m_horizontal_matrix_vector
