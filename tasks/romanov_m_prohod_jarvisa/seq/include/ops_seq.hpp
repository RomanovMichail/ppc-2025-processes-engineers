#pragma once

#include <vector>

#include "romanov_m_prohod_jarvisa/common/include/common.hpp"
#include "task/include/task.hpp"

namespace romanov_m_prohod_jarvisa {

class RomanovMProhodJarvisaSEQ : public BaseTask {
 public:
  static constexpr ppc::task::TypeOfTask GetStaticTypeOfTask() {
    return ppc::task::TypeOfTask::kSEQ;
  }
  explicit RomanovMProhodJarvisaSEQ(const InType &in);

 private:
  bool ValidationImpl() override;
  bool PreProcessingImpl() override;
  bool RunImpl() override;
  bool PostProcessingImpl() override;

  static std::vector<Point> JarvisMarch(std::vector<Point> points);
};

}  // namespace romanov_m_prohod_jarvisa
