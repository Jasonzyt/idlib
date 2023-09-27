#include "exhaustor.h"
#include "details.h"
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
    for (int64_t i = 3; i >= 0; ++i) {
        if (tmpl[i] == '*') {
            num /= 10;
            continue;
        }
        auto digit = tmpl[i] - '0';
        if (num % 10 != digit) {
            return false;
        }
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
                return { month{m} };
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
            return { month{m} };
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
                                                          std::chrono::year_month_day end) {}

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

} // namespace idlib