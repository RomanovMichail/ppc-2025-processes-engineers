#pragma once

#include <string>
#include <tuple>

#include "task/include/task.hpp"

namespace romanov_m_closest_elem_vec{

using InType = int;
using OutType = int;
using TestType = std::tuple<int, std::string>;
using BaseTask = ppc::task::Task<InType, OutType>;

}  // namespace romanov_m_closest_elem_vec
