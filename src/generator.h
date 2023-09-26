/*
 * 免责声明：
 * 本身份证号生成器仅供学习与交流使用，不得用于违法用途，否则后果自负，与开发者无关。
 * 若生成的身份证号码与真实身份证号码重复，纯属巧合。
 * 生成器的生成结果仅保证理论上的正确性，不保证实际可用性。
 * 身份证生成器依赖随机数生成器，随机数生成器的随机性决定了生成器的生成结果的随机性，因此，生成器的结果是可操纵的。
 * 此外，本生成器的生成器并未进行单元测试，如有问题，请提交Issue。
 */
#pragma once
#include <random>

#include "details.h"
#include "mod11-2.h"
#include "region-codes.h"

namespace idlib {

template <typename Random> class generator {

    Random &random_;

    template <class T = int>
    T random_num(T min = std::numeric_limits<T>::min(), T max = std::numeric_limits<T>::max()) {
        std::uniform_int_distribution<T> dist(min, max);
        return dist(random_);
    }

  public:
    enum part : uint64_t {
        kRegionCode = 0x1,
        kDateOfBirth = 0x2,
        kCheckCode = 0x4,
        kAll = 0x5 - 1,
    };

    explicit generator(Random &random) : random_(random) {}

    std::string random_region(bool isValid) {
        if (isValid) {
            return std::string(kRegionCodes[random_num<size_t>(0, kRegionCodes.size() - 1)]);
        } else {
            std::string result(6, '0');
            do {
                for (size_t i = 0; i < 6; i++) {
                    result[i] = random_num(0, 9) + '0';
                }
            } while (std::find(kRegionCodes.begin(), kRegionCodes.end(), result) != kRegionCodes.end());
            return result;
        }
    }

    std::string random_date(bool isValid, std::chrono::year_month_day start, std::chrono::year_month_day end) {
        if (isValid) {
            auto days_start = (std::chrono::local_days)start;
            auto days_end = (std::chrono::local_days)end;
            auto days = random_num(days_start.time_since_epoch().count(), days_end.time_since_epoch().count());
            std::chrono::year_month_day ymd{std::chrono::local_days(std::chrono::days(days))};
            return detail::ymd2str(ymd);
        }
        switch (random_num(0, 4)) {
        case 0: // bad month
            return detail::ymd2str(random_num((int)start.year(), (int)end.year()), random_num(13, 99),
                                   random_num(1, 31));
        case 1: // bad day
            return detail::ymd2str(random_num((int)start.year(), (int)end.year()), random_num(1, 12),
                                   random_num(32, 99));
        case 2: // month is 0
            return detail::ymd2str(random_num((int)start.year(), (int)end.year()), 0, random_num(1, 31));
        case 3: // day is 0
            return detail::ymd2str(random_num((int)start.year(), (int)end.year()), random_num(1, 12), 0);
        case 4: { // out of range
            auto days_start = (std::chrono::local_days)start;
            auto days_end = (std::chrono::local_days)end;
            auto days = (bool)random_num(0, 1)
                            ? random_num(((std::chrono::local_days)std::chrono::year_month_day{
                                              std::chrono::year(0), std::chrono::month(1), std::chrono::day(1)})
                                             .time_since_epoch()
                                             .count(),
                                         days_start.time_since_epoch().count())
                            : random_num(days_end.time_since_epoch().count(),
                                         ((std::chrono::local_days)std::chrono::year_month_day{
                                              std::chrono::year(9999), std::chrono::month(12), std::chrono::day(31)})
                                             .time_since_epoch()
                                             .count());
            std::chrono::year_month_day ymd{std::chrono::local_days(std::chrono::days(days))};
            return detail::ymd2str(ymd);
        }
        default:
            return detail::ymd2str(random_num((int)start.year(), (int)end.year()), random_num(1, 12),
                                   random_num(1, 31));
        }
    }

    std::string random_registry_code() {
        auto num = random_num(0, 99);
        if (num < 10) {
            return std::to_string(0) + std::to_string(num);
        }
        return std::to_string(num);
    }

    std::string random_sequence_code() { return std::to_string(random_num(0, 9)); }

    std::string generate_valid(std::chrono::year_month_day start, std::chrono::year_month_day end) {
        std::string result;
        result += random_region(true);
        result += random_date(true, start, end);
        result += random_registry_code();
        result += random_sequence_code();
        result += mod11_2::do_mod11_2(result);
        return result;
    }

    std::string generate_invalid(bool invalidRegion, bool invalidDate, bool invalidCheckCode,
                                 std::chrono::year_month_day start, std::chrono::year_month_day end) {
        std::string result;
        result += random_region(invalidRegion);
        result += random_date(invalidDate, start, end);
        result += random_registry_code();
        result += random_sequence_code();
        if (invalidCheckCode) {
            char valid_num = mod11_2::do_mod11_2_int(result);
            char invalid_num;
            do {
                invalid_num = random_num(0, 10);
            } while (invalid_num == valid_num);
            result += invalid_num == 10 ? 'X' : static_cast<char>(invalid_num + '0');
        } else {
            result += mod11_2::do_mod11_2(result);
        }
        return result;
    }

    std::string
    generate(uint64_t validParts = kAll,
             std::chrono::year_month_day start = {std::chrono::year(1920), std::chrono::month(1), std::chrono::day(1)},
             std::chrono::year_month_day end = std::chrono::year_month_day{std::chrono::local_days(
                 std::chrono::duration_cast<std::chrono::days>(std::chrono::system_clock::now().time_since_epoch()))}) {
        if (validParts == part::kAll) {
            return generate_valid(start, end);
        }
        bool invalidRegion = !(validParts & kRegionCode);
        bool invalidDate = !(validParts & kDateOfBirth);
        bool invalidCheckCode = !(validParts & kCheckCode);
        return generate_invalid(invalidRegion, invalidDate, invalidCheckCode, start, end);
    }

    std::string generate_all_kinds() {
        if (random_num(0, 1)) {
            return generate(part::kAll);
        }
        switch (random_num(0, 6)) {
        case 0:
            return generate(part::kRegionCode);
        case 1:
            return generate(part::kDateOfBirth);
        case 2:
            return generate(part::kCheckCode);
        case 3:
            return generate(part::kRegionCode | part::kDateOfBirth);
        case 4:
            return generate(part::kRegionCode | part::kCheckCode);
        case 5:
            return generate(part::kDateOfBirth | part::kCheckCode);
        default:
            return generate(part::kAll);
        }
    }

    std::string generate_all_kinds(bool &isValid) {
        if (random_num(0, 1)) {
            isValid = true;
            return generate(part::kAll);
        }
        isValid = false;
        switch (random_num(0, 6)) {
        case 0:
            return generate(part::kRegionCode);
        case 1:
            return generate(part::kDateOfBirth);
        case 2:
            return generate(part::kCheckCode);
        case 3:
            return generate(part::kRegionCode | part::kDateOfBirth);
        case 4:
            return generate(part::kRegionCode | part::kCheckCode);
        case 5:
            return generate(part::kDateOfBirth | part::kCheckCode);
        default:
            return generate(part::kAll);
        }
    }
};

} // namespace idlib