#pragma once
#include <chrono>
#include <string>

namespace idlib::detail {

std::string ymd2str(const std::chrono::year_month_day &ymd);
std::string ymd2str(int y, int m, int d);

} // namespace idlib::detail
