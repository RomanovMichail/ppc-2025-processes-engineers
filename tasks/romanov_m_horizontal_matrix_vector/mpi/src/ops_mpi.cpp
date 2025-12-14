#include "romanov_m_horizontal_matrix_vector/mpi/include/ops_mpi.hpp"

#include <mpi.h>

#include <vector>

namespace romanov_m_horizontal_matrix_vector {

RomanovMHorizontalMatrixVectorMPI::RomanovMHorizontalMatrixVectorMPI(const InType &in) {
  SetTypeOfTask(GetStaticTypeOfTask());

  int rank = 0;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  if (rank == 0) {
    GetInput() = in;
  }
  GetOutput() = std::vector<double>{};
}

bool RomanovMHorizontalMatrixVectorMPI::ValidationImpl() {
  int rank = 0;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  if (rank == 0) {
    const auto &input = GetInput();
    const auto &matrix = std::get<0>(input);
    const int rows = std::get<1>(input);
    const int cols = std::get<2>(input);
    const auto &vec = std::get<3>(input);

    if (rows <= 0 || cols <= 0) {
      return false;
    }

    bool mat_ok = (matrix.size() == static_cast<size_t>(rows) * static_cast<size_t>(cols));
    bool vec_ok = (vec.size() == static_cast<size_t>(cols));

    return mat_ok && vec_ok;
  }
  return true;
}

bool RomanovMHorizontalMatrixVectorMPI::PreProcessingImpl() {
  int rank = 0;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  int rows = 0;
  if (rank == 0) {
    rows = std::get<1>(GetInput());
  }
  MPI_Bcast(&rows, 1, MPI_INT, 0, MPI_COMM_WORLD);

  GetOutput().resize(rows);
  return true;
}

void RomanovMHorizontalMatrixVectorMPI::CalculateDistribution(int rows, int proc_num, std::vector<int> &counts,
                                                              std::vector<int> &displs) {
  int rows_per_proc = rows / proc_num;
  int remainder = rows % proc_num;

  counts.resize(proc_num);
  displs.resize(proc_num);

  int current_disp = 0;
  for (int i = 0; i < proc_num; ++i) {
    counts[i] = rows_per_proc;
    if (i < remainder) {
      counts[i]++;
    }
    displs[i] = current_disp;
    current_disp += counts[i];
  }
}

bool RomanovMHorizontalMatrixVectorMPI::RunImpl() {
  int rank = 0;
  int size = 0;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  int rows = 0;
  int cols = 0;
  std::vector<double> vec;

  if (rank == 0) {
    rows = std::get<1>(GetInput());
    cols = std::get<2>(GetInput());
    vec = std::get<3>(GetInput());
  }

  MPI_Bcast(&rows, 1, MPI_INT, 0, MPI_COMM_WORLD);
  MPI_Bcast(&cols, 1, MPI_INT, 0, MPI_COMM_WORLD);

  if (rank != 0) {
    vec.resize(cols);
  }
  MPI_Bcast(vec.data(), cols, MPI_DOUBLE, 0, MPI_COMM_WORLD);

  std::vector<int> rows_counts;
  std::vector<int> rows_displs;
  CalculateDistribution(rows, size, rows_counts, rows_displs);

  std::vector<int> send_counts(size);
  std::vector<int> send_displs(size);
  for (int i = 0; i < size; ++i) {
    send_counts[i] = rows_counts[i] * cols;
    send_displs[i] = rows_displs[i] * cols;
  }

  int my_rows = rows_counts[rank];
  std::vector<double> local_matrix(my_rows * cols);

  const double *sendbuf = nullptr;
  if (rank == 0) {
    sendbuf = std::get<0>(GetInput()).data();
  }

  MPI_Scatterv(sendbuf, send_counts.data(), send_displs.data(), MPI_DOUBLE, local_matrix.data(), my_rows * cols,
               MPI_DOUBLE, 0, MPI_COMM_WORLD);

  std::vector<double> local_res(my_rows);
  for (int i = 0; i < my_rows; ++i) {
    double sum = 0.0;
    for (int j = 0; j < cols; ++j) {
      sum += local_matrix[i * cols + j] * vec[j];
    }
    local_res[i] = sum;
  }

  MPI_Allgatherv(local_res.data(), my_rows, MPI_DOUBLE, GetOutput().data(), rows_counts.data(), rows_displs.data(),
                 MPI_DOUBLE, MPI_COMM_WORLD);

  return true;
}

bool RomanovMHorizontalMatrixVectorMPI::PostProcessingImpl() {
  return true;
}

}  // namespace romanov_m_horizontal_matrix_vector
