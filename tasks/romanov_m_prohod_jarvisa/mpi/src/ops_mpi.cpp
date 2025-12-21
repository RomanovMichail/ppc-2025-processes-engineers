#include "romanov_m_prohod_jarvisa/mpi/include/ops_mpi.hpp"

#include <mpi.h>

#include <algorithm>
#include <cstddef>
#include <utility>
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

namespace {
void CreateMpiPointType(MPI_Datatype *p_type) {
  MPI_Type_contiguous(2, MPI_INT, p_type);
  MPI_Type_commit(p_type);
}

void InitCountsAndDispls(int rank, int size, int n, std::vector<int> &counts, std::vector<int> &displs) {
  if (rank == 0) {
    const int base = n / size;
    const int rem = n % size;
    for (int i = 0; i < size; ++i) {
      counts[i] = (i < rem) ? (base + 1) : base;
    }
    displs[0] = 0;
    for (int i = 1; i < size; ++i) {
      displs[i] = displs[i - 1] + counts[i - 1];
    }
  }
}
}  // namespace

std::vector<Point> RomanovMProhodJarvisaMPI::JarvisMarch(std::vector<Point> points) {
  int n = static_cast<int>(points.size());
  if (n < 3) {
    return points;
  }
  std::vector<Point> hull;
  int leftmost = 0;
  for (int i = 1; i < n; ++i) {
    if (points[i].x < points[leftmost].x || (points[i].x == points[leftmost].x && points[i].y < points[leftmost].y)) {
      leftmost = i;
    }
  }
  int p = leftmost;
  int q;
  do {
    hull.push_back(points[p]);
    q = (p + 1) % n;
    for (int i = 0; i < n; ++i) {
      int cross = CalcCross(points[p], points[i], points[q]);
      if (cross > 0 || (cross == 0 && CalcDistSq(points[p], points[i]) > CalcDistSq(points[p], points[q]))) {
        q = i;
      }
    }
    p = q;
  } while (p != leftmost);
  return hull;
}

std::vector<Point> RomanovMProhodJarvisaMPI::ComputeFinalHull(int rank, std::vector<Point> &all_hull_points) {
  if (rank != 0) {
    return {};
  }
  return JarvisMarch(std::move(all_hull_points));
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
  CreateMpiPointType(&p_type);

  std::vector<int> counts(size), displs(size);
  InitCountsAndDispls(rank, size, n, counts, displs);

  int l_size = (rank < (n % size)) ? (n / size + 1) : (n / size);
  std::vector<Point> l_points(l_size);
  MPI_Scatterv(rank == 0 ? GetInput().data() : nullptr, counts.data(), displs.data(), p_type, l_points.data(), l_size,
               p_type, 0, MPI_COMM_WORLD);

  std::vector<Point> l_hull = JarvisMarch(l_points);
  int l_count = static_cast<int>(l_hull.size());
  std::vector<int> r_counts(size), r_displs(size);
  MPI_Gather(&l_count, 1, MPI_INT, rank == 0 ? r_counts.data() : nullptr, 1, MPI_INT, 0, MPI_COMM_WORLD);

  int total = 0;
  if (rank == 0) {
    for (int i = 0; i < size; ++i) {
      r_displs[i] = total;
      total += r_counts[i];
    }
  }

  std::vector<Point> a_hull_points(total);
  MPI_Gatherv(l_hull.data(), l_count, p_type, rank == 0 ? a_hull_points.data() : nullptr, r_counts.data(),
              r_displs.data(), p_type, 0, MPI_COMM_WORLD);

  std::vector<Point> f_hull = ComputeFinalHull(rank, a_hull_points);
  int f_size = static_cast<int>(f_hull.size());
  MPI_Bcast(&f_size, 1, MPI_INT, 0, MPI_COMM_WORLD);
  if (rank != 0) {
    f_hull.resize(f_size);
  }
  MPI_Bcast(f_hull.data(), f_size, p_type, 0, MPI_COMM_WORLD);

  GetOutput() = std::move(f_hull);
  MPI_Type_free(&p_type);
  return true;
}

bool RomanovMProhodJarvisaMPI::PostProcessingImpl() {
  return true;
}

}  // namespace romanov_m_prohod_jarvisa
