#include "romanov_m_prohod_jarvisa/mpi/include/ops_mpi.hpp"

#include <mpi.h>

#include <algorithm>
#include <vector>

#include "romanov_m_prohod_jarvisa/common/include/common.hpp"

namespace romanov_m_prohod_jarvisa {

RomanovMProhodJarvisaMPI::RomanovMProhodJarvisaMPI(const InType &in) {
  SetTypeOfTask(GetStaticTypeOfTask());
  GetInput() = in;
  GetOutput().clear();
}

bool RomanovMProhodJarvisaMPI::ValidationImpl() {
  return GetInput().size() >= 3;
}

bool RomanovMProhodJarvisaMPI::PreProcessingImpl() {
  return true;
}

bool RomanovMProhodJarvisaMPI::RunImpl() {
  int rank, size;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  int n = 0;
  if (rank == 0) {
    n = static_cast<int>(GetInput().size());
  }
  MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);

  if (n < 3) {
    if (rank == 0) {
      GetOutput() = GetInput();
    }
    return true;
  }

  MPI_Datatype p_type;
  MPI_Type_contiguous(2, MPI_INT, &p_type);
  MPI_Type_commit(&p_type);

  std::vector<int> counts(size), displs(size);
  if (rank == 0) {
    int base = n / size;
    int rem = n % size;
    for (int i = 0; i < size; ++i) {
      counts[i] = (i < rem) ? (base + 1) : base;
      displs[i] = (i == 0) ? 0 : displs[i - 1] + counts[i - 1];
    }
  }
  MPI_Bcast(counts.data(), size, MPI_INT, 0, MPI_COMM_WORLD);

  std::vector<Point> l_points(counts[rank]);
  MPI_Scatterv(rank == 0 ? GetInput().data() : nullptr, counts.data(), displs.data(), p_type, l_points.data(),
               counts[rank], p_type, 0, MPI_COMM_WORLD);

  Point pivot;
  if (rank == 0) {
    pivot = FindPivotPoint(GetInput());
  }
  MPI_Bcast(&pivot, 1, p_type, 0, MPI_COMM_WORLD);

  std::vector<Point> hull;
  Point current_p = pivot;

  do {
    if (rank == 0) {
      hull.push_back(current_p);
    }

    Point local_q = current_p;
    bool first_cand = true;

    for (const auto &p : l_points) {
      if (p == current_p) {
        continue;
      }
      if (first_cand) {
        local_q = p;
        first_cand = false;
        continue;
      }
      int64_t cross = CalcCross(current_p, p, local_q);
      if (cross > 0 || (cross == 0 && CalcDistSq(current_p, p) > CalcDistSq(current_p, local_q))) {
        local_q = p;
      }
    }

    std::vector<Point> candidates(size);
    MPI_Gather(&local_q, 1, p_type, candidates.data(), 1, p_type, 0, MPI_COMM_WORLD);

    if (rank == 0) {
      Point global_q = current_p;
      bool global_first = true;
      for (int i = 0; i < size; ++i) {
        if (candidates[i] == current_p) {
          continue;
        }
        if (global_first) {
          global_q = candidates[i];
          global_first = false;
          continue;
        }
        int64_t cross = CalcCross(current_p, candidates[i], global_q);
        if (cross > 0 || (cross == 0 && CalcDistSq(current_p, candidates[i]) > CalcDistSq(current_p, global_q))) {
          global_q = candidates[i];
        }
      }
      current_p = global_q;
    }
    MPI_Bcast(&current_p, 1, p_type, 0, MPI_COMM_WORLD);
  } while (current_p != pivot);

  int hull_size = 0;
  if (rank == 0) {
    hull_size = static_cast<int>(hull.size());
  }
  MPI_Bcast(&hull_size, 1, MPI_INT, 0, MPI_COMM_WORLD);

  if (rank != 0) {
    hull.resize(hull_size);
  }
  MPI_Bcast(hull.data(), hull_size, p_type, 0, MPI_COMM_WORLD);

  GetOutput() = std::move(hull);

  MPI_Type_free(&p_type);
  return true;
}

bool RomanovMProhodJarvisaMPI::PostProcessingImpl() {
  return true;
}

}  // namespace romanov_m_prohod_jarvisa
