#include "exhaustor.h"
#include "details.h"
#include "mod11-2.h"
#include "region-codes.h"

#include <stdexcept>

namespace idlib {

namespace {

using namespace std::chrono;

year get_min_matched_year(const std::string &tmpl) {
    if (tmpl.size() != 4) {
        throw std::invalid_argument("The length of the template must be 4.");
    }
    int num = 1;
    for (int64_t i = 0; i < 4; ++i) {
        if (tmpl[i] == '*') {
            num *= 10;
            continue;
        }
        auto add = tmpl[i] - '0';
        if (add < 0 || add > 9) {
            throw std::invalid_argument("The template must only contain digits and '*'.");
        }
        num = num * 10 + add;
    }
    num %= 10000;
    return year{num};
}

year get_max_matched_year(const std::string &tmpl) {
    if (tmpl.size() != 4) {
        throw std::invalid_argument("The length of the template must be 4.");
    }
    int num = 1;
    for (int64_t i = 0; i < 4; ++i) {
        if (tmpl[i] == '*') {
            num = num * 10 + 9;
            continue;
        }
        auto add = tmpl[i] - '0';
        if (add < 0 || add > 9) {
            throw std::invalid_argument("The template must only contain digits and '*'.");
        }
        num = num * 10 + add;
    }
    num %= 10000;
    return year{num};
}

bool match_year_template(year yr, const std::string &tmpl) {
    if (tmpl.size() != 4) {
        throw std::invalid_argument("The length of the template must be 4.");
    }
    auto num = (int)yr;
    for (int64_t i = 3; i >= 0; --i) {
        if (tmpl[i] == '*') {
            num /= 10;
            continue;
        }
        auto digit = tmpl[i] - '0';
        if ((num % 10) != digit) {
            return false;
        }
        num /= 10;
    }
    return true;
}

std::vector<year> range_years(year start, year end) {
    if (start > end) {
        throw std::invalid_argument("The start year must be earlier than the end year.");
    }
    std::vector<year> result;
    for (auto y = start; y <= end; y += years{1}) {
        result.push_back(y);
    }
    return result;
}

std::vector<year> exhaust_years(year start, year end, const std::string &tmpl) {
    if (tmpl.size() != 4) {
        throw std::invalid_argument("The length of the template must be 4.");
    }
    if (tmpl == "****") {
        return range_years(start, end);
    }
    std::vector<year> result;
    auto max_matched_year = get_max_matched_year(tmpl);
    auto min_matched_year = get_min_matched_year(tmpl);
    if (start < min_matched_year) {
        start = min_matched_year;
    }
    if (end > max_matched_year) {
        end = max_matched_year;
    }
    for (auto y = start; y <= end; y++) {
        if (match_year_template(y, tmpl)) {
            result.push_back(y);
        }
    }
    return result;
}

std::vector<month> range_months(month start, month end) {
    if (start > end) {
        throw std::invalid_argument("The start month must be earlier than the end month.");
    }
    std::vector<month> result;
    for (auto m = start; m <= end; m += months{1}) {
        result.push_back(m);
        if (m == month{12}) {
            break;
        }
    }
    return result;
}

std::vector<month> exhaust_months(month start, month end, const std::string &tmpl) {
    if (tmpl.size() != 2) {
        throw std::invalid_argument("The length of the template must be 2.");
    }
    if (tmpl == "**") {
        return range_months(start, end);
    }
    if (tmpl[0] == '*') {
        uint32_t m = tmpl[1] - '0';
        if (m > 2) {
            // tmpl[0] = '0'
            if (start < month{m} && end > month{m}) {
                return {month{m}};
            }
            return {};
        } else {
            // tmpl[0] = '1' or '0'
            std::vector<month> result;
            if (start <= month{m} && end >= month{m}) {
                result.emplace_back(m);
            }
            if (start <= month{m + 10} && end >= month{m + 10}) {
                result.emplace_back(m + 10);
            }
            return result;
        }
    } else if (tmpl[1] == '*') {
        uint32_t m = tmpl[0] - '0';
        if (m == 0) {
            // tmpl[1] = '0'~'9'
            if (start < month{10}) {
                return range_months(start, end >= month(10) ? month(9) : end);
            }
            return {};
        } else if (m == 1) {
            // tmpl[1] = '0'~'2'
            if (end >= month{10}) {
                return range_months(start >= month(10) ? start : month(10), end);
            }
            return {};
        }
        return {};
    } else {
        uint32_t m = (tmpl[0] - '0') * 10 + (tmpl[1] - '0');
        if (m > 12) {
            return {};
        }
        if (start < month{m} && end > month{m}) {
            return {month{m}};
        }
        return {};
    }
}

std::vector<year_month> range_year_months(year_month start, year_month end) {
    if (start > end) {
        throw std::invalid_argument("The start year_month must be earlier than the end year_month.");
    }
    std::vector<year_month> result;
    for (auto ym = start; ym <= end; ym += months{1}) {
        result.push_back(ym);
    }
    return result;
}

std::vector<year_month> exhaust_year_months(year_month start, year_month end, const std::string &tmpl) {
    if (tmpl.size() != 6) {
        throw std::invalid_argument("The length of the template must be 6.");
    }
    if (tmpl == "******") {
        return range_year_months(start, end);
    }
    auto year_tmpl = tmpl.substr(0, 4);
    auto month_tmpl = tmpl.substr(4, 2);
    auto years = exhaust_years(start.year(), end.year(), year_tmpl);
    std::vector<year_month> result;
    for (auto y : years) {
        auto start_month = y == start.year() ? start.month() : std::chrono::month{1};
        auto end_month = y == end.year() ? end.month() : std::chrono::month{12};
        auto months = exhaust_months(start_month, end_month, month_tmpl);
        for (auto m : months) {
            result.emplace_back(y, m);
        }
    }
    return result;
}

std::vector<day> range_days(day start, day end) {
    if (start > end) {
        throw std::invalid_argument("The start day must be earlier than the end day.");
    }
    std::vector<day> result;
    for (auto d = start; d <= end; d += days{1}) {
        result.push_back(d);
    }
    return result;
}

std::vector<day> exhaust_days(day start, day end, const std::string &tmpl) {
    if (tmpl.size() != 2) {
        throw std::invalid_argument("The length of the template must be 2.");
    }
    if (tmpl == "**") {
        return range_days(start, end);
    }
    if (tmpl[0] == '*') {
        std::vector<day> result;
        uint32_t d = tmpl[1] - '0';
        for (int i = 0; i <= 3; i++) {
            uint32_t num = d + i * 10;
            if (start <= day{num} && end >= day{num}) {
                result.emplace_back(num);
            }
        }
        return result;
    } else if (tmpl[1] == '*') {
        switch (tmpl[0] - '0') {
        case 0:
            if (start < day{10}) {
                return range_days(start, end >= day(10) ? day(9) : end);
            }
            return {};
        case 1:
            if (start < day{20} && end > day{10}) {
                return range_days(start < day{10} ? day{10} : start, end >= day{20} ? day{19} : end);
            }
            return {};
        case 2:
            if (start < day{30} && end > day{20}) {
                return range_days(start < day{20} ? day{20} : start, end >= day{30} ? day{29} : end);
            }
            return {};
        case 3:
            if (start < day{31} && end >= day{30}) {
                return range_days(start < day{30} ? day{30} : start, end);
            }
            return {};
        default:
            return {};
        }
    } else {
        uint32_t d = (tmpl[0] - '0') * 10 + (tmpl[1] - '0');
        if (start <= day{d} && end >= day{d}) {
            return {day{d}};
        }
        return {};
    }
}

std::vector<year_month_day> exhaust_year_month_days(year_month_day start, year_month_day end, const std::string &tmpl) {
    if (tmpl.size() != 8) {
        throw std::invalid_argument("The length of the template must be 8.");
    }
    auto year_tmpl = tmpl.substr(0, 4);
    auto month_tmpl = tmpl.substr(4, 2);
    auto day_tmpl = tmpl.substr(6, 2);
    auto start_ym = year_month{start.year(), start.month()};
    auto end_ym = year_month{end.year(), end.month()};
    auto year_months = exhaust_year_months(start_ym, end_ym, year_tmpl + month_tmpl);
    std::vector<year_month_day> result;
    for (auto &ym : year_months) {
        auto start_day = ym == start_ym ? start.day() : std::chrono::day{1};
        auto end_day = ym == end_ym ? end.day() : year_month_day_last(ym.year(), month_day_last(ym.month())).day();
        auto days = exhaust_days(start_day, end_day, day_tmpl);
        for (auto d : days) {
            result.emplace_back(ym.year(), ym.month(), d);
        }
    }
    return result;
}

std::vector<std::string> combine_vector(const std::vector<std::string> &v1, const std::vector<std::string> &v2) {
    std::vector<std::string> result;
    for (auto &s1 : v1) {
        for (auto &s2 : v2) {
            result.emplace_back(s1 + s2);
        }
    }
    return result;
}

std::vector<std::string> combine_vector(const std::vector<std::string> &v1, const std::vector<char> &v2) {
    std::vector<std::string> result;
    for (auto &s1 : v1) {
        for (auto &s2 : v2) {
            result.emplace_back(s1 + s2);
        }
    }
    return result;
}

template <typename ... Args>
std::vector<std::string> combine_vector(const std::vector<std::string> &v1, const std::vector<std::string> &v2,
                                        const Args &... args) {
    return combine_vector(combine_vector(v1, v2), args...);
}

} // namespace

exhaustor::exhaustor(const std::string &id) {
    if (id.size() != 18) {
        throw std::invalid_argument("The length of the id must be 18.");
    }
    for (size_t i = 0; i < 18; i++) {
        if (i == detail::kSequenceCodeIndex && id[i] != '*' && id[i] != 'm' && id[i] != 'M' && id[i] != 'f' &&
            id[i] != 'F' && (id[i] < '0' || id[i] > '9')) {
            throw std::invalid_argument("The sequence code must be a digit, 'm'/'M', 'f'/'F' or '*'.");
        } else if (i == detail::kCheckCodeIndex && id[i] != '*' && id[i] != 'X' && id[i] != 'x' &&
                   (id[i] < '0' || id[i] > '9')) {
            throw std::invalid_argument("The check code must be a digit or 'X'/'x' or '*'.");
        } else if (id[i] != '*' && (id[i] < '0' || id[i] > '9')) {
            throw std::invalid_argument("The id must only contain digits and '*'.");
        }
    }

    id_ = id;
}

std::vector<std::string> exhaustor::exhaust_region_code() {
    std::vector<std::string> result;
    for (auto &region : kRegionCodes) {
        bool matched = true;
        for (size_t i = detail::kRegionCodeStart; i < detail::kRegionCodeEnd; i++) {
            if (id_[i] != '*' && region[i] != id_[i]) {
                matched = false;
                break;
            }
        }
        if (matched) {
            result.emplace_back(region);
        }
    }
    return result;
}

std::vector<std::string> exhaustor::exhaust_date_of_birth(std::chrono::year_month_day start,
                                                          std::chrono::year_month_day end) {
    if (start > end) {
        throw std::invalid_argument("The start date must be earlier than the end date.");
    }
    auto tmpl = id_.substr(detail::kDateOfBirthStart, detail::kDateOfBirthLength);
    std::vector<std::string> result;
    auto ymds = exhaust_year_month_days(start, end, tmpl);
    for (auto &ymd : ymds) {
        result.emplace_back(detail::ymd2str(ymd));
    }
    return result;
}

std::vector<std::string> exhaustor::exhaust_registry_code() {
    std::vector<std::string> result;
    auto registry_code = id_.substr(detail::kRegistryCodeStart, detail::kRegistryCodeLength);
    if (registry_code[0] == '*' && registry_code[1] == '*') {
        // 00 - 99
        for (char i = 0; i < 100; i++) {
            auto &str = result.emplace_back();
            str.push_back(static_cast<char>(i / 10 + '0'));
            str.push_back(static_cast<char>(i % 10 + '0'));
        }
    } else if (registry_code[0] == '*') {
        // *0 - *9
        for (char i = 0; i < 10; i++) {
            auto &str = result.emplace_back();
            str.push_back(static_cast<char>(i + '0'));
            str.push_back(registry_code[1]);
        }
    } else if (registry_code[1] == '*') {
        // 0* - 9*
        for (char i = 0; i < 10; i++) {
            auto &str = result.emplace_back();
            str.push_back(registry_code[0]);
            str.push_back(static_cast<char>(i + '0'));
        }
    } else {
        result.emplace_back(registry_code);
    }
    return result;
}

std::vector<char> exhaustor::exhaust_sequence_code() {
    char seq = id_[detail::kSequenceCodeIndex];
    switch (seq) {
    case '*':
        return {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};
    case 'm':
    case 'M':
        return {'1', '3', '5', '7', '9'};
    case 'f':
    case 'F':
        return {'0', '2', '4', '6', '8'};
    default:
        return {seq};
    }
}

std::vector<std::string> exhaustor::exhaust_all(std::chrono::year_month_day start, std::chrono::year_month_day end) {
    auto region_codes = exhaust_region_code();
    auto date_of_births = exhaust_date_of_birth(start, end);
    auto registry_codes = exhaust_registry_code();
    auto sequence_codes = exhaust_sequence_code();
    std::vector<std::string> result;
    char check_code = id_[detail::kCheckCodeIndex];
    if (check_code == '*') {
        result = combine_vector(region_codes, date_of_births, registry_codes, sequence_codes);
        for (auto &id : result) {
            id += mod11_2::do_mod11_2(id);
        }
    } else {
        auto combined = combine_vector(region_codes, date_of_births, registry_codes, sequence_codes);
        for (auto &id : combined) {
            if (mod11_2::do_mod11_2(id) == check_code) {
                result.emplace_back(id + check_code);
            }
        }
    }
    return result;
}

} // namespace idlib