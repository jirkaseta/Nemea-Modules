#ifndef KEY_TEMPLATE_H
#define KEY_TEMPLATE_H

#include <libtrap/trap.h>
#include <unirec/unirec.h>
#include <vector>
#include <tuple>

#include "xxhash.h"

class Key_template {
    static std::vector<std::tuple<const ur_field_id_t, const ur_field_id_t, const std::size_t>> key_fields_;
    static std::size_t key_size_;

public:
    enum Tuple_name {ID, REVERSE_ID, SIZE};

    static void update(ur_field_id_t field_id, ur_field_id_t rev_field_id, std::size_t field_size);
    static void reset() noexcept;
    static const std::vector<std::tuple<const ur_field_id_t, const ur_field_id_t, const std::size_t>> get_fields() noexcept;
    static std::size_t get_size();
};

class FlowKey {
    uint8_t *data_;
    std::size_t size_;

public:
    void init(std::size_t key_size);
    void update(const void *src, std::size_t size);
    void reset();
    bool generate(const void *flow_data, ur_template_t *tmplt, bool is_biflow);
    std::pair<void *, size_t> get_key() const;
    bool operator==(const FlowKey &other) const noexcept;

    FlowKey(const FlowKey &other);

    FlowKey()
    {   
    }
};

namespace std
{
    template<> struct hash<FlowKey>
    {
        std::size_t operator()(FlowKey const& key) const noexcept
        {
            auto flow_key = key.get_key(); 
            return static_cast<std::size_t>(XXH3_64bits(flow_key.first, flow_key.second));
        }
    };
}

#endif // KEY_TEMPLATE_H