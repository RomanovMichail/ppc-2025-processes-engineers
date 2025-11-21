#pragma once

#include "romanov_m_closest_elem_vec/common/include/common.hpp"
#include "task/include/task.hpp"

namespace romanov_m_closest_elem_vec {

void calculate_distribution(int total_size, int comm_size, std::vector<int> &send_counts, std::vector<int> &displs);

void local_find_min_diff(const std::vector<int> &local_data, int local_sz, int global_offset, Result &local_res);

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
