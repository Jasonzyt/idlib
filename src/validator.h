#pragma once
#include <chrono>
#include <string>

namespace idlib {

class validator {

    std::string id_{};
    std::string errmsg_{};
    std::pair<size_t, size_t> where_{};
    std::pair<std::chrono::year_month_day, std::chrono::year_month_day> valid_date_range_{};

  public:
    static bool validate_basic(const std::string &id);

    static bool validate_region_code(const std::string &region_code);

    static bool
    validate_date_of_birth(const std::string &date_of_birth,
                           const std::pair<std::chrono::year_month_day, std::chrono::year_month_day> &valid_date_range);

    /**
     * @brief Construct a new validator object.
     *
     * @param id The id to be validated.
     * @param valid_year_range The valid year range of the id(-1 for this year).
     */
    explicit validator(std::string id,
              std::pair<std::chrono::year_month_day, std::chrono::year_month_day> valid_date_range = {}) noexcept;

    /**
     * @brief Validate the id.
     *
     * @return true if the id is valid.
     * @return false if the id is invalid.
     */
    bool validate();

    /**
     * @brief Validate or throw if the id is invalid.
     */
    void validate_or_throw();

    /**
     * @brief Get the position of the first invalid part.
     *
     * @return const std::pair<size_t, size_t>& The position of the first invalid part.
     */
    [[nodiscard]] const std::pair<size_t, size_t> &where() const noexcept;

    /**
     * @brief Get the error message if the id is invalid.
     *
     * @return const std::string& The error message.
     */
    [[nodiscard]] const std::string &errmsg() const noexcept;
};

} // namespace idlib