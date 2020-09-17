/**
 * @file key_template.h
 * @author Pavel Siska (siska@cesnet.cz)
 * @brief Classes that represent flow key.
 * @version 1.0
 * @date 17.9.2020
 * 
 * @copyright Copyright (c) 2020 CESNET
 */

#ifndef KEY_TEMPLATE_H
#define KEY_TEMPLATE_H

#include "xxhash.h"

#include <libtrap/trap.h>
#include <unirec/unirec.h>

#include <memory>
#include <vector>
#include <tuple>

/**
 * 
 */
class Key_template {

    /**
     * Vector that store all information about key.
     */
    static std::vector<std::tuple<ur_field_id_t, ur_field_id_t, std::size_t>> key_fields_;

    /**
     * Size of key.
     */
    static std::size_t key_size_;

    friend class FlowKey;

public:

    /**
     * Named tuple indexes.
     */
    enum Tuple_name {ID, REVERSE_ID, SIZE};

    /**
     * Add new field to key.
     */
    static void update(ur_field_id_t field_id, ur_field_id_t rev_field_id, std::size_t field_size);

    /**
     * Reset class to default state.
     */
    static void reset() noexcept;

    /**
     * Get key fields;
     */
    static std::vector<std::tuple<ur_field_id_t, ur_field_id_t, std::size_t>> get_fields() noexcept;

    /**
     * Get template key size.
     */
    static std::size_t get_size() noexcept;
};

/**
 * Class that store key of input flow.
 */
class FlowKey {

    /**
     * Smart pointer that store key data.
     * This pointer is released by himself after his reference counter is zero.
     */
    std::shared_ptr<std::vector<uint8_t>> s_key_;

    /**
     * Write data from src to key.
     */
    void update(const void *src, std::size_t size, std::size_t& offset) noexcept;

public:

    /**
     * Generate biflow or non-biflow key from input flow and template.
     * 
     */
    bool generate(const void *flow_data, ur_template_t *tmplt, bool is_biflow);

    /**
     * Get pair of key data that contains pointer to key memory and size of this memory.
     */
    std::pair<void *, std::size_t> get_key() const;

    /**
     * Weak copy constructor.
     */
    FlowKey(const FlowKey &other) noexcept : s_key_(other.s_key_) { }

    /**
     * Move constructor.
     */
    FlowKey(FlowKey &&other) noexcept : s_key_(other.s_key_) 
    {
        other.s_key_ = nullptr;
    }

    FlowKey()
    {   
        s_key_ = nullptr;
    }

    // TODO perf copy and move assign insted of unified AO.
    FlowKey& operator=(FlowKey other) noexcept 
    {
        s_key_ = other.s_key_;
        return *this; 
    }

    bool operator==(const FlowKey &other) const noexcept
    {
        return !memcmp(s_key_.get()->data(), other.s_key_.get()->data(), Key_template::key_size_);
    }
};

/**
 * XXH3_64bits hash function in std namespace.
 */
namespace std
{
    template<> struct hash<FlowKey>
    {
        std::size_t operator()(FlowKey const& key) const noexcept
        {
            void *data;
            std::size_t size;
            std::tie(data, size) = key.get_key(); 
            return static_cast<std::size_t>(XXH3_64bits(data, size));
        }
    };
}

#endif // KEY_TEMPLATE_H