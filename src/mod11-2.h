#pragma once
#include <string_view>

namespace idlib {

namespace mod11_2 {

constexpr char kFactors[] = {7, 9, 10, 5,  8, 4, 2, 1, 6,
                             3, 7, 9,  10, 5, 8, 4, 2};
constexpr char kCheckDigits[] = {'1', '0', 'X', '9', '8', '7',
                                 '6', '5', '4', '3', '2'};

constexpr char do_mod11_2(char id[17]) {
    for (int i = 0; i < 16; ++i) {
        if (id[i] < '0' || id[i] > '9') {
            return -1;
        }
        id[i] -= '0';
    }
    int sum = 0;
    for (int i = 0; i < 17; ++i) {
        sum += id[i] * kFactors[i];
    }
    return kCheckDigits[sum % 11];
}

constexpr char do_mod11_2(std::string_view id) {
    if (id.size() != 17) {
        return -1;
    }
    char id_array[17];
    for (int i = 0; i < 17; ++i) {
        id_array[i] = id[i];
    }
    return do_mod11_2(id_array);
}

} // namespace mod11_2

} // namespace idlib