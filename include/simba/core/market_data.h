#pragma once

#include <string>

namespace simba {

inline constexpr int64_t INT64_NULL = std::numeric_limits<int64_t>::max();
inline constexpr int32_t INT32_NULL = std::numeric_limits<int32_t>::max();
inline constexpr uint32_t UINT32_NULL = std::numeric_limits<uint32_t>::max();
inline constexpr uint64_t UINT64_NULL = std::numeric_limits<uint64_t>::max();

#pragma pack(push, 1)
struct Int64NULL {
    int64_t value;

    bool is_null() const {
        return value == INT64_NULL;
    }

    std::string to_string() const {
        return is_null() ? "null" : std::to_string(value);
    }
};

struct UInt32NULL {
    uint32_t value;

    bool is_null() const {
        return value == UINT32_NULL;
    }

    std::string to_string() const {
        return is_null() ? "null" : std::to_string(value);
    }
};

struct Decimal5 {
    int64_t mantissa;
    static constexpr int exponent = -5;

    double to_double() const {
        return mantissa * 1e-5;
    }
};

struct Decimal2 {
    int64_t mantissa;
    static constexpr int exponent = -2;

    double to_double() const {
        return mantissa * 1e-2;
    }
};

struct Decimal5NULL {
    int64_t mantissa;
    static constexpr int exponent = -5;

    bool is_null() const {
        return mantissa == INT64_NULL;
    }

    double to_double() const {
        return is_null() ? 0.0 : mantissa * 1e-5;
    }

    std::string to_string() const {
        return is_null() ? "null" : std::to_string(to_double());
    }
};

struct Decimal2NULL {
    int64_t mantissa;
    static constexpr int exponent = -2;

    bool is_null() const {
        return mantissa == INT64_NULL;
    }

    double to_double() const {
        return is_null() ? 0.0 : mantissa * 1e-2;
    }

    std::string to_string() const {
        return is_null() ? "null" : std::to_string(to_double());
    }
};

#pragma pack(pop)

} // namespace simba
