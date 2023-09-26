#include "validator.h"
#include "mod11-2.h"
#include "region-codes.h"
#include <stdexcept>

namespace idlib {

namespace {} // namespace

bool validator::validate_basic(const std::string &id) {
    if (id.size() != 18) {
        return false;
    }
    for (size_t i = 0; i < 18; i++) {
        if (id[i] != 'X' && id[i] != 'x' && (id[i] < '0' || id[i] > '9')) {
            return false;
        }
    }
    auto cc = mod11_2::do_mod11_2(id.substr(0, 17));
    if (cc != id[17] || (cc == 'X' && id[17] != 'x')) {
        return false;
    }
    return true;
}

bool validator::validate_region_code(const std::string &region_code) {
    if (std::find(kRegionCodes.begin(), kRegionCodes.end(), region_code) == kRegionCodes.end()) {
        return false;
    }
    return true;
}

bool validator::validate_date_of_birth(
    const std::string &date_of_birth,
    const std::pair<std::chrono::year_month_day, std::chrono::year_month_day> &valid_date_range) {
    using namespace std::chrono;
    if (date_of_birth.size() != 8) {
        return false;
    }
    int year_of_birth;
    int month_of_birth;
    int day_of_birth;
    try {
        year_of_birth = std::stoi(date_of_birth.substr(0, 4));
        month_of_birth = std::stoi(date_of_birth.substr(4, 2));
        day_of_birth = std::stoi(date_of_birth.substr(6, 2));
    } catch (...) {
        return false;
    }
    auto date = year(year_of_birth) / month(month_of_birth) / day(day_of_birth);
    if (!date.ok() || date < valid_date_range.first || date > valid_date_range.second) {
        return false;
    }
    return true;
}

validator::validator(std::string id,
                     std::pair<std::chrono::year_month_day, std::chrono::year_month_day> valid_date_range) noexcept
    : id_(std::move(id)), valid_date_range_(valid_date_range) {}

bool validator::validate() {
    if (id_.size() != 18) {
        errmsg_ = "The length of the id must be 18.";
        where_ = {0, id_.size() - 1};
        return false;
    }
    for (size_t i = 0; i < 18; i++) {
        if (id_[i] != 'X' && id_[i] != 'x' && (id_[i] < '0' || id_[i] > '9')) {
            errmsg_ = "The id must only contain digits, 'X' and 'x'.";
            where_ = {i, i};
            return false;
        }
    }
    if (std::find(kRegionCodes.begin(), kRegionCodes.end(), id_.substr(0, 6)) == kRegionCodes.end()) {
        errmsg_ = "The region code is invalid.";
        where_ = {0, 5};
        return false;
    }
    if (!validate_date_of_birth(id_.substr(6, 8), valid_date_range_)) {
        errmsg_ = "The date of birth is invalid.";
        where_ = {6, 13};
        return false;
    }
    // Currently there is no way to validate the registry code.
    // https://www.zhihu.com/question/68016278
    auto cc = mod11_2::do_mod11_2(id_.substr(0, 17));
    if (cc != id_[17] && (cc != 'X' || id_[17] != 'x')) {
        errmsg_ = "The check code is invalid.";
        where_ = {17, 17};
        return false;
    }
    return true;
}

void validator::validate_or_throw() {
    if (!validate()) {
        throw std::runtime_error(errmsg_);
    }
}

const std::pair<size_t, size_t> &validator::where() const noexcept { return where_; }

const std::string &validator::errmsg() const noexcept { return errmsg_; }

} // namespace idlib