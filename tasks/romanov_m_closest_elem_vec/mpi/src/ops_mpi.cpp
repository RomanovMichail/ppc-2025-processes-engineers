#include "romanov_m_closest_elem_vec/mpi/include/ops_mpi.hpp"

#include <mpi.h>

#include <numeric>
#include <vector>

#include "romanov_m_closest_elem_vec/common/include/common.hpp"
#include "util/include/util.hpp"

namespace romanov_m_closest_elem_vec {

RomanovMClosestElemVecMPI::RomanovMClosestElemVecMPI(const InType &in) {
  SetTypeOfTask(GetStaticTypeOfTask());
  GetInput() = in;
  GetOutput() = std::make_tuple(-1, -1);
}

bool RomanovMClosestElemVecMPI::ValidationImpl() {
  int rank = 0;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  int size = 0;
  if (rank == 0) {
    size = static_cast<int>(GetInput().size());
  }

  MPI_Bcast(&size, 1, MPI_INT, 0, MPI_COMM_WORLD);
  return size >= 2;
}

bool RomanovMClosestElemVecMPI::PreProcessingImpl() {
  return true;
}

bool RomanovMClosestElemVecMPI::RunImpl() {
  int rank = 0;
  int comm_size = 0;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &comm_size);

  int global_vec_size = 0;
  if (rank == 0) {
    global_vec_size = static_cast<int>(GetInput().size());
  }
  MPI_Bcast(&global_vec_size, 1, MPI_INT, 0, MPI_COMM_WORLD);

  const int base_count = global_vec_size / comm_size;
  const int remainder = global_vec_size % comm_size;

  std::vector<int> send_counts(comm_size);
  std::vector<int> displs(comm_size);

  int current_offset = 0;
  for (int i = 0; i < comm_size; ++i) {
    send_counts[i] = base_count + (i < remainder ? 1 : 0);
    displs[i] = current_offset;
    current_offset += send_counts[i];
  }

  std::vector<int> local_data(send_counts[rank]);
  MPI_Scatterv(rank == 0 ? GetInput().data() : nullptr, send_counts.data(), displs.data(), MPI_INT, local_data.data(),
               send_counts[rank], MPI_INT, 0, MPI_COMM_WORLD);

  struct Result {
    int diff;
    int idx;
  } local_res, global_res;

  local_res.diff = std::numeric_limits<int>::max();
  local_res.idx = -1;

  const int global_offset = displs[rank];
  const int local_sz = static_cast<int>(local_data.size());

  if (local_sz >= 2) {
    for (int i = 0; i < local_sz - 1; ++i) {
      const int diff = std::abs(local_data[i + 1] - local_data[i]);
      if (diff < local_res.diff) {
        local_res.diff = diff;
        local_res.idx = global_offset + i;
      }
    }
  }

  if (comm_size > 1) {
    const int dest = (rank == comm_size - 1) ? MPI_PROC_NULL : rank + 1;
    const int source = (rank == 0) ? MPI_PROC_NULL : rank - 1;

    const int send_val = (local_sz > 0) ? local_data.back() : 0;
    int recv_val = 0;

    MPI_Sendrecv(&send_val, 1, MPI_INT, dest, 0, &recv_val, 1, MPI_INT, source, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    if (rank > 0 && local_sz > 0) {
      const int boundary_diff = std::abs(local_data[0] - recv_val);
      const int boundary_idx = global_offset - 1;

      if (boundary_diff < local_res.diff) {
        local_res.diff = boundary_diff;
        local_res.idx = boundary_idx;
      } else if (boundary_diff == local_res.diff) {
        if (local_res.idx == -1 || boundary_idx < local_res.idx) {
          local_res.idx = boundary_idx;
        }
      }
    }
  }

  MPI_Allreduce(&local_res, &global_res, 1, MPI_2INT, MPI_MINLOC, MPI_COMM_WORLD);

  GetOutput() = std::make_tuple(global_res.idx, global_res.idx + 1);
  return true;
}

bool RomanovMClosestElemVecMPI::PostProcessingImpl() {
  return true;
}

}  // namespace romanov_m_closest_elem_vec
