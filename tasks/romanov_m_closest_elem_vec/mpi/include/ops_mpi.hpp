#pragma once

#include "romanov_m_closest_elem_vec/common/include/common.hpp"
#include "task/include/task.hpp"

namespace romanov_m_closest_elem_vec {

class RomanovMClosestElemVecMPI : public BaseTask {
 public:
  static constexpr ppc::task::TypeOfTask GetStaticTypeOfTask() {
    return ppc::task::TypeOfTask::kMPI;
  }
  explicit RomanovMClosestElemVecMPI(const InType &in);

 private:
  bool ValidationImpl() override;
  bool PreProcessingImpl() override;
  bool RunImpl() override;
  bool PostProcessingImpl() override;
};

}  // namespace romanov_m_closest_elem_vec
