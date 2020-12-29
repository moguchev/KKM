// Leonid Moguchev (c) 2020
#pragma once

#include <stdint.h>
#include <vector>
#include <map>

#include "utils.h"

class TLV {
public:
    TLV(uint16_t tag, const std::vector<uint8_t>& data) noexcept;

    TLV(uint16_t tag, std::vector<uint8_t>&& data) noexcept;

    TLV(TLV&& t) noexcept;

    TLV(const TLV& t) noexcept;

    TLV& operator = (const TLV& t);

    TLV& operator = (TLV&& t) noexcept;

    ~TLV() = default;

    uint16_t get_tag() const;

    std::vector<uint8_t> to_bytes() const;
private:
    uint16_t _tag;
    uint16_t _len;
    std::vector<uint8_t> _data;
};



class TLVList {
public:
    TLVList() = default;
    ~TLVList() = default;

    void push(const TLV& obj);

    void push(TLV&& obj);

    std::vector<std::vector<uint8_t>> to_bytes_with_limit(size_t limit) const;
private:
    std::map<uint16_t, TLV> _list;
};