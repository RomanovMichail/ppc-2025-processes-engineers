#include "romanov_m_prohod_jarvisa/seq/include/ops_seq.hpp"

#include <cstdint>
#include <vector>

#include "romanov_m_prohod_jarvisa/common/include/common.hpp"

namespace romanov_m_prohod_jarvisa {

RomanovMProhodJarvisaSEQ::RomanovMProhodJarvisaSEQ(const InType &in) {
  SetTypeOfTask(GetStaticTypeOfTask());
  GetInput() = in;
  GetOutput().clear();
}

bool RomanovMProhodJarvisaSEQ::ValidationImpl() {
  return GetInput().size() >= 3;
}

bool RomanovMProhodJarvisaSEQ::PreProcessingImpl() {
  return true;
}

namespace {

int FindLeftmostPoint(const std::vector<Point> &points) {
  int idx = 0;
  for (std::size_t i = 1; i < points.size(); ++i) {
    if (points[i].x < points[idx].x || (points[i].x == points[idx].x && points[i].y < points[idx].y)) {
      idx = static_cast<int>(i);
    }
  }
  return idx;
}

int SelectNextPoint(const std::vector<Point> &points, int p) {
  const int n = static_cast<int>(points.size());
  int q = (p + 1) % n;

  for (int i = 0; i < n; ++i) {
    const int64_t cross = CalcCross(points[p], points[i], points[q]);
    if (cross > 0) {
      q = i;
    } else if (cross == 0 && CalcDistSq(points[p], points[i]) > CalcDistSq(points[p], points[q])) {
      q = i;
    }
  }
  return q;
}

}  // namespace

std::vector<Point> RomanovMProhodJarvisaSEQ::JarvisMarch(std::vector<Point> points) {
  if (points.size() < 3) {
    return points;
  }

  std::vector<Point> hull;
  const int start = FindLeftmostPoint(points);

  int p = start;
  while (true) {
    hull.push_back(points[p]);
    p = SelectNextPoint(points, p);
    if (p == start) {
      break;
    }
  }

  return hull;
}

bool RomanovMProhodJarvisaSEQ::RunImpl() {
  GetOutput() = JarvisMarch(GetInput());
  return true;
}

bool RomanovMProhodJarvisaSEQ::PostProcessingImpl() {
  return true;
}

}  // namespace romanov_m_prohod_jarvisa
