#include <gtest/gtest.h>

#include <cstddef>

#include "romanov_m_horizontal_matrix_vector/common/include/common.hpp"
#include "romanov_m_horizontal_matrix_vector/mpi/include/ops_mpi.hpp"
#include "romanov_m_horizontal_matrix_vector/seq/include/ops_seq.hpp"
#include "util/include/perf_test_util.hpp"

namespace romanov_m_horizontal_matrix_vector {

class RomanovMHorizontalMatrixVectorRunPerfTests : public ppc::util::BaseRunPerfTests<InType, OutType> {
  const int kRows = 2000;
  const int kCols = 2000;
  InType input_data_;
  OutType expected_data_;

  void SetUp() override {
    std::vector<double> mat(kRows * kCols);
    std::vector<double> vec(kCols);

    expected_data_.resize(kRows);

    for (int j = 0; j < kCols; ++j) {
      vec[j] = 0.5;
    }

    for (int i = 0; i < kRows; ++i) {
      double row_sum = 0.0;
      for (int j = 0; j < kCols; ++j) {
        double val = (i % 10) + (j % 10);
        mat[i * kCols + j] = val;
        row_sum += val * 0.5;
      }
      expected_data_[i] = row_sum;
    }

    input_data_ = std::make_tuple(mat, kRows, kCols, vec);
  }

  bool CheckTestOutputData(OutType &output_data) final {
    if (output_data.size() != expected_data_.size()) {
      return false;
    }
    if (std::abs(output_data[0] - expected_data_[0]) > 1e-4) {
      return false;
    }
    if (std::abs(output_data.back() - expected_data_.back()) > 1e-4) {
      return false;
    }
    return true;
  }

  InType GetTestInputData() final {
    return input_data_;
  }
};

TEST_P(RomanovMHorizontalMatrixVectorRunPerfTests, RunPerfModes) {
  ExecuteTest(GetParam());
}

const auto kAllPerfTasks =
    ppc::util::MakeAllPerfTasks<InType, RomanovMHorizontalMatrixVectorMPI, RomanovMHorizontalMatrixVectorSEQ>(
        PPC_SETTINGS_romanov_m_horizontal_matrix_vector);

const auto kGtestValues = ppc::util::TupleToGTestValues(kAllPerfTasks);

INSTANTIATE_TEST_SUITE_P(RunModeTests, RomanovMHorizontalMatrixVectorRunPerfTests, kGtestValues,
                         RomanovMHorizontalMatrixVectorRunPerfTests::CustomPerfTestName);

}  // namespace romanov_m_horizontal_matrix_vector
