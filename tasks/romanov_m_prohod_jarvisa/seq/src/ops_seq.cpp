#include "romanov_m_prohod_jarvisa/seq/include/ops_seq.hpp"

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

std::vector<Point> RomanovMProhodJarvisaSEQ::JarvisMarch(std::vector<Point> points) {
  const int n = static_cast<int>(points.size());
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
  int q = 0;

  while (true) {
    hull.push_back(points[p]);
    q = (p + 1) % n;
    for (int i = 0; i < n; ++i) {
      const int64_t cross = CalcCross(points[p], points[i], points[q]);
      if (cross > 0 || (cross == 0 && CalcDistSq(points[p], points[i]) > CalcDistSq(points[p], points[q]))) {
        q = i;
      }
    }
    p = q;
    if (p == leftmost) {
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
