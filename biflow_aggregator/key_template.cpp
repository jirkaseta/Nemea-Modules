#include "key_template.h"
#include <cstring>
#include <iostream>

void KeyTemplate::update_template(int field_id, std::size_t field_size)
{
    key_fields_.emplace_back(std::make_pair(field_id, field_size));
    key_size_ += field_size;
}

void KeyTemplate::reset_template() noexcept
{
    key_size_ = 0;
    key_fields_.clear();
}

const std::vector<std::pair<int, std::size_t>> KeyTemplate::get_template_fields() noexcept
{
    return key_fields_;
}

std::size_t KeyTemplate::get_template_size()
{
    return key_size_;
}



void FlowKey::update(const void *src, std::size_t size)
{
    memcpy(&data_[size_], src, size);
    size_ += size;
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