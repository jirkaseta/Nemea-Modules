#include "key_template.h"
#include "fields.h"

#include <cstring>
#include <iostream>

using uint128_t = unsigned __int128;

std::vector<std::tuple<const ur_field_id_t, const ur_field_id_t, const std::size_t>> Key_template::key_fields_;
std::size_t Key_template::key_size_ = 0;

void Key_template::update(ur_field_id_t field_id, ur_field_id_t rev_field_id, std::size_t field_size)
{
    key_fields_.emplace_back(std::make_tuple(field_id, rev_field_id, field_size));
    key_size_ += field_size;
}

void Key_template::reset() noexcept
{
    key_size_ = 0;
    key_fields_.clear();
}

const std::vector<std::tuple<const ur_field_id_t, const ur_field_id_t, const std::size_t>> Key_template::get_fields() noexcept
{
    return key_fields_;
}

std::size_t Key_template::get_size()
{
    return key_size_;
}

void FlowKey::update(const void *src, std::size_t size)
{
    memcpy(&data_[size_], src, size);
    size_ += size;
}

bool FlowKey::generate(const void *flow_data, ur_template_t *tmplt, bool is_biflow)
{
    size_ = 0;

    // Generate non biflow key
    if (is_biflow == false) {
        for (auto field : Key_template::get_fields()) {
            update(ur_get_ptr_by_id(tmplt, flow_data, std::get<Key_template::ID>(field)), 
                std::get<Key_template::SIZE>(field));
        }
        return false;
    }

    if (*reinterpret_cast<uint128_t*>(&ur_get(tmplt, flow_data, F_SRC_IP)) 
        > *reinterpret_cast<uint128_t*>(&ur_get(tmplt, flow_data, F_DST_IP))) {
        for (auto field : Key_template::get_fields()) {
            update(ur_get_ptr_by_id(tmplt, flow_data, std::get<Key_template::REVERSE_ID>(field)), 
                std::get<Key_template::SIZE>(field));
        }
        return true;
    } else {
        for (auto field : Key_template::get_fields()) {
            update(ur_get_ptr_by_id(tmplt, flow_data, std::get<Key_template::ID>(field)), 
                std::get<Key_template::SIZE>(field));
        }
        return false;
    }
}

void FlowKey::init(std::size_t key_size)
{
    data_ = new uint8_t[key_size];
    size_ = 0;
}

void FlowKey::reset()
{
    std::memset(data_, 0, size_); // TODO check
    size_ = 0;
}

std::pair<void *, size_t> FlowKey::get_key() const 
{
    return std::make_pair(data_, size_);
}    

bool FlowKey::operator==(const FlowKey &other) const noexcept
{ 
    return !memcmp(data_, other.data_, size_);
}



FlowKey::FlowKey(const FlowKey &other)
{
    size_ = other.size_;
    data_ = new uint8_t[other.size_];
    memcpy(data_, other.data_, size_);
}