#pragma once
#include <chrono>
#include <map>
#include <string>
#include <vector>

//
// Basic PRC ID format:
// | INDEX | 00 01 | 02 03 | 04 05 | 06 07 08 09 | 10 11 | 12 13 | 14 15 | 16 | 17 |
// | FIELD | prov  | cty   | dist  | year ob     | mob   | dob   | reg   | sq | cc |
// | VALUE | 1  1  | 0  1  | 0  1  | 1  9  1  9  | 0  8  | 1  0  | 1  0  | 1  | X  |
//
// * Sequence Code: is odd -> Male | is even -> Female
// * Check Code: is calculated by mod 11-2 algorithm, may be 'X' if the result is 10
//

namespace idlib {

class exhaustor {

    std::string id_{};

  public:
    /**
     * @brief Construct a new exhaustor object.
     *
     * @param id The known part of the id, like "11****19190810***0" or "11****19190810**m0"(m=male, f=female).
     * @throw std::invalid_argument if the length of the id is not 18.
     * @throw std::invalid_argument if the id contains invalid characters.
     * @throw std::invalid_argument if the region code is invalid.
     * @throw std::invalid_argument if the check code is invalid.
     */
    explicit exhaustor(const std::string &id);

    /**
     * @brief Exhaust the region code.
     *
     * @return std::vector<std::string> The possible region codes.
     */
    std::vector<std::string> exhaust_region_code();

    /**
     * @brief Exhaust the date of birth.
     *
     * @param start The start date.
     * @param end The end date.
     * @return std::vector<std::string> The possible date of birth.
     * @throw std::invalid_argument if the start date is later than the end date.
     */
    std::vector<std::string> exhaust_date_of_birth(std::chrono::year_month_day start, std::chrono::year_month_day end);

    /**
     * @brief Exhaust the registry code.
     *
     * @return std::vector<std::string> The possible registry codes.
     */
    std::vector<std::string> exhaust_registry_code();

    /**
     * @brief Exhaust the sequence code.
     *
     * @return std::vector<char> The possible sequence codes.
     */
    std::vector<char> exhaust_sequence_code();

};
} // namespace idlib