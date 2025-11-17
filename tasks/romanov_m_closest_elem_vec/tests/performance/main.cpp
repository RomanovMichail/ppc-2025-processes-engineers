#include <gtest/gtest.h>

#include "romanov_m_closest_elem_vec/common/include/common.hpp"
#include "romanov_m_closest_elem_vec/mpi/include/ops_mpi.hpp"
#include "romanov_m_closest_elem_vec/seq/include/ops_seq.hpp"
#include "util/include/perf_test_util.hpp"

namespace romanov_m_closest_elem_vec {

class RomanovMClosestElemVecRunPerfTestProcesses : public ppc::util::BaseRunPerfTests<InType, OutType> {
  const int kCount_ = 100;
  InType input_data_{};

  void SetUp() override {
    input_data_ = kCount_;
  }

  bool CheckTestOutputData(OutType &output_data) final {
    return input_data_ == output_data;
  }

  InType GetTestInputData() final {
    return input_data_;
  }
};

TEST_P(RomanovMClosestElemVecRunPerfTestProcesses, RunPerfModes) {
  ExecuteTest(GetParam());
}

const auto kAllPerfTasks =
    ppc::util::MakeAllPerfTasks<InType, RomanovMClosestElemVecMPI, RomanovMClosestElemVecSEQ>(PPC_SETTINGS_example_processes);

const auto kGtestValues = ppc::util::TupleToGTestValues(kAllPerfTasks);

const auto kPerfTestName = RomanovMClosestElemVecRunPerfTestProcesses::CustomPerfTestName;

INSTANTIATE_TEST_SUITE_P(RunModeTests, RomanovMClosestElemVecRunPerfTestProcesses, kGtestValues, kPerfTestName);

}  // namespace nesterov_a_test_task_processes
