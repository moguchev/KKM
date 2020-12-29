// Leonid Moguchev (c) 2020
#include "TLV.h"

std::vector<uint8_t> TLV::to_bytes() const {
    auto result = utils::split_le(_tag);
    auto len = utils::split_le(_len);
    result.insert(result.end(), len.begin(), len.end());
    result.insert(result.end(), _data.begin(), _data.end());
    return result;
}

TLV::TLV(uint16_t tag, const std::vector<uint8_t>& data) noexcept :
    _tag(tag), _len(data.size()), _data(data) {}

TLV::TLV(uint16_t tag, std::vector<uint8_t>&& data) noexcept :
    _tag(tag), _len(data.size()), _data(std::move(data)) {}

TLV::TLV(TLV&& t) noexcept :
    _tag(t._tag), _len(t._len), _data(std::move(t._data)) {}

TLV::TLV(const TLV& t) noexcept :
    _tag(t._tag), _len(t._len), _data(t._data) {}

TLV& TLV::operator = (const TLV& t) {
    _tag = t._tag;
    _len = t._len;
    _data = t._data;
    return *this;
}

TLV& TLV::operator = (TLV&& t) noexcept {
    _tag = t._tag;
    _len = t._len;
    _data = std::move(t._data);
    return *this;
}

uint16_t TLV::get_tag() const {
    return _tag;
}

std::vector<std::vector<uint8_t>> TLVList::to_bytes_with_limit(size_t limit) const {
    std::vector<std::vector<uint8_t>> result;
    std::vector<uint8_t> chunk;

    for (const auto& obj : _list) {
        auto bytes = obj.second.to_bytes();
        if (chunk.size() + bytes.size() > limit) {
            result.push_back(chunk);
            chunk.clear();
        }
        chunk.insert(chunk.end(), bytes.begin(), bytes.end());
    }
    if (chunk.size() > 0) {
        result.push_back(chunk);
        chunk.clear();
    }

    return result;
}

void TLVList::push(const TLV& obj) {
    _list.emplace(std::make_pair<>(obj.get_tag(), obj));
}

void TLVList::push(TLV&& obj) {
    _list.emplace(std::make_pair<>(obj.get_tag(), std::move(obj)));
}