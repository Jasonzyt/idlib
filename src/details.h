#pragma once
#include <chrono>
#include <string>

namespace idlib::detail {

constexpr size_t kRegionCodeStart = 0;
constexpr size_t kRegionCodeEnd = 6;
constexpr size_t kRegionCodeLength = 6;
constexpr size_t kDateOfBirthStart = 6;
constexpr size_t kDateOfBirthEnd = 14;
constexpr size_t kDateOfBirthLength = 8;
constexpr size_t kRegistryCodeStart = 14;
constexpr size_t kRegistryCodeEnd = 16;
constexpr size_t kRegistryCodeLength = 2;
constexpr size_t kSequenceCodeIndex = 16;
constexpr size_t kCheckCodeIndex = 17;

std::string ymd2str(const std::chrono::year_month_day &ymd);
std::string ymd2str(int y, int m, int d);

} // namespace idlib::detail
