#include <gtest/gtest.h>

#include <algorithm>
#include <cstddef>
#include <random>
#include <vector>

#include "romanov_m_prohod_jarvisa/common/include/common.hpp"
#include "romanov_m_prohod_jarvisa/mpi/include/ops_mpi.hpp"
#include "romanov_m_prohod_jarvisa/seq/include/ops_seq.hpp"
#include "util/include/perf_test_util.hpp"

namespace romanov_m_prohod_jarvisa {

static bool IsValidConvexHull(const std::vector<Point> &points, const std::vector<Point> &hull) {
  if (hull.empty()) {
    return points.empty();
  }
  for (const auto &h : hull) {
    bool found = false;
    for (const auto &p : points) {
      if (p == h) {
        found = true;
        break;
      }
    }
    if (!found) {
      return false;
    }
  }
  for (std::size_t i = 0; i < hull.size(); ++i) {
    if (CalcCross(hull[i], hull[(i + 1) % hull.size()], hull[(i + 2) % hull.size()]) < 0) {
      return false;
    }
  }
  return true;
}

class RomanovMProhodJarvisaRunPerfTests : public ppc::util::BaseRunPerfTests<InType, OutType> {
 public:
  static constexpr std::size_t kSize = 10000;

 protected:
  void SetUp() override {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dist(-10000, 10000);
    i_points_.resize(kSize);
    for (std::size_t i = 0; i < kSize; ++i) {
      i_points_[i] = Point{dist(gen), dist(gen)};
    }
    i_points_[0] = {-20000, -20000};
    i_points_[1] = {20000, -20000};
    i_points_[2] = {20000, 20000};
    i_points_[3] = {-20000, 20000};
  }

  bool CheckTestOutputData(OutType &output_data) final {
    return IsValidConvexHull(i_points_, output_data);
  }
  InType GetTestInputData() final {
    return i_points_;
  }

 private:
  InType i_points_;
};

TEST_P(RomanovMProhodJarvisaRunPerfTests, RunPerfModes) {
  ExecuteTest(GetParam());
}

const auto kAllPerfTasks = ppc::util::MakeAllPerfTasks<InType, RomanovMProhodJarvisaMPI, RomanovMProhodJarvisaSEQ>(
    PPC_SETTINGS_romanov_m_prohod_jarvisa);
const auto kGtestValues = ppc::util::TupleToGTestValues(kAllPerfTasks);
INSTANTIATE_TEST_SUITE_P(RunModeTests, RomanovMProhodJarvisaRunPerfTests, kGtestValues,
                         RomanovMProhodJarvisaRunPerfTests::CustomPerfTestName);

}  // namespace romanov_m_prohod_jarvisa
