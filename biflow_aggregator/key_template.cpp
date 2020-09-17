/**
 * @file key_template.cpp
 * @author Pavel Siska (siska@cesnet.cz)
 * @brief Implementation of classes that represent flow key.
 * @version 1.0
 * @date 17.9.2020
 * 
 * @copyright Copyright (c) 2020 CESNET
 */

#include "key_template.h"
#include "fields.h"

#include <cstring>
#include <iostream>

std::vector<std::tuple<ur_field_id_t, ur_field_id_t, std::size_t>> Key_template::key_fields_;
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

std::vector<std::tuple<ur_field_id_t, ur_field_id_t, std::size_t>> Key_template::get_fields() noexcept
{
    return key_fields_;
}

std::size_t Key_template::get_size() noexcept
{
    return key_size_;
}


void FlowKey::update(const void *src, std::size_t size, std::size_t& offset) noexcept
{
    memcpy(std::addressof(s_key_.get()->data()[offset]), src, size);
    offset += size;
}

bool FlowKey::generate(const void *flow_data, ur_template_t *tmplt, bool is_biflow)
{
    using uint128_t = unsigned __int128;

    // current offset in key data
    std::size_t offset = 0;

    // Allocate key memory if it is necessary
    if (s_key_ == nullptr)
        s_key_ = std::make_shared<std::vector<uint8_t>>(Key_template::key_size_);

    // Generate non biflow key
    if (is_biflow == false) {
        for (auto field : Key_template::get_fields()) {
            update(ur_get_ptr_by_id(tmplt, flow_data, std::get<Key_template::ID>(field)), 
                std::get<Key_template::SIZE>(field), offset);
        }
        return false;
    }

    // Generate biflow key
    if (*reinterpret_cast<uint128_t*>(&ur_get(tmplt, flow_data, F_SRC_IP)) 
        > *reinterpret_cast<uint128_t*>(&ur_get(tmplt, flow_data, F_DST_IP))) {
        for (auto field : Key_template::get_fields()) {
            update(ur_get_ptr_by_id(tmplt, flow_data, std::get<Key_template::REVERSE_ID>(field)), 
                std::get<Key_template::SIZE>(field), offset);
        }
        return true;
    } else {
        for (auto field : Key_template::get_fields()) {
            update(ur_get_ptr_by_id(tmplt, flow_data, std::get<Key_template::ID>(field)), 
                std::get<Key_template::SIZE>(field), offset);
        }
        return false;
    }
}

std::pair<void *, std::size_t> FlowKey::get_key() const
{
    return std::make_pair(s_key_.get()->data(), Key_template::key_size_);
}    