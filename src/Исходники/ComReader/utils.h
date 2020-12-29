// Leonid Moguchev (c) 2020
#pragma once

#include <string>
#include <vector>
#include <memory>
#include <stdexcept>
#include <time.h>

namespace utils {
    template<typename ...Args>
    std::string string_format(const std::string& format, Args ...args) {
        size_t size = uint64_t(snprintf(nullptr, 0, format.c_str(), args ...)) + 1; // Extra space for '\0'
        if (size <= 0) {
            throw std::runtime_error("Error during formatting.");
        }
        std::unique_ptr<char[]> buf(new char[size]);
        snprintf(buf.get(), size, format.c_str(), args...);
        return std::string(buf.get(), buf.get() + size - 1); // We don't want the '\0' inside
    }

    template<typename T>
    std::vector<uint8_t> split_le(T value) {
        auto res = std::vector<uint8_t>(sizeof(T));

        uint8_t* ptr = (uint8_t*)&value;

        for (size_t i = 0; i < sizeof(T); ++i) {
            res[i] = *ptr++;
        }

        return res;
    }

    const auto HEX = "0123456789ABCDEF";

    inline std::string byte_to_hex(uint8_t c) {
        return string_format("0x%c%c", HEX[(c >> 4) & 0xF], HEX[c & 0xF]);
    }

    inline uint16_t union_bytes(uint8_t l, uint8_t h) {
        return uint16_t(l) | (uint16_t(h) << 8);
    }

    inline uint32_t union_bytes(uint8_t b0, uint8_t b1, uint8_t b2, uint8_t b3) {
        return (uint32_t(b0)) | (uint32_t(b1) << 8) | (uint32_t(b2) << 16) | (uint32_t(b3) << 24);
    }

    inline uint64_t union_bytes(uint8_t b0, uint8_t b1, uint8_t b2, uint8_t b3, uint8_t b4, uint8_t b5, uint8_t b6, uint8_t b7) {
        return (uint64_t(b0)) | (uint64_t(b1) << 8) | (uint64_t(b2) << 16) | (uint64_t(b3) << 24) | (uint64_t(b4) << 32) | (uint64_t(b5) << 40) | (uint64_t(b6) << 48) | (uint64_t(b7) << 56);
    }

    inline std::string date_from_bytes(const std::vector<uint8_t>& bytes) {
        if (bytes.size() != 3) {
            return "";
        }

        auto day = string_format("%d", bytes[2]);
        if (bytes[2] < 10) {
            day = "0" + day;
        }
        auto month = string_format("%d", bytes[1]);
        if (bytes[1] < 10) {
            month = "0" + month;
        }
        auto year = string_format("%d", bytes[0]);
        if (bytes[1] < 10) {
            year = "0" + year;
        }

        return string_format("%s.%s.20%s", day.c_str(), month.c_str(), year.c_str());
    }

    inline std::string date_time_from_bytes(const std::vector<uint8_t>& bytes) {
        if (bytes.size() != 5) {
            return "";
        }
        auto min = string_format("%d", bytes[4]);
        if (bytes[4] < 10) {
            min = "0" + min;
        }
        auto hour = string_format("%d", bytes[3]);
        if (bytes[3] < 10) {
            hour = "0" + hour;
        }
        auto day = string_format("%d", bytes[2]);
        if (bytes[2] < 10) {
            day = "0" + day;
        }
        auto month = string_format("%d", bytes[1]);
        if (bytes[1] < 10) {
            month = "0" + month;
        }
        auto year = string_format("%d", bytes[0]);
        if (bytes[0] < 10) {
            year = "0" + year;
        }

        return string_format("%s.%s.20%s %s:%s", day.c_str(), month.c_str(), year.c_str(), hour.c_str(), min.c_str());
    }

    inline std::vector<uint8_t> time_to_bytes(time_t t) {
        std::vector<uint8_t> data_time;

        struct tm ltm;
        localtime_s(&ltm, &t);
        data_time.push_back(ltm.tm_year - 30);
        data_time.push_back(ltm.tm_mon + 1);
        data_time.push_back(ltm.tm_mday);
        data_time.push_back(ltm.tm_hour + 1);
        data_time.push_back(ltm.tm_min + 1);

        return data_time;
    }
}