#include "details.h"

namespace idlib::detail {

std::string ymd2str(const std::chrono::year_month_day &ymd) {
    using namespace std::chrono;
    auto y = (int)ymd.year();
    auto m = (uint32_t)ymd.month();
    auto d = (uint32_t)ymd.day();
    return ymd2str(y, (int)m, (int)d);
}
std::string ymd2str(int y, int m, int d) {
    std::string result(8, '0');
    result[0] = static_cast<char>(y / 1000 + '0');
    result[1] = static_cast<char>(y / 100 % 10 + '0');
    result[2] = static_cast<char>(y / 10 % 10 + '0');
    result[3] = static_cast<char>(y % 10 + '0');
    result[4] = static_cast<char>(m / 10 + '0');
    result[5] = static_cast<char>(m % 10 + '0');
    result[6] = static_cast<char>(d / 10 + '0');
    result[7] = static_cast<char>(d % 10 + '0');
    return result;
}

} // namespace idlib::detail
