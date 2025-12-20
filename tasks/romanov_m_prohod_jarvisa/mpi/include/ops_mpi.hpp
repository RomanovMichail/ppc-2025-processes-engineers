#pragma once

#include <vector>

#include "romanov_m_prohod_jarvisa/common/include/common.hpp"
#include "task/include/task.hpp"

namespace romanov_m_prohod_jarvisa {

class RomanovMProhodJarvisaMPI : public BaseTask {
 public:
  static constexpr ppc::task::TypeOfTask GetStaticTypeOfTask() noexcept {
    return ppc::task::TypeOfTask::kMPI;
  }

  explicit RomanovMProhodJarvisaMPI(const InType &in);

 private:
  bool ValidationImpl() override;
  bool PreProcessingImpl() override;
  bool RunImpl() override;
  bool PostProcessingImpl() override;
};

}  // namespace romanov_m_prohod_jarvisa
